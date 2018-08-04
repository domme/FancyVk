#pragma once
#include "DescriptorDX12.h"
#include "Texture.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class TextureDX12 final : public Texture
  {
    friend class RenderOutputDX12;  // Remove after backbuffers are handled through the texture class

  public:
    TextureDX12();
    ~TextureDX12() override;

    void Create(const TextureProperties& someProperties, const TextureSubData* someInitialDatas = nullptr, uint aNumInitialDatas = 0u) override;
    void GetSubresourceLayout(const TextureSubLocation& aStartSubLocation, uint aNumSubDatas, DynamicArray<TextureSubLayout>& someLayoutsOut, DynamicArray<uint64>& someOffsetsOut, uint64& aTotalSizeOut) const override;
    uint GetSubresourceIndex(const TextureSubLocation& aSubresourceLocation) const override;
    TextureSubLocation GetSubresourceLocation(uint aSubresourceIndex) const override;

  protected:
    void Destroy() override;
  };
//---------------------------------------------------------------------------//
}
