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

#ifndef CARTRIDGECM_WIDGET_HXX
#define CARTRIDGECM_WIDGET_HXX

class CartridgeCM;
class CheckboxWidget;
class DataGridWidget;
class EditTextWidget;
class PopUpWidget;
class ToggleBitWidget;

#include "CartDebugWidget.hxx"

class CartridgeCMWidget : public CartDebugWidget
{
  public:
    CartridgeCMWidget(GuiObject* boss, const GUI::Font& lfont,
                      const GUI::Font& nfont,
                      int x, int y, int w, int h,
                      CartridgeCM& cart);
    virtual ~CartridgeCMWidget() = default;

  private:
    struct CartState {
      uInt8 swcha;
      uInt8 column;
      ByteArray internalram;
      uInt16 bank;
    };

    CartridgeCM& myCart;
    PopUpWidget* myBank;

    ToggleBitWidget* mySWCHA;
    DataGridWidget* myColumn;
    CheckboxWidget *myAudIn, *myAudOut, *myIncrease, *myReset;
    CheckboxWidget* myRow[4];
    CheckboxWidget *myFunc, *myShift;
    EditTextWidget* myRAM;

    CartState myOldState;

    enum { kBankChanged = 'bkCH' };

  private:
    void saveOldState() override;

    void loadConfig() override;
    void handleCommand(CommandSender* sender, int cmd, int data, int id) override;

    string bankState() override;

    // start of functions for Cartridge RAM tab
    uInt32 internalRamSize() override;
    uInt32 internalRamRPort(int start) override;
    string internalRamDescription() override;
    const ByteArray& internalRamOld(int start, int count) override;
    const ByteArray& internalRamCurrent(int start, int count) override;
    void internalRamSetValue(int addr, uInt8 value) override;
    uInt8 internalRamGetValue(int addr) override;
    string internalRamLabel(int addr) override;
    // end of functions for Cartridge RAM tab

    // Following constructors and assignment operators not supported
    CartridgeCMWidget() = delete;
    CartridgeCMWidget(const CartridgeCMWidget&) = delete;
    CartridgeCMWidget(CartridgeCMWidget&&) = delete;
    CartridgeCMWidget& operator=(const CartridgeCMWidget&) = delete;
    CartridgeCMWidget& operator=(CartridgeCMWidget&&) = delete;
};

#endif
