#pragma once

#include "TextureSampler.h"
#include "DescriptorDX12.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class TextureSamplerDX12 : public TextureSampler
  {
  public:
    TextureSamplerDX12();
    ~TextureSamplerDX12() override;

  protected:
    DescriptorDX12 myDescriptor;

    void Create() override;
    bool IsCreated() override;
  };
//---------------------------------------------------------------------------//
}

