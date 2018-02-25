#include "RenderCore.h"

#include <mutex>
#include <array>

#include "DepthStencilState.h"
#include "TextureRefs.h"
#include "GpuBuffer.h"
#include "GpuProgramCompiler.h"
#include "FileWatcher.h"
#include "PathService.h"
#include "GpuProgramPipeline.h"
#include "TextureLoader.h"
#include "BinaryCache.h"
#include "Mesh.h"
#include "GeometryData.h"
#include "BlendState.h"
#include "GpuProgram.h"
#include "RenderCore_PlatformDX12.h"
#include "VertexInputLayout.h"
#include "TextureDesc.h"
#include "RenderOutput.h"
#include "CommandContext.h"
#include "RenderingStartupParameters.h"

//---------------------------------------------------------------------------//
namespace Fancy {
//---------------------------------------------------------------------------//
  std::unique_ptr<RenderCore_Platform> RenderCore::ourPlatformImpl;

  std::map<uint64, SharedPtr<GpuProgram>> RenderCore::ourShaderCache;
  std::map<uint64, SharedPtr<GpuProgramPipeline>> RenderCore::ourGpuProgramPipelineCache;
  std::map<uint64, SharedPtr<Texture>> RenderCore::ourTextureCache;
  std::map<uint64, SharedPtr<Mesh>> RenderCore::ourMeshCache;
  std::map<uint64, SharedPtr<BlendState>> RenderCore::ourBlendStateCache;
  std::map<uint64, SharedPtr<DepthStencilState>> RenderCore::ourDepthStencilStateCache;

  std::vector<std::unique_ptr<CommandContext>> RenderCore::ourRenderContextPool;
  std::vector<std::unique_ptr<CommandContext>> RenderCore::ourComputeContextPool;
  std::list<CommandContext*> RenderCore::ourAvailableRenderContexts;
  std::list<CommandContext*> RenderCore::ourAvailableComputeContexts;
  
  SharedPtr<Texture> RenderCore::ourDefaultDiffuseTexture;
  SharedPtr<Texture> RenderCore::ourDefaultNormalTexture;
  SharedPtr<Texture> RenderCore::ourDefaultSpecularTexture;
  SharedPtr<DepthStencilState> RenderCore::ourDefaultDepthStencilState;
  SharedPtr<BlendState> RenderCore::ourDefaultBlendState;
  
  std::unique_ptr<FileWatcher> RenderCore::ourShaderFileWatcher;
  std::unique_ptr<GpuProgramCompiler> RenderCore::ourShaderCompiler;
//---------------------------------------------------------------------------//  
  bool RenderCore::IsInitialized()
  {
    return ourPlatformImpl != nullptr && ourPlatformImpl->IsInitialized();
  }

  const Texture* RenderCore::GetDefaultDiffuseTexture()
  {
    return ourDefaultDiffuseTexture.get();
  }

  const Texture* RenderCore::GetDefaultNormalTexture()
  {
    return ourDefaultNormalTexture.get();
  }

  const Texture* RenderCore::GetDefaultMaterialTexture()
  {
    return ourDefaultSpecularTexture.get();
  }

