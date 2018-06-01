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

#ifndef CARTRIDGE_CDF_HXX
#define CARTRIDGE_CDF_HXX

class System;
class Thumbulator;
#ifdef DEBUGGER_SUPPORT
  #include "CartCDFWidget.hxx"
#endif

#include "bspf.hxx"
#include "Cart.hxx"

/**
  Cartridge class used for CDF.

  There are seven 4K program banks, a 4K Display Data RAM,
  1K C Variable and Stack, and the CDF chip.
  CDF chip access is mapped to $1000 - $103F (both read and write).
  Program banks are accessible by read/write to $1FF5 - $1FFB.

  FIXME: THIS NEEDS TO BE UPDATED

  @authors: Darrell Spice Jr, Chris Walton, Fred Quimby,
            Stephen Anthony, Bradford W. Mott
*/
class CartridgeCDF : public Cartridge
{
  friend class CartridgeCDFWidget;
  friend class CartridgeRamCDFWidget;

  public:
    /**
      Create a new cartridge using the specified image

      @param image     Pointer to the ROM image
      @param size      The size of the ROM image
      @param settings  A reference to the various settings (read-only)
    */
    CartridgeCDF(const BytePtr& image, uInt32 size, const Settings& settings);
    virtual ~CartridgeCDF() = default;

  public:
    /**
      Reset device to its power-on state
    */
    void reset() override;

    /**
      Notification method invoked by the system when the console type
      has changed.  We need this to inform the Thumbulator that the
      timing has changed.

      @param timing  Enum representing the new console type
    */
    void consoleChanged(ConsoleTiming timing) override;

    /**
      Install cartridge in the specified system.  Invoked by the system
      when the cartridge is attached to it.

     @param system The system the device should install itself in
     */
    void install(System& system) override;

    /**
      Install pages for the specified bank in the system.

     @param bank The bank that should be installed in the system
    */
    bool bank(uInt16 bank) override;

    /**
      Get the current bank.
    */
    uInt16 getBank() const override;

    /**
      Query the number of banks supported by the cartridge.
    */
    uInt16 bankCount() const override;

    /**
      Patch the cartridge ROM.

      @param address  The ROM address to patch
      @param value    The value to place into the address
      @return    Success or failure of the patch operation
    */
    bool patch(uInt16 address, uInt8 value) override;

    /**
      Access the internal ROM image for this cartridge.

      @param size  Set to the size of the internal ROM image data
      @return  A pointer to the internal ROM image data
    */
    const uInt8* getImage(uInt32& size) const override;

    /**
      Save the current state of this cart to the given Serializer.

      @param out  The Serializer object to use
      @return  False on any errors, else true
    */
    bool save(Serializer& out) const override;

    /**
      Load the current state of this cart from the given Serializer.

      @param in  The Serializer object to use
      @return  False on any errors, else true
    */
    bool load(Serializer& in) override;

    /**
      Get a descriptor for the device name (used in error checking).

      @return The name of the object
    */
    string name() const override { return "CartridgeCDF"; }

    /**
      Used for Thumbulator to pass values back to the cartridge
    */
    uInt32 thumbCallback(uInt8 function, uInt32 value1, uInt32 value2) override;

#ifdef DEBUGGER_SUPPORT
    /**
      Get debugger widget responsible for accessing the inner workings
      of the cart.
    */
    CartDebugWidget* debugWidget(GuiObject* boss, const GUI::Font& lfont,
                                 const GUI::Font& nfont, int x, int y, int w, int h) override
    {
      return new CartridgeCDFWidget(boss, lfont, nfont, x, y, w, h, *this);
    }
#endif

  public:
    /**
      Get the byte at the specified address.

      @return The byte at the specified address
    */
    uInt8 peek(uInt16 address) override;

    /**
      Change the byte at the specified address to the given value

      @param address The address where the value should be stored
      @param value The value to be stored at the address
      @return  True if the poke changed the device address space, else false
    */
    bool poke(uInt16 address, uInt8 value) override;

