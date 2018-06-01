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

#ifndef CARTRIDGE3EPLUS_WIDGET_HXX
#define CARTRIDGE3EPLUS_WIDGET_HXX

class Cartridge3EPlus;
class ButtonWidget;
class EditTextWidget;
class PopUpWidget;

#include "CartDebugWidget.hxx"

class Cartridge3EPlusWidget : public CartDebugWidget
{
  public:
    Cartridge3EPlusWidget(GuiObject* boss, const GUI::Font& lfont,
                          const GUI::Font& nfont,
                          int x, int y, int w, int h,
                          Cartridge3EPlus& cart);
    virtual ~Cartridge3EPlusWidget() = default;

  private:
    void updateUIState();

  private:
    Cartridge3EPlus& myCart;

    PopUpWidget* myBankNumber[4];
    PopUpWidget* myBankType[4];
    ButtonWidget* myBankCommit[4];
    EditTextWidget* myBankState[8];

    struct CartState {
      ByteArray internalram;
    };
    CartState myOldState;

    enum BankID {
      kBank0Changed = 'b0CH',
      kBank1Changed = 'b1CH',
      kBank2Changed = 'b2CH',
      kBank3Changed = 'b3CH'
    };
    static const BankID bankEnum[4];

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
    // end of functions for Cartridge RAM tab

    // Following constructors and assignment operators not supported
    Cartridge3EPlusWidget() = delete;
    Cartridge3EPlusWidget(const Cartridge3EPlusWidget&) = delete;
    Cartridge3EPlusWidget(Cartridge3EPlusWidget&&) = delete;
    Cartridge3EPlusWidget& operator=(const Cartridge3EPlusWidget&) = delete;
    Cartridge3EPlusWidget& operator=(Cartridge3EPlusWidget&&) = delete;
};

#endif
