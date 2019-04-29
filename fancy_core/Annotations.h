#pragma once

#include "FancyCoreDefines.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct AnnotationTagData
  {
    char myName[260] = "Default";
    uint myColor = 0xFF7C7C7C;
  };
//---------------------------------------------------------------------------//
  namespace Annotations
  {
    uint8 CreateTag(const char* aName, uint aColor);
    const AnnotationTagData& GetTagData(uint8 aTag);
  }
//---------------------------------------------------------------------------//
}

#define ANNOTATION_CREATE_TAG(anId, aName, aColor) \
  const uint8 anId = Fancy::Annotations::CreateTag(aName, aColor);

#define ANNOTATION_USE_TAG(anId) \
  extern const uint8 anId;