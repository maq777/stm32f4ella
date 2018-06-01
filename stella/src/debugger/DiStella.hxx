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

#ifndef DISTELLA_HXX
#define DISTELLA_HXX

#include <queue>

#include "Base.hxx"
#include "CartDebug.hxx"
#include "bspf.hxx"

/**
  This class is a wrapper around the Distella code.  Much of the code remains
  exactly the same, except that generated data is now redirected to a
  DisassemblyList structure rather than being printed.

  All 7800-related stuff has been removed, as well as some commandline options.
  Over time, some of the configurability of Distella may be added again.

  @authors  Stephen Anthony and Thomas Jentzsch
            Original distella developers (http://distella.sf.net)
*/
class DiStella
{
  public:
    // A list of options that can be applied to the disassembly
    // This will eventually grow to include all options supported by
    // standalone Distella
    struct Settings {
      Common::Base::Format gfxFormat;
      bool resolveCode;    // Attempt to detect code vs. data sections
      bool showAddresses;  // Show PC addresses (always off for external output)
      bool aFlag;  // Turns 'A' off in accumulator instructions (-a in Distella)
      bool fFlag;  // Forces correct address length (-f in Distella)
      bool rFlag;  // Relocate calls out of address range (-r in Distella)
      bool bFlag;  // Process break routine (-b in Distella)
      int bytesWidth;  // Number of bytes to use per line (with .byte xxx)
    };
    static Settings settings;  // Default settings

  public:
    /**
      Disassemble the current state of the System from the given start address.

      @param dbg         The CartDebug instance containing all label information
      @param list        The results of the disassembly are placed here
      @param info        Various info about the current bank
      @param settings    The various distella flags/options to use
      @param labels      Array storing label info determined by Distella
      @param directives  Array storing directive info determined by Distella
      @param reserved    The TIA/RIOT addresses referenced in the disassembled code
    */
    DiStella(const CartDebug& dbg, CartDebug::DisassemblyList& list,
             CartDebug::BankInfo& info, const DiStella::Settings& settings,
             uInt8* labels, uInt8* directives,
             CartDebug::ReservedEquates& reserved);

  private:
    // Indicate that a new line of disassembly has been completed
    // In the original Distella code, this indicated a new line to be printed
    // Here, we add a new entry to the DisassemblyList
    void addEntry(CartDebug::DisasmType type);

    // Process directives given in the list
    // Directives are basically the contents of a distella configuration file
    void processDirectives(const CartDebug::DirectiveList& directives);

    // These functions are part of the original Distella code
    void disasm(uInt32 distart, int pass);
    void disasmPass1(CartDebug::AddressList& debuggerAddresses);
    void disasmFromAddress(uInt32 distart);

    bool check_range(uInt16 start, uInt16 end) const;
    int mark(uInt32 address, uInt8 mask, bool directive = false);
    bool checkBit(uInt16 address, uInt8 mask, bool useDebugger = true) const;

    bool checkBits(uInt16 address, uInt8 mask, uInt8 notMask, bool useDebugger = true) const;
    void outputGraphics();
    void outputBytes(CartDebug::DisasmType type);

    // Convenience methods to generate appropriate labels
    inline void labelA12High(stringstream& buf, uInt8 op, uInt16 addr, int labfound)
    {
      if(!myDbg.getLabel(buf, addr, true))
        buf << "L" << Common::Base::HEX4 << addr;
    }
    inline void labelA12Low(stringstream& buf, uInt8 op, uInt16 addr, int labfound)
    {
      myDbg.getLabel(buf, addr, ourLookup[op].rw_mode == READ, 2);
      if (labfound == 2)
      {
        if(ourLookup[op].rw_mode == READ)
          myReserved.TIARead[addr & 0x0F] = true;
        else
          myReserved.TIAWrite[addr & 0x3F] = true;
      }
      else if (labfound == 3)
        myReserved.IOReadWrite[(addr & 0xFF) - 0x80] = true;
      else if (labfound == 5)
        myReserved.ZPRAM[(addr & 0xFF) - 0x80] = true;
    }

  private:
    const CartDebug& myDbg;
    CartDebug::DisassemblyList& myList;
    const Settings& mySettings;
    CartDebug::ReservedEquates& myReserved;
    stringstream myDisasmBuf;
    std::queue<uInt16> myAddressQueue;
    uInt16 myOffset, myPC, myPCEnd;
    uInt16 mySegType;

    struct resource {
      uInt16 start;
      uInt16 end;
      uInt16 length;
    } myAppData;

    /* Stores info on how each address is marked, both in the general
       case as well as when manual directives are enabled (in which case
       the directives take priority
       The address mark type is defined in CartDebug.hxx
    */
    uInt8 *myLabels, *myDirectives;

    /**
      Enumeration of the 6502 addressing modes
    */
    enum AddressingMode
    {
      IMPLIED, ACCUMULATOR, IMMEDIATE,
      ZERO_PAGE, ZERO_PAGE_X, ZERO_PAGE_Y,
      ABSOLUTE, ABSOLUTE_X, ABSOLUTE_Y,
      ABS_INDIRECT, INDIRECT_X, INDIRECT_Y,
      RELATIVE, ASS_CODE
    };

    /**
      Enumeration of the 6502 access modes
    */
    enum AccessMode
    {
      M_NONE, M_AC, M_XR, M_YR, M_SP, M_SR, M_PC, M_IMM, M_ZERO, M_ZERX, M_ZERY,
      M_ABS, M_ABSX, M_ABSY, M_AIND, M_INDX, M_INDY, M_REL, M_FC, M_FD, M_FI,
      M_FV, M_ADDR, M_,

      M_ACIM, /* Source: AC & IMMED (bus collision) */
      M_ANXR, /* Source: AC & XR (bus collision) */
      M_AXIM, /* Source: (AC | #EE) & XR & IMMED (bus collision) */
      M_ACNC, /* Dest: M_AC and Carry = Negative */
      M_ACXR, /* Dest: M_AC, M_XR */

      M_SABY, /* Source: (ABS_Y & SP) (bus collision) */
      M_ACXS, /* Dest: M_AC, M_XR, M_SP */
      M_STH0, /* Dest: Store (src & Addr_Hi+1) to (Addr +0x100) */
      M_STH1,
      M_STH2,
      M_STH3
    };

    /**
      Enumeration of the 6502 read/write mode
      (if the opcode is reading or writing its operand)
    */
    enum ReadWriteMode
    {
      READ, WRITE, NONE
    };

    struct Instruction_tag {
      const char* const mnemonic;
      AddressingMode addr_mode;
      AccessMode     source;
      ReadWriteMode  rw_mode;
      uInt8          cycles;
      uInt8          bytes;
    };
    static const Instruction_tag ourLookup[256];

  private:
    // Following constructors and assignment operators not supported
    DiStella() = delete;
    DiStella(const DiStella&) = delete;
    DiStella(DiStella&&) = delete;
    DiStella& operator=(const DiStella&) = delete;
    DiStella& operator=(DiStella&&) = delete;
};

#endif
