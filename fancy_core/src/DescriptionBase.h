#pragma once

#include "FancyCorePrerequisites.h"
#include "ObjectName.h"

namespace Fancy {
//---------------------------------------------------------------------------// 
 namespace IO {
  class Serializer;
}
//---------------------------------------------------------------------------//
  struct DLLEXPORT DescriptionBase
  {
    virtual ~DescriptionBase() {}

    virtual ObjectName GetTypeName() const = 0;
    virtual uint64 GetHash() const = 0;
    virtual void Serialize(IO::Serializer* aSerializer) = 0;

    virtual bool IsEmpty() const { return false; }
  };
//---------------------------------------------------------------------------//
}