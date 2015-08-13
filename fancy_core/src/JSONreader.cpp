#include "JSONreader.h"
#include "GpuProgram.h"

#include "MaterialPass.h"
#include "Material.h"
#include "GpuProgram.h"
#include "Model.h"
#include "Mesh.h"
#include "SubModel.h"

namespace Fancy { namespace IO {
  Json::Value nullVal = Json::Value(NULL);
//---------------------------------------------------------------------------//
  JSONreader::JSONreader(const String& anArchivePath) : Serializer(ESerializationMode::LOAD)
  {
    uint32 archiveFlags = 0u;

    archiveFlags |= std::ios::in;

    String archivePath = anArchivePath + ".json";
    myArchive.open(archivePath, archiveFlags);

    if (myArchive.good())
    {
      myArchive >> myDocumentVal;
    }

    myTypeStack.push(&myDocumentVal);
    //JSONreader::beginName("Root", false);
    loadHeader();
  }
//---------------------------------------------------------------------------//
  JSONreader::~JSONreader()
  {
    
  }
//---------------------------------------------------------------------------//
  bool JSONreader::serializeImpl(DataType aDataType, void* anObject, const char* aName)
  {
    beginName(aName, aDataType.myBaseType == EBaseDataType::Array);

    Json::Value& currJsonVal = *myTypeStack.top();

    bool handled = true;
    switch (aDataType.myBaseType)
    {
    case EBaseDataType::Serializable:
    case EBaseDataType::SerializablePtr:
    {
      MetaTable* metaTable = static_cast<MetaTable*>(aDataType.myUserData);

      if (currJsonVal.type() == Json::nullValue)
      {
        metaTable->invalidate(anObject);
        break;
      }

      ObjectName typeName = currJsonVal["Type"].asString();
      ObjectName instanceName = currJsonVal["Name"].asString();

      metaTable->create(anObject, typeName, instanceName);

      const bool isManaged = metaTable->isManaged(anObject);
      if (isManaged)
      {
        String key = String(typeName) + "_" + String(instanceName);
        if (!wasManagedObjectLoaded(key))
        {
          metaTable->serialize(this, anObject);
          myHeader.myLoadedManagedObjects.push_back(key);
        }
      }
      else
      {
        metaTable->serialize(this, anObject);
      }
    } break;

    case EBaseDataType::Int:
    {
      *static_cast<int*>(anObject) = currJsonVal.asInt();
    } break;

    case EBaseDataType::Uint:
    {
      *static_cast<uint*>(anObject) = currJsonVal.asUInt64();
    } break;

    case EBaseDataType::Uint32:
    {
      *static_cast<uint32*>(anObject) = currJsonVal.asUInt();
    } break;

    case EBaseDataType::Float:
    {
      *static_cast<float*>(anObject) = currJsonVal.asFloat();
    } break;

    case EBaseDataType::Char:
    {
      *static_cast<char*>(anObject) = currJsonVal.asUInt();
    } break;

    case EBaseDataType::Bool:
    {
      *static_cast<bool*>(anObject) = currJsonVal.asBool();
    } break;

    case EBaseDataType::String:
    {
      *static_cast<String*>(anObject) = currJsonVal.asString();
    } break;

    case EBaseDataType::CString:
    {
      *static_cast<const char**>(anObject) = currJsonVal.asCString();
    } break;

    case EBaseDataType::ObjectName:
    {
      *static_cast<ObjectName*>(anObject) = currJsonVal.asString();
    } break;

    case EBaseDataType::Array:
    {
      MetaTableArray* arrayVtable = reinterpret_cast<MetaTableArray*>(aDataType.myUserData);
      uint numElements = currJsonVal.size();
      arrayVtable->resize(anObject, numElements);
      for (uint i = 0u; i < numElements; ++i)
        serializeImpl(arrayVtable->getElementDataType(), arrayVtable->getElement(anObject, i), nullptr);
    } break;

    case EBaseDataType::Map: break;

    case EBaseDataType::Vector3:
    {
      glm::vec3& val = *static_cast<glm::vec3*>(anObject);
      for (Json::ArrayIndex i = 0u; i < val.length(); ++i)
        val[i] = currJsonVal[i].asFloat();
    } break;

    case EBaseDataType::Vector4:
    {
      glm::vec4& val = *static_cast<glm::vec4*>(anObject);
      for (Json::ArrayIndex i = 0u; i < val.length(); ++i)
        val[i] = currJsonVal[i].asFloat();
    } break;

    case EBaseDataType::Quaternion:
    {
      glm::quat& val = *static_cast<glm::quat*>(anObject);
      for (Json::ArrayIndex i = 0u; i < val.length(); ++i)
        val[i] = currJsonVal[i].asFloat();
    } break;

    case EBaseDataType::Matrix3x3:
    {
      glm::mat3& val = *static_cast<glm::mat3*>(anObject);
      for (Json::ArrayIndex y = 0u; y < val.length(); ++y)
        for (Json::ArrayIndex x = 0u; x < val[y].length(); ++x)
          val[x][y] = currJsonVal[y * val[y].length() + x].asFloat();
    } break;

    case EBaseDataType::Matrix4x4:
    {
      glm::mat4& val = *static_cast<glm::mat4*>(anObject);
      for (Json::ArrayIndex y = 0u; y < val.length(); ++y)
        for (Json::ArrayIndex x = 0u; x < val[y].length(); ++x)
          val[x][y] = currJsonVal[y * val[y].length() + x].asFloat();
    } break;

    case EBaseDataType::None:
    default:
      handled = false;
      break;
    }

    endName();

    return handled;
  }
//---------------------------------------------------------------------------//
  void JSONreader::beginName(const char* aName, bool anIsArray)
  {
    Json::Value& parentVal = *myTypeStack.top();

    Json::Value* newVal = nullptr;
    if (parentVal.isArray())
    {
      Json::ArrayIndex& index = myArrayIndexStack.top();
      Json::Value& val = parentVal[index];
      newVal = &val;
      ++index;
    }
    else
    {
      Json::Value& val = parentVal[aName];
      newVal = &val;
    }

    if ((*newVal).type() == Json::nullValue)
      return;

    myTypeStack.push(newVal);

    if (newVal->isArray())
      myArrayIndexStack.push(0);
  }
//---------------------------------------------------------------------------//
  void JSONreader::endName()
  {
    ASSERT_M(!myTypeStack.empty(), "Mismatching number of beginType() / endType() calls");

    Json::Value& val = *myTypeStack.top();
    if (val.isArray())
      myArrayIndexStack.pop();

    myTypeStack.pop();
  }
//---------------------------------------------------------------------------//
  void JSONreader::loadHeader()
  {
    Json::Value& rootVal = *myTypeStack.top();
    myHeader.myVersion = rootVal["myVersion"].asUInt();

    serialize(&myHeader.myGpuPrograms, "myGpuPrograms");
    serialize(&myHeader.myMaterialPasses, "myMaterialPasses");
    serialize(&myHeader.myMaterials, "myMaterials");
    serialize(&myHeader.myMeshes, "myMeshes");
    serialize(&myHeader.mySubModels, "mySubModels");
    serialize(&myHeader.myModels, "myModels");

    // myHeader.myManagedObjects = rootVal["myManagedResources"];
    // beginName("myManagedResources", true);
    // 
    // for (Json::ValueIterator it = myHeader.myManagedObjects.begin(); it != myHeader.myManagedObjects.end(); ++it)
    // {
    //   Json::Value& currVal = *it;
    //   ObjectName typeName = currVal["Type"].asString();
    // 
    //   if (typeName == _N(GpuProgram))
    //   {
    //     Rendering::GpuProgram* program = nullptr;
    //     serialize(&program);
    //   }
    // }

    endName();
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  bool JSONreader::wasManagedObjectLoaded(const ObjectName& aName)
  {
    for (const ObjectName& loadedName : myHeader.myLoadedManagedObjects)
    {
      if (loadedName == aName)
        return true;
    }

    return false;
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::IO