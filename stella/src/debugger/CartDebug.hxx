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

#ifndef CART_DEBUG_HXX
#define CART_DEBUG_HXX

class Settings;
class CartDebugWidget;

// Function type for CartDebug instance methods
class CartDebug;
using CartMethod = int (CartDebug::*)();

#include <map>
#include <set>
#include <list>

#include "bspf.hxx"
#include "DebuggerSystem.hxx"

class CartState : public DebuggerState
{
  public:
    ByteArray ram;    // The actual data values
    ShortArray rport; // Address for reading from RAM
    ShortArray wport; // Address for writing to RAM
    string bank;      // Current banking layout
};

class CartDebug : public DebuggerSystem
{
  // The disassembler needs special access to this class
  friend class DiStella;

  public:
    enum DisasmType {
      NONE        = 0,
      REFERENCED  = 1 << 0, /* 0x01, code somewhere in the program references it,
                               i.e. LDA $F372 referenced $F372 */
      VALID_ENTRY = 1 << 1, /* 0x02, addresses that can have a label placed in front of it.
                               A good counterexample would be "FF00: LDA $FE00"; $FF01
                               would be in the middle of a multi-byte instruction, and
                               therefore cannot be labelled. */

      // The following correspond to specific types that can be set within the
      // debugger, or specified in a Distella cfg file, and are listed in order
      // of decreasing hierarchy
      //
      CODE  = 1 << 7, // 0x80, disassemble-able code segments
      TCODE = 1 << 6, // 0x40, (tentative) disassemble-able code segments
      GFX   = 1 << 5, // 0x20, addresses loaded into GRPx registers
      PGFX  = 1 << 4, // 0x10, addresses loaded into PFx registers
      DATA  = 1 << 3, // 0x08, addresses loaded into registers other than GRPx / PFx
      ROW   = 1 << 2, // 0x04, all other addresses
      // special type for poke()
      WRITE = TCODE   // 0x40, address written to
    };
    struct DisassemblyTag {
      DisasmType type;
      uInt16 address;
      string label;
      string disasm;
      string ccount;
      string ctotal;
      string bytes;
      bool hllabel;
    };
    using DisassemblyList = vector<DisassemblyTag>;
    struct Disassembly {
      DisassemblyList list;
      int fieldwidth;
    };

    // Determine 'type' of address (ie, what part of the system accessed)
    enum AddrType {
      ADDR_TIA,
      ADDR_IO,
      ADDR_ZPRAM,
      ADDR_ROM
    };
    AddrType addressType(uInt16 addr) const;

  public:
    CartDebug(Debugger& dbg, Console& console, const OSystem& osystem);
    virtual ~CartDebug() = default;

    const DebuggerState& getState() override;
    const DebuggerState& getOldState() override { return myOldState; }

    void saveOldState() override;
    string toString() override;

    // Used to get/set the debug widget, which contains cart-specific
    // functionality
    CartDebugWidget* getDebugWidget() const { return myDebugWidget; }
    void setDebugWidget(CartDebugWidget* w) { myDebugWidget = w; }

    // Indicate that a read from write port has occurred at the specified
    // address.
    void triggerReadFromWritePort(uInt16 address);

    // Return the address at which an invalid read was performed in a
    // write port area.
    int readFromWritePort();

    // Return the base (= non-mirrored) address of the last CPU read
    int lastReadBaseAddress();
    // Return the base (= non-mirrored) address of the last CPU write
    int lastWriteBaseAddress();

    // The following two methods are meant to be used together
    // First, a call is made to disassemble(), which updates the disassembly
    // list; it will figure out when an actual complete disassembly is
    // required, and when the previous results can be used
    //
    // Later, successive calls to disassemblyList() simply return the
    // previous results; no disassembly is done in this case
    /**
      Disassemble from the given address using the Distella disassembler
      Address-to-label mappings (and vice-versa) are also determined here

      @param force  Force a re-disassembly, even if the state hasn't changed

      @return  True if disassembly changed from previous call, else false
    */
    bool disassemble(bool force = false);

    /**
      Get the results from the most recent call to disassemble()
    */
    const Disassembly& disassembly() const { return myDisassembly; }

    /**
      Determine the line in the disassembly that corresponds to the given address.

      @param address  The address to search for

      @return Line number of the address, else -1 if no such address exists
    */
    int addressToLine(uInt16 address) const;

    /**
      Disassemble from the starting address the specified number of lines.
      Note that automatic code determination is turned off for this method;

      @param start  The start address for disassembly
      @param lines  The number of disassembled lines to generate

      @return  The disassembly represented as a string
    */
    string disassemble(uInt16 start, uInt16 lines) const;

    /**
      Add a directive to the disassembler.  Directives are basically overrides
      for the automatic code determination algorithm in Distella, since some
      things can't be automatically determined.  For now, these directives
      have exactly the same syntax as in a distella configuration file.

      @param type   Currently, CODE/DATA/GFX are supported
      @param start  The start address (inclusive) to mark with the given type
      @param end    The end address (inclusive) to mark with the given type
      @param bank   Bank to which these directive apply (0 indicated current bank)

      @return  True if directive was added, else false if it was removed
    */
    bool addDirective(CartDebug::DisasmType type, uInt16 start, uInt16 end,
                      int bank = -1);

    // The following are convenience methods that query the cartridge object
    // for the desired information.
    /**
      Get the current bank in use by the cartridge
      (non-const because of use in YaccParser)
    */
    int getBank();

    /**
      Get the total number of banks supported by the cartridge.
    */
    int bankCount() const;

    /**
      Add a label and associated address.
      Labels that reference either TIA or RIOT spaces will not be processed.
    */
    bool addLabel(const string& label, uInt16 address);

