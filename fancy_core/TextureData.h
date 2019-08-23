#pragma once

#include "FancyCoreDefines.h"
#include "MathIncludes.h"
#include "DynamicArray.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  struct TextureProperties;
//---------------------------------------------------------------------------//
  struct TextureSubLayout
  {
    TextureSubLayout()
      : myWidth(0u)
      , myHeight(0u)
      , myDepth(0u)
      , myAlignedRowSize(0u)
      , myRowSize(0u)
      , myNumRows(0u)
    {}

    uint myWidth;
    uint myHeight;
    uint myDepth;
    uint64 myAlignedRowSize;
    uint64 myRowSize;
    uint myNumRows;
  };
//---------------------------------------------------------------------------//
  struct TextureSubLocation
  {
    TextureSubLocation(uint aMipLevel = 0u, uint anArrayIndex = 0u, uint aPlaneIndex = 0u)
      : myMipLevel(aMipLevel)
      , myArrayIndex(anArrayIndex)
      , myPlaneIndex(aPlaneIndex) 
    {}

    uint myMipLevel;
    uint myArrayIndex;
    uint myPlaneIndex;
  };
//---------------------------------------------------------------------------//
  struct TextureSubRange
  {
    TextureSubRange(
      uint aPlaneMask = (uint)TexturePlaneFlag::Color, 
      uint aFirstMipLevel = 0u, 
      uint aNumMipLevels = 1u, 
      uint aFirstArrayIndex = 0u, 
      uint aNumArrayIndices = 1u)
      : myPlaneMask(aPlaneMask)
      , myFirstMipLevel(aFirstMipLevel)
      , myNumMipLevels(aNumMipLevels)
      , myFirstArrayIndex(aFirstArrayIndex)
      , myNumArrayIndices(aNumArrayIndices)
    { }

    uint myPlaneMask;
    uint myFirstMipLevel;
    uint myNumMipLevels;
    uint myFirstArrayIndex;
    uint myNumArrayIndices;
  };
//---------------------------------------------------------------------------//
  struct TextureRegion
  {
    TextureRegion()
      : myTexelPos(glm::uvec3(0))
      , myTexelSize(glm::uvec3(UINT_MAX))
    {}

    static TextureRegion ourMaxRegion;

    glm::uvec3 myTexelPos;
    glm::uvec3 myTexelSize;
  };
//---------------------------------------------------------------------------//
  struct TextureSubData
  {
    TextureSubData()
      : myData(nullptr)
      , myPixelSizeBytes(0u)
      , myRowSizeBytes(0u)
      , mySliceSizeBytes(0u)
      , myTotalSizeBytes(0u)
    {}

    TextureSubData(const TextureProperties& someProperties);

    uint8* myData;
    uint64 myPixelSizeBytes;
    uint64 myRowSizeBytes;
    uint64 mySliceSizeBytes;
    uint64 myTotalSizeBytes;
  };
//---------------------------------------------------------------------------//
  struct TextureData
  {
    TextureData() = default;
    TextureData(DynamicArray<uint8> someData, DynamicArray<TextureSubData> someSubDatas)
      : myData(std::move(someData))
      , mySubDatas(std::move(someSubDatas))
    {}
    DynamicArray<uint8> myData;
    DynamicArray<TextureSubData> mySubDatas;
  };
//---------------------------------------------------------------------------//
}