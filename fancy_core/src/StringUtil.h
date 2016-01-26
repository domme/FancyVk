#ifndef INCLUDE_STRINGUTIL_H
#define INCLUDE_STRINGUTIL_H

#include <list>

#include "FancyCorePrerequisites.h"

namespace Fancy {

  class StringUtil
  {
  public:
//---------------------------------------------------------------------------//
    static std::wstring ToWideString(const String& aStr)
    {
      std::wstring wstr;
      wstr.resize(aStr.size());

      for (uint i = 0u; i < aStr.size(); ++i)
      {
        wstr[i] = aStr[i];
      }

      return wstr;
    }
//---------------------------------------------------------------------------//
    template<class T>
    static std::string toString(const T& _val)
    {
      std::stringstream ss;
      ss << _val;
      return ss.str();
    }
//---------------------------------------------------------------------------//
    static void tokenize(const String& _str, const char* _szDelimiters, std::list<String>& _outTokenList )
    {
      // Create a temporary string we can "destroy" in the process
      String str = _str;
      char* cstr = const_cast<char*>(str.c_str());
      cstr = strtok(cstr, _szDelimiters);
      
      while(cstr != nullptr)
      {
        _outTokenList.push_back(cstr);
        cstr = strtok(NULL, _szDelimiters);
      }
    }
//---------------------------------------------------------------------------//
  };

}  // end of namespace Fancy


#endif  // INCLUDE_STRINGUTIL_H