    /**
      Remove the given label and its associated address.
      Labels that reference either TIA or RIOT spaces will not be processed.
    */
    bool removeLabel(const string& label);

    /**
      Accessor methods for labels and addresses

      The mapping from address to label can be one-to-many (ie, an
      address can have different labels depending on its context, and
      whether its being read or written; if isRead is true, the context
      is a read, else it's a write
      If places is not -1 and a label hasn't been defined, return a
      formatted hexidecimal address
    */
    bool getLabel(ostream& buf, uInt16 addr, bool isRead, int places = -1) const;
    string getLabel(uInt16 addr, bool isRead, int places = -1) const;
    int getAddress(const string& label) const;

    /**
      Load constants from list file (as generated by DASM).
    */
    string loadListFile();

    /**
      Load user equates from symbol file (as generated by DASM).
    */
    string loadSymbolFile();

    /**
      Load/save Distella config files (Distella directives)
    */
    string loadConfigFile();
    string saveConfigFile();

    /**
      Save disassembly and ROM file
    */
    string saveDisassembly();
    string saveRom();

    /**
      Show Distella directives (both set by the user and determined by Distella)
      for the given bank (or all banks, if no bank is specified).
    */
    string listConfig(int bank = -1);

    /**
      Clear Distella directives (set by the user) for the given bank
      (or all banks, if no bank is specified.)
    */
    string clearConfig(int bank = -1);

    /**
      Methods used by the command parser for tab-completion
      In this case, return completions from the equate list(s)
    */
    void getCompletions(const char* in, StringList& list) const;

    // Convert given address to corresponding disassembly type and append to buf
    void addressTypeAsString(ostream& buf, uInt16 addr) const;

  private:
    using AddrToLabel = std::map<uInt16, string>;
    using LabelToAddr = std::map<string, uInt16,
        std::function<bool(const string&, const string&)>>;

    struct DirectiveTag {
      DisasmType type;
      uInt16 start;
      uInt16 end;
    };
    using AddressList = std::list<uInt16>;
    using DirectiveList = std::list<DirectiveTag>;

    struct BankInfo {
      uInt16 start;                // start of address space
      uInt16 end;                  // end of address space
      uInt16 offset;               // ORG value
      uInt32 size;                 // size of a bank (in bytes)
      AddressList addressList;     // addresses which PC has hit
      DirectiveList directiveList; // overrides for automatic code determination

      BankInfo() : start(0), end(0), offset(0), size(0) { }
    };

    // Address type information determined by Distella
    uInt8 myDisLabels[0x1000], myDisDirectives[0x1000];

    // Information on equates used in the disassembly
    struct ReservedEquates {
      bool TIARead[16];
      bool TIAWrite[64];
      bool IOReadWrite[24];
      bool ZPRAM[128];
      AddrToLabel Label;
      bool breakFound;
    };
    ReservedEquates myReserved;

    // Actually call DiStella to fill the DisassemblyList structure
    // Return whether the search address was actually in the list
    bool fillDisassemblyList(BankInfo& bankinfo, uInt16 search);

    // Analyze of bank of ROM, generating a list of Distella directives
    // based on its disassembly
    void getBankDirectives(ostream& buf, BankInfo& info) const;

    // Get disassembly enum type from 'flags', taking precendence into account
    DisasmType disasmTypeAbsolute(uInt8 flags) const;

    // Convert disassembly enum type to corresponding string and append to buf
    void disasmTypeAsString(ostream& buf, DisasmType type) const;

    // Convert all disassembly types in 'flags' to corresponding string and
    // append to buf
    void disasmTypeAsString(ostream& buf, uInt8 flags) const;

  private:
    const OSystem& myOSystem;

    CartState myState;
    CartState myOldState;

    CartDebugWidget* myDebugWidget;

    // A complete record of relevant diassembly information for each bank
    vector<BankInfo> myBankInfo;

    // Used for the disassembly display, and mapping from addresses
    // to corresponding lines of text in that display
    Disassembly myDisassembly;
    std::map<uInt16, int> myAddrToLineList;
    bool myAddrToLineIsROM;

    // Mappings from label to address (and vice versa) for items
    // defined by the user (either through a DASM symbol file or manually
    // from the commandline in the debugger)
    AddrToLabel myUserLabels;
    LabelToAddr myUserAddresses;

    // Mappings from label to address (and vice versa) for constants
    // defined through a DASM lst file
    // AddrToLabel myUserCLabels;
    // LabelToAddr myUserCAddresses;

    // Mappings for labels to addresses for system-defined equates
    // Because system equate addresses can have different names
    // (depending on access in read vs. write mode), we can only create
    // a mapping from labels to addresses; addresses to labels are
    // handled differently
    LabelToAddr mySystemAddresses;

    // Holds address at which the most recent read from a write port
    // occurred
    uInt16 myRWPortAddress;

    // The maximum length of all labels currently defined
    uInt16 myLabelLength;

    // Filenames to use for various I/O (currently these are hardcoded)
    string myListFile, mySymbolFile, myCfgFile, myDisasmFile, myRomFile;

    /// Table of instruction mnemonics
    static const char* const ourTIAMnemonicR[16]; // read mode
    static const char* const ourTIAMnemonicW[64]; // write mode
    static const char* const ourIOMnemonic[24];
    static const char* const ourZPMnemonic[128];

  private:
    // Following constructors and assignment operators not supported
    CartDebug() = delete;
    CartDebug(const CartDebug&) = delete;
    CartDebug(CartDebug&&) = delete;
    CartDebug& operator=(const CartDebug&) = delete;
    CartDebug& operator=(CartDebug&&) = delete;
};

#endif