  const GpuProgramCompiler* RenderCore::GetGpuProgramCompiler()
  {
    return ourShaderCompiler.get();
  }

//---------------------------------------------------------------------------//
  void RenderCore::Init(RenderingApi aRenderingApi)
  {
    Init_0_Platform(aRenderingApi);
    Init_1_Services();
    Init_2_Resources();
  }
//---------------------------------------------------------------------------//
  void RenderCore::Shutdown()
  {
    Shutdown_0_Resources();
    Shutdown_1_Services();
    Shutdown_2_Platform();
  }
//---------------------------------------------------------------------------//
  RenderCore_PlatformDX12* RenderCore::GetPlatformDX12()
  {
    return static_cast<RenderCore_PlatformDX12*>(ourPlatformImpl.get());
  }
//---------------------------------------------------------------------------//
  void RenderCore::Init_0_Platform(RenderingApi aRenderingApi)
  {
    ASSERT(ourPlatformImpl == nullptr);

    switch (aRenderingApi)
    {
      case RenderingApi::DX12:
        ourPlatformImpl = std::make_unique<RenderCore_PlatformDX12>();
        break;
      case RenderingApi::VULKAN: break;
      default:;
    }

    ASSERT(ourPlatformImpl != nullptr, "Unsupported rendering API requested");

    // From here, resources can be created that depend on ourPlatformImpl
    ourPlatformImpl->InitInternalResources();
  }
//---------------------------------------------------------------------------//
  void RenderCore::Init_1_Services()
  {
    ASSERT(ourPlatformImpl != nullptr);

    ourShaderFileWatcher = std::make_unique<FileWatcher>();
    std::function<void(const String&)> onUpdatedFn(&RenderCore::OnShaderFileUpdated);
    ourShaderFileWatcher->myOnFileUpdated.Connect(onUpdatedFn);

    std::function<void(const String&)> onDeletedFn(&RenderCore::OnShaderFileDeletedMoved);
    ourShaderFileWatcher->myOnFileDeletedMoved.Connect(onDeletedFn);

    ourShaderCompiler.reset(ourPlatformImpl->CreateShaderCompiler());

    {
      ShaderVertexInputLayout& modelVertexLayout = ShaderVertexInputLayout::ourDefaultModelLayout;
      modelVertexLayout.myVertexInputElements.clear();

      uint registerIndex = 0u;
      ShaderVertexInputElement* elem = &modelVertexLayout.addVertexInputElement();
      elem->myName = "Position";
      elem->mySemantics = VertexSemantics::POSITION;
      elem->myFormat = DataFormat::RGB_32F;
      elem->myRegisterIndex = registerIndex++;
      elem->mySizeBytes = 12;

      elem = &modelVertexLayout.addVertexInputElement();
      elem->myName = "Normal";
      elem->mySemantics = VertexSemantics::NORMAL;
      elem->myFormat = DataFormat::RGB_32F;
      elem->myRegisterIndex = registerIndex++;
      elem->mySizeBytes = 12;

      elem = &modelVertexLayout.addVertexInputElement();
      elem->myName = "Tangent";
      elem->mySemantics = VertexSemantics::TANGENT;
      elem->myFormat = DataFormat::RGB_32F;
      elem->myRegisterIndex = registerIndex++;
      elem->mySizeBytes = 12;

      elem = &modelVertexLayout.addVertexInputElement();
      elem->myName = "Bitangent";
      elem->mySemantics = VertexSemantics::BITANGENT;
      elem->myFormat = DataFormat::RGB_32F;
      elem->myRegisterIndex = registerIndex++;
      elem->mySizeBytes = 12;

      elem = &modelVertexLayout.addVertexInputElement();
      elem->myName = "Texcoord";
      elem->mySemantics = VertexSemantics::TEXCOORD;
      elem->myFormat = DataFormat::RG_32F;
      elem->myRegisterIndex = registerIndex++;
      elem->mySizeBytes = 8;
    }
  }
//---------------------------------------------------------------------------//
  void RenderCore::Init_2_Resources()
  {
    ASSERT(ourPlatformImpl != nullptr);

    {
      TextureParams params;
      params.myIsExternalTexture = false;
      params.eFormat = DataFormat::SRGB_8;
      params.u16Height = 1u;
      params.u16Width = 1u;
      params.myInternalRefIndex = (uint)TextureRef::DEFAULT_DIFFUSE;

      TextureUploadData data(params);
      uint8 color[3] = { 0, 0, 0 };
      data.myData = color;

      ourDefaultDiffuseTexture = CreateTexture(params, &data, 1);

      params.myInternalRefIndex = (uint)TextureRef::DEFAULT_SPECULAR;
      ourDefaultSpecularTexture = CreateTexture(params, &data, 1);
    }

    {
      TextureParams params;
      params.myIsExternalTexture = false;
      params.eFormat = DataFormat::RGB_8;
      params.u16Height = 1u;
      params.u16Width = 1u;
      params.myInternalRefIndex = (uint)TextureRef::DEFAULT_NORMAL;

      TextureUploadData data(params);
      uint8 color[3] = { 128, 128, 128 };
      data.myData = color;

      ourDefaultNormalTexture = CreateTexture(params, &data, 1);
    }

    ourTextureCache.insert(std::make_pair(ourDefaultDiffuseTexture->GetDescription().GetHash(), ourDefaultDiffuseTexture));
    ourTextureCache.insert(std::make_pair(ourDefaultSpecularTexture->GetDescription().GetHash(), ourDefaultSpecularTexture));
    ourTextureCache.insert(std::make_pair(ourDefaultNormalTexture->GetDescription().GetHash(), ourDefaultNormalTexture));

    ourDefaultDepthStencilState = CreateDepthStencilState(DepthStencilStateDesc::GetDefaultDepthNoStencil());
    ASSERT(ourDefaultDepthStencilState != nullptr);

    ourDefaultBlendState = CreateBlendState(BlendStateDesc::GetDefaultSolid());
    ASSERT(ourDefaultBlendState != nullptr);
  }
//---------------------------------------------------------------------------//
  void RenderCore::Shutdown_0_Resources()
  {
    ASSERT(ourRenderContextPool.size() == ourAvailableRenderContexts.size(), "There are still some rendercontexts in flight");
    ourAvailableRenderContexts.clear();
    ourRenderContextPool.clear();

    ASSERT(ourComputeContextPool.size() == ourAvailableComputeContexts.size(), "There are still some compute contexts in flight");
    ourAvailableComputeContexts.clear();
    ourComputeContextPool.clear();

#define CHECK_UNIQUE_PTRS(aCollection, aName) \
    for (const auto& entry : aCollection) \
      ASSERT(entry.second.unique(), "Dangling reference found when trying to delete % cache", aName); 

    ourDefaultDiffuseTexture.reset();
    ourDefaultNormalTexture.reset();
    ourDefaultSpecularTexture.reset();

    ourDefaultDepthStencilState.reset();
    ourDefaultBlendState.reset();

    CHECK_UNIQUE_PTRS(ourGpuProgramPipelineCache, "Gpu program pipeline");
    ourGpuProgramPipelineCache.clear();

    CHECK_UNIQUE_PTRS(ourShaderCache,  "Shader");
    ourShaderCache.clear();

    CHECK_UNIQUE_PTRS(ourTextureCache, "Texture");
    ourTextureCache.clear();

    CHECK_UNIQUE_PTRS(ourMeshCache, "Mesh");
    ourMeshCache.clear();

    CHECK_UNIQUE_PTRS(ourBlendStateCache, "BlendState");
    ourBlendStateCache.clear();

    CHECK_UNIQUE_PTRS(ourDepthStencilStateCache, "DepthStencilState");
    ourDepthStencilStateCache.clear();
    
#undef CHECK_UNIQUE_PTRS
  }
//---------------------------------------------------------------------------//  
  void RenderCore::Shutdown_1_Services()
  {
    ourShaderFileWatcher.reset();
  }
//---------------------------------------------------------------------------//
  void RenderCore::Shutdown_2_Platform()
  {
    ourPlatformImpl.reset();
  }
//---------------------------------------------------------------------------//
  SharedPtr<RenderOutput> RenderCore::CreateRenderOutput(void* aNativeInstanceHandle)
  {
    SharedPtr<RenderOutput> output;
    output.reset(ourPlatformImpl->CreateRenderOutput(aNativeInstanceHandle));
    return output;
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuProgram> RenderCore::CreateGpuProgram(const GpuProgramDesc& aDesc)
  {
    uint64 hash = aDesc.GetHash();

    auto it = ourShaderCache.find(hash);
    if (it != ourShaderCache.end())
      return it->second;

    SharedPtr<GpuProgram> program(ourPlatformImpl->CreateGpuProgram());
    if (program->SetFromDescription(aDesc, ourShaderCompiler.get()))
    {
      ourShaderCache.insert(std::make_pair(hash, program));

      const String actualShaderPath =
        Resources::FindPath(ourShaderCompiler->ResolvePlatformShaderPath(aDesc.myShaderFileName));

      ourShaderFileWatcher->AddFileWatch(actualShaderPath);

      return program;
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuProgramPipeline> RenderCore::CreateGpuProgramPipeline(const GpuProgramPipelineDesc& aDesc)
  {
    uint64 hash = aDesc.GetHash();

    auto it = ourGpuProgramPipelineCache.find(hash);
    if (it != ourGpuProgramPipelineCache.end())
      return it->second;

    std::array<SharedPtr<GpuProgram>, (uint)ShaderStage::NUM> pipelinePrograms{ nullptr };
    for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
    {
      if (!aDesc.myGpuPrograms[i].myShaderFileName.empty())
        pipelinePrograms[i] = CreateGpuProgram(aDesc.myGpuPrograms[i]);
    }

    SharedPtr<GpuProgramPipeline> pipeline(new GpuProgramPipeline);
    pipeline->SetFromShaders(pipelinePrograms);

    ourGpuProgramPipelineCache.insert(std::make_pair(hash, pipeline));

    return pipeline;
  }
//---------------------------------------------------------------------------//
  DataFormat RenderCore::ResolveFormat(DataFormat aFormat)
  {
    return ourPlatformImpl->ResolveFormat(aFormat);
  }
//---------------------------------------------------------------------------//
  SharedPtr<Texture> RenderCore::GetTexture(uint64 aDescHash)
  {
    auto it = ourTextureCache.find(aDescHash);
    if (it != ourTextureCache.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuProgram> RenderCore::GetGpuProgram(uint64 aDescHash)
  {
    auto it = ourShaderCache.find(aDescHash);
    if (it != ourShaderCache.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuProgramPipeline> RenderCore::GetGpuProgramPipeline(uint64 aDescHash)
  {
    auto it = ourGpuProgramPipelineCache.find(aDescHash);
    if (it != ourGpuProgramPipelineCache.end())
      return it->second;

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<BlendState> RenderCore::CreateBlendState(const BlendStateDesc& aDesc)
  {
    auto it = ourBlendStateCache.find(aDesc.GetHash());
    if (it != ourBlendStateCache.end())
      return it->second;

    SharedPtr<BlendState> blendState(FANCY_NEW(BlendState, MemoryCategory::GENERAL));
    blendState->SetFromDescription(aDesc);

    ourBlendStateCache.insert(std::make_pair(aDesc.GetHash(), blendState));
    return blendState;
  }
//---------------------------------------------------------------------------//
  SharedPtr<DepthStencilState> RenderCore::CreateDepthStencilState(const DepthStencilStateDesc& aDesc)
  {
    auto it = ourDepthStencilStateCache.find(aDesc.GetHash());
    if (it != ourDepthStencilStateCache.end())
      return it->second;

    SharedPtr<DepthStencilState> depthStencilState(FANCY_NEW(DepthStencilState, MemoryCategory::GENERAL));
    depthStencilState->SetFromDescription(aDesc);

    ourDepthStencilStateCache.insert(std::make_pair(aDesc.GetHash(), depthStencilState));
    return depthStencilState;
  }
//---------------------------------------------------------------------------//
  const SharedPtr<BlendState>& RenderCore::GetDefaultBlendState()
  {
    return ourDefaultBlendState;
  }
//---------------------------------------------------------------------------//
  const SharedPtr<DepthStencilState>& RenderCore::GetDefaultDepthStencilState()
  {
    return ourDefaultDepthStencilState;
  }
//---------------------------------------------------------------------------//
  RenderCore_Platform* RenderCore::GetPlatform()
  {
    return ourPlatformImpl.get();
  }
//---------------------------------------------------------------------------//
  SharedPtr<Mesh> RenderCore::GetMesh(uint64 aVertexIndexHash)
  {
    auto it = ourMeshCache.find(aVertexIndexHash);
    if (it != ourMeshCache.end())
      return it->second;

    SharedPtr<Mesh> mesh(FANCY_NEW(Mesh, MemoryCategory::GEOMETRY));
    if (BinaryCache::read(mesh, aVertexIndexHash, 0u))
    {
      ourMeshCache.insert(std::make_pair(aVertexIndexHash, mesh));
      return mesh;
    }

    return nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Mesh> RenderCore::CreateMesh(const MeshDesc& aDesc,
    const std::vector<void*>& someVertexDatas, const std::vector<void*>& someIndexDatas,
    const std::vector<uint>& someNumVertices, const std::vector<uint>& someNumIndices)
  {
    SharedPtr<Mesh> mesh = GetMesh(aDesc.myVertexAndIndexHash);
    if (mesh != nullptr)
      return mesh;

    const uint numSubMeshes = (uint) aDesc.myVertexLayouts.size();
    ASSERT(numSubMeshes == someVertexDatas.size() && numSubMeshes == someIndexDatas.size());

    DynamicArray<GeometryData*> vGeometryDatas;

    for (uint iSubMesh = 0u; iSubMesh < numSubMeshes; ++iSubMesh)
    {
      GeometryData* pGeometryData = FANCY_NEW(GeometryData, MemoryCategory::GEOMETRY);

      const GeometryVertexLayout& vertexLayout = aDesc.myVertexLayouts[iSubMesh];

      // Construct the vertex buffer
      void* ptrToVertexData = someVertexDatas[iSubMesh];
      uint numVertices = someNumVertices[iSubMesh];

      SharedPtr<GpuBuffer> vertexBuffer(ourPlatformImpl->CreateGpuBuffer());

      GpuBufferCreationParams bufferParams;
      bufferParams.bIsMultiBuffered = false;
      bufferParams.myUsageFlags = static_cast<uint>(GpuBufferUsage::VERTEX_BUFFER);
      bufferParams.uAccessFlags = static_cast<uint>(GpuResourceAccessFlags::NONE);
      bufferParams.uNumElements = numVertices;
      bufferParams.uElementSizeBytes = vertexLayout.myStride;

      vertexBuffer->Create(bufferParams, ptrToVertexData);
      pGeometryData->setVertexLayout(vertexLayout);
      pGeometryData->setVertexBuffer(vertexBuffer);

      // Construct the index buffer
      void* ptrToIndexData = someIndexDatas[iSubMesh];
      uint numIndices = someNumIndices[iSubMesh];

      SharedPtr<GpuBuffer> indexBuffer(ourPlatformImpl->CreateGpuBuffer());

      GpuBufferCreationParams indexBufParams;
      indexBufParams.bIsMultiBuffered = false;
      indexBufParams.myUsageFlags = static_cast<uint>(GpuBufferUsage::INDEX_BUFFER);
      indexBufParams.uAccessFlags = static_cast<uint>(GpuResourceAccessFlags::NONE);
      indexBufParams.uNumElements = numIndices;
      indexBufParams.uElementSizeBytes = sizeof(uint);

      indexBuffer->Create(indexBufParams, ptrToIndexData);
      pGeometryData->setIndexBuffer(indexBuffer);

      vGeometryDatas.push_back(pGeometryData);
    }

    mesh = SharedPtr<Mesh>(FANCY_NEW(Mesh, MemoryCategory::GEOMETRY));
    mesh->setGeometryDataList(vGeometryDatas);
    mesh->SetVertexIndexHash(aDesc.myVertexAndIndexHash);

    ourMeshCache.insert(std::make_pair(aDesc.myVertexAndIndexHash, mesh));
    BinaryCache::write(mesh, someVertexDatas, someIndexDatas);

    return mesh;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Texture> RenderCore::CreateTexture(const TextureDesc& aTextureDesc)
  {
    if (aTextureDesc.IsEmpty())
      return nullptr;

    TextureDesc desc = aTextureDesc;

    if (aTextureDesc.myIsExternalTexture)
    {
      String texPathAbs = aTextureDesc.mySourcePath;
      String texPathRel = aTextureDesc.mySourcePath;
      if (!Path::IsPathAbsolute(texPathAbs))
        texPathAbs = Resources::FindPath(texPathAbs);
      else
        texPathRel = Resources::FindName(texPathAbs);

      desc.mySourcePath = texPathAbs;
    }

    auto it = ourTextureCache.find(desc.GetHash());
    if (it != ourTextureCache.end())
      return it->second;

    ASSERT(aTextureDesc.myIsExternalTexture,
      "Couldn't find internal texture with refIndex %",
      aTextureDesc.myInternalRefIndex);

    return CreateTexture(desc.mySourcePath);
  }
//---------------------------------------------------------------------------//
  SharedPtr<Texture> RenderCore::CreateTexture(const String& aTexturePath)
  {
    String texPathAbs = aTexturePath;
    String texPathRel = aTexturePath;
    if (!Path::IsPathAbsolute(aTexturePath))
      texPathAbs = Resources::FindPath(texPathAbs);
    else
      texPathRel = Resources::FindName(texPathAbs);

    TextureDesc desc;
    desc.mySourcePath = texPathAbs;
    desc.myIsExternalTexture = true;
    uint64 hash = desc.GetHash();

    auto it = ourTextureCache.find(hash);
    if (it != ourTextureCache.end())
      return it->second;

    std::vector<uint8> textureBytes;
    TextureLoadInfo textureInfo;
    if (!TextureLoader::loadTexture(texPathAbs, textureBytes, textureInfo))
    {
      LOG_ERROR("Failed to load texture at path %", texPathAbs);
      return nullptr;
    }

    if (textureInfo.bitsPerPixel / textureInfo.numChannels != 8u)
    {
      LOG_ERROR("Unsupported texture format: %", texPathAbs);
      return nullptr;
    }

    SharedPtr<Texture> texture(ourPlatformImpl->CreateTexture());

    TextureParams texParams;
    texParams.myIsExternalTexture = true;
    texParams.path = texPathRel;
    texParams.bIsDepthStencil = false;
    texParams.eFormat = textureInfo.numChannels == 3u ? DataFormat::SRGB_8 : DataFormat::SRGB_8_A_8;
    texParams.u16Width = textureInfo.width;
    texParams.u16Height = textureInfo.height;
    texParams.u16Depth = 0u;
    texParams.uAccessFlags = (uint)GpuResourceAccessFlags::NONE;

    TextureUploadData uploadData;
    uploadData.myData = &textureBytes[0];
    uploadData.myPixelSizeBytes = textureInfo.bitsPerPixel / 8u;
    uploadData.myRowSizeBytes = textureInfo.width * uploadData.myPixelSizeBytes;
    uploadData.mySliceSizeBytes = textureInfo.width * textureInfo.height * uploadData.myPixelSizeBytes;
    uploadData.myTotalSizeBytes = uploadData.mySliceSizeBytes;

    texture->Create(texParams, &uploadData, 1u);

    if (!texture->IsValid())
    {
      LOG_ERROR("Failed to upload pixel data of texture %", texPathAbs);
      return nullptr;
    }

    BinaryCache::write(texture, uploadData);
    ourTextureCache.insert(std::make_pair(hash, texture));

    return texture;
  }
//---------------------------------------------------------------------------//
  SharedPtr<Texture> RenderCore::CreateTexture(const TextureParams& someParams, TextureUploadData* someUploadDatas, uint aNumUploadDatas)
  {
    SharedPtr<Texture> texture(ourPlatformImpl->CreateTexture());
    texture->Create(someParams, someUploadDatas, aNumUploadDatas);

    return texture->IsValid() ? texture : nullptr;
  }
//---------------------------------------------------------------------------//
  SharedPtr<GpuBuffer> RenderCore::CreateBuffer(const GpuBufferCreationParams& someParams, void* someInitialData /* = nullptr */)
  {
    SharedPtr<GpuBuffer> buffer(ourPlatformImpl->CreateGpuBuffer());
    buffer->Create(someParams, someInitialData);
    return buffer->IsValid() ? buffer : nullptr;
  }
//---------------------------------------------------------------------------//
  CommandContext* RenderCore::AllocateContext(CommandListType aType)
  {
    ASSERT(aType == CommandListType::Graphics || aType == CommandListType::Compute,
      "CommandContext type % not implemented", (uint)aType);

    std::vector<std::unique_ptr<CommandContext>>& contextPool =
      aType == CommandListType::Graphics ? ourRenderContextPool : ourComputeContextPool;

    std::list<CommandContext*>& availableContextList =
      aType == CommandListType::Graphics ? ourAvailableRenderContexts : ourAvailableComputeContexts;

    if (!availableContextList.empty())
    {
      CommandContext* context = availableContextList.front();
      context->Reset();
      availableContextList.pop_front();
      return context;
    }

    contextPool.push_back(std::unique_ptr<CommandContext>(ourPlatformImpl->CreateContext(aType)));

    return contextPool.back().get();
  }
  //---------------------------------------------------------------------------//
  void RenderCore::FreeContext(CommandContext* aContext)
  {
    CommandListType type = aContext->GetType();

    ASSERT(type == CommandListType::Graphics || type == CommandListType::Compute,
      "CommandContext type % not implemented", (uint)type);

    std::list<CommandContext*>& availableContextList =
      type == CommandListType::Graphics ? ourAvailableRenderContexts : ourAvailableComputeContexts;

    if (std::find(availableContextList.begin(), availableContextList.end(), aContext)
      != availableContextList.end())
      return;
    
    availableContextList.push_back(aContext);
  }
//---------------------------------------------------------------------------//
  void RenderCore::UpdateBufferData(GpuBuffer* aBuffer, void* aData, uint aDataSizeBytes, uint aByteOffsetFromBuffer /* = 0 */)
  {
    ASSERT(aByteOffsetFromBuffer + aDataSizeBytes <= aBuffer->GetSizeBytes());

    const GpuBufferCreationParams& bufParams = aBuffer->GetParameters();

    if (bufParams.uAccessFlags & (uint)GpuResourceAccessFlags::WRITE)
    {
      uint8* dest = static_cast<uint8*>(aBuffer->Lock(GpuResoruceLockOption::WRITE));
      ASSERT(dest != nullptr);
      memcpy(dest + aByteOffsetFromBuffer, aData, aDataSizeBytes);
      aBuffer->Unlock();
    }
    else
    {
      CommandContext* updateContext = AllocateContext(CommandListType::Graphics);
      ourPlatformImpl->UpdateBufferData(aBuffer, aData, aByteOffsetFromBuffer, aDataSizeBytes, updateContext);
      FreeContext(updateContext);
    }
  }
//---------------------------------------------------------------------------//
  void RenderCore::InitTextureData(Texture* aTexture, const TextureUploadData* someUploadDatas, uint aNumUploadDatas)
  {
    CommandContext* initContext = AllocateContext(CommandListType::Graphics);
    ourPlatformImpl->InitTextureData(aTexture, someUploadDatas, aNumUploadDatas, initContext);
    FreeContext(initContext);
  }
//---------------------------------------------------------------------------//
  void RenderCore::InitBufferData(GpuBuffer* aBuffer, void* aDataPtr)
  {
    CommandContext* initContext = AllocateContext(CommandListType::Graphics);
    ourPlatformImpl->InitBufferData(aBuffer, aDataPtr, initContext);
    FreeContext(initContext);
  }
//---------------------------------------------------------------------------//
  void RenderCore::OnShaderFileUpdated(const String& aShaderFile)
  {
    // Find GpuPrograms for this file
    std::vector<GpuProgram*> programsToRecompile;
    for (auto it = ourShaderCache.begin(); it != ourShaderCache.end(); ++it)
    {
      GpuProgram* program = it->second.get();

      const GpuProgramDesc& desc = program->GetDescription();
      String actualShaderPath =
        Resources::FindPath(ourShaderCompiler->ResolvePlatformShaderPath(desc.myShaderFileName));

      if (actualShaderPath == aShaderFile)
        programsToRecompile.push_back(program);
    }

    for (GpuProgram* program : programsToRecompile)
      program->SetFromDescription(program->GetDescription(), ourShaderCompiler.get());


    // Check which pipelines need to be updated...
    std::vector<GpuProgramPipeline*> changedPipelines;
    for (auto it = ourGpuProgramPipelineCache.begin(); it != ourGpuProgramPipelineCache.end(); ++it)
    {
      GpuProgramPipeline* pipeline = it->second.get();

      for (GpuProgram* changedProgram : programsToRecompile)
      {
        const uint stage = static_cast<uint>(changedProgram->myStage);
        if (changedProgram == pipeline->myGpuPrograms[stage].get())
        {
          changedPipelines.push_back(pipeline);
          break;
        }
      }
    }

    for (GpuProgramPipeline* pipeline : changedPipelines)
    {
      pipeline->UpdateResourceInterface();
      pipeline->UpdateShaderByteCodeHash();
    }
  }
//---------------------------------------------------------------------------//
  void RenderCore::OnShaderFileDeletedMoved(const String& aShaderFile)
  {
  }
//---------------------------------------------------------------------------//
}

