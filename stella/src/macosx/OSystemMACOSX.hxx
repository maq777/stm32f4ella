//============================================================================
//
//   SSSS    tt          lll  lll
//  SS  SS   tt           ll   ll
//  SS     tttttt  eeee   ll   ll   aaaa
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2018 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//============================================================================

#ifndef OSYSTEM_MACOSX_HXX
#define OSYSTEM_MACOSX_HXX

#include "OSystem.hxx"

/**
  This class defines UNIX-like OS's (MacOS X) system specific settings.

  @author  Mark Grebe
*/
class OSystemMACOSX : public OSystem
{
  public:
    /**
      Create a new MACOSX-specific operating system object
    */
    OSystemMACOSX();
    virtual ~OSystemMACOSX() = default;

    /**
      Returns the default paths for loading/saving files.
    */
    string defaultSaveDir() const override;
    string defaultLoadDir() const override;

  private:
    // Following constructors and assignment operators not supported
    OSystemMACOSX(const OSystemMACOSX&) = delete;
    OSystemMACOSX(OSystemMACOSX&&) = delete;
    OSystemMACOSX& operator=(const OSystemMACOSX&) = delete;
    OSystemMACOSX& operator=(OSystemMACOSX&&) = delete;
};

#endif