  private:
    /**
      Sets the initial state of the DPC pointers and RAM
    */
    void setInitialState();

    /**
      Updates any data fetchers in music mode based on the number of
      CPU cycles which have passed since the last update.
    */
    void updateMusicModeDataFetchers();

    /**
      Call Special Functions
    */
    void callFunction(uInt8 value);

    uInt32 getDatastreamPointer(uInt8 index) const;
    void setDatastreamPointer(uInt8 index, uInt32 value);

    uInt32 getDatastreamIncrement(uInt8 index) const;
    void setDatastreamIncrement(uInt8 index, uInt32 value);

    uInt8 readFromDatastream(uInt8 index);

    uInt32 getWaveform(uInt8 index) const;
    uInt32 getWaveformSize(uInt8 index) const;
    uInt32 getSample();
    void setVersion();

  private:
    // The 32K ROM image of the cartridge
    uInt8 myImage[32768];

    // Pointer to the 28K program ROM image of the cartridge
    uInt8* myProgramImage;

    // Pointer to the 4K display ROM image of the cartridge
    uInt8* myDisplayImage;

    // Pointer to the 2K CDF driver image in RAM
    uInt8* myBusDriverImage;

    // The CDF 8k RAM image, used as:
    //   $0000 - 2K CDF driver
    //   $0800 - 4K Display Data
    //   $1800 - 2K C Variable & Stack
    uInt8 myCDFRAM[8192];

    // Pointer to the Thumb ARM emulator object
    unique_ptr<Thumbulator> myThumbEmulator;

    // Indicates the offset into the ROM image (aligns to current bank)
    uInt16 myBankOffset;

    // System cycle count from when the last update to music data fetchers occurred
    uInt64 myAudioCycles;

    // ARM cycle count from when the last callFunction() occurred
    uInt64 myARMCycles;

    // The audio routines in the driver run in 32-bit mode and take advantage
    // of the FIQ Shadow Registers which are not accessible to 16-bit thumb
    // code.  As such, Thumbulator does not support them.  The driver supplies a
    // few 16-bit subroutines used to pass values from 16-bit to 32-bit.  The
    // Thumbulator will trap these calls and pass the appropriate information to
    // the Cartridge Class via callFunction() so it can emulate the 32 bit audio routines.

    /* Register usage for audio:
      r8  = channel0 accumulator
      r9  = channel1 accumulator
      r10 = channel2 accumulator
      r11 = channel0 frequency
      r12 = channel1 frequency
      r13 = channel2 frequency
      r14 = timer base  */

    // The music counters, ARM FIQ shadow registers r8, r9, r10
    uInt32 myMusicCounters[3];

    // The music frequency, ARM FIQ shadow registers r11, r12, r13
    uInt32 myMusicFrequencies[3];

    // The music waveform sizes
    uInt8 myMusicWaveformSize[3];

    // Fractional CDF music, OSC clocks unused during the last update
    double myFractionalClocks;

    // Controls mode, lower nybble sets Fast Fetch, upper nybble sets audio
    // -0 = Fast Fetch ON
    // -F = Fast Fetch OFF
    // 0- = Packed Digital Sample
    // F- = 3 Voice Music
    uInt8 myMode;

    // set to address of #value if last byte peeked was A9 (LDA #)
    uInt16 myLDAimmediateOperandAddress;

    // set to address of the JMP operand if last byte peeked was 4C
    // *and* the next two bytes in ROM are 00 00
    uInt16 myJMPoperandAddress;

    uInt8 myFastJumpActive;

    // version of CDF
    uInt16 myVersion;

  private:
    // Following constructors and assignment operators not supported
    CartridgeCDF() = delete;
    CartridgeCDF(const CartridgeCDF&) = delete;
    CartridgeCDF(CartridgeCDF&&) = delete;
    CartridgeCDF& operator=(const CartridgeCDF&) = delete;
    CartridgeCDF& operator=(CartridgeCDF&&) = delete;
};

#endif
