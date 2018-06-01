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

#include "Dialog.hxx"
#include "OSystem.hxx"
#include "Version.hxx"
#include "Widget.hxx"
#include "Font.hxx"
#include "AboutDialog.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
AboutDialog::AboutDialog(OSystem& osystem, DialogContainer& parent,
                         const GUI::Font& font)
  : Dialog(osystem, parent, font, "About Stella"),
    myPage(1),
    myNumPages(4),
    myLinesPerPage(13)
{
  const int lineHeight   = font.getLineHeight(),
            fontWidth    = font.getMaxCharWidth(),
            fontHeight   = font.getFontHeight(),
            buttonWidth  = font.getStringWidth("Defaults") + 20,
            buttonHeight = font.getLineHeight() + 4;
  int xpos, ypos;
  WidgetArray wid;

  // Set real dimensions
  _w = 55 * fontWidth + 8;
  _h = 15 * lineHeight + 20 + _th;

  // Add Previous, Next and Close buttons
  xpos = 10;  ypos = _h - buttonHeight - 10;
  myPrevButton =
    new ButtonWidget(this, font, xpos, ypos, buttonWidth, buttonHeight,
                     "Previous", GuiObject::kPrevCmd);
  myPrevButton->clearFlags(WIDGET_ENABLED);
  wid.push_back(myPrevButton);

  xpos += buttonWidth + 8;
  myNextButton =
    new ButtonWidget(this, font, xpos, ypos, buttonWidth, buttonHeight,
                     "Next", GuiObject::kNextCmd);
  wid.push_back(myNextButton);

  xpos = _w - buttonWidth - 10;
  ButtonWidget* b =
    new ButtonWidget(this, font, xpos, ypos, buttonWidth, buttonHeight,
                     "Close", GuiObject::kCloseCmd);
  wid.push_back(b);
  addCancelWidget(b);

  xpos = 5;  ypos = 5 + _th;
  myTitle = new StaticTextWidget(this, font, xpos, ypos, _w - xpos * 2, fontHeight,
                                 "", TextAlign::Center);
  myTitle->setTextColor(kTextColorEm);

  xpos = 16;  ypos += lineHeight + 4;
  for(int i = 0; i < myLinesPerPage; i++)
  {
    myDesc.push_back(new StaticTextWidget(this, font, xpos, ypos, _w - xpos * 2,
                                          fontHeight, "", TextAlign::Left));
    myDescStr.push_back("");
    ypos += fontHeight;
  }

  addToFocusList(wid);
}

// The following commands can be put at the start of a line (all subject to change):
//   \C, \L, \R  -- set center/left/right alignment
//   \c0 - \c5   -- set a custom color:
//                  0 normal text (green)
//                  1 highlighted text (light green)
//                  2 light border (light gray)
//                  3 dark border (dark gray)
//                  4 background (black)
//                  5 emphasized text (red)
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void AboutDialog::updateStrings(int page, int lines, string& title)
{
  int i = 0;
  auto ADD_ATEXT = [&](const string& d) { myDescStr[i] = d; i++; };
  auto ADD_ALINE = [&]() { ADD_ATEXT(""); };

  switch(page)
  {
    case 1:
      title = string("Stella ") + STELLA_VERSION;
      ADD_ATEXT("\\CA multi-platform Atari 2600 VCS emulator");
      ADD_ATEXT(string("\\C\\c2Features: ") + instance().features());
      ADD_ATEXT(string("\\C\\c2") + instance().buildInfo());
      ADD_ALINE();
      ADD_ATEXT("\\CCopyright (C) 1995-2018 The Stella Team");
      ADD_ATEXT("\\C(https://stella-emu.github.io)");
      ADD_ALINE();
      ADD_ATEXT("\\CStella is now DonationWare!");
      ADD_ATEXT("\\C(https://stella-emu.github.io/donations.html)");
      ADD_ALINE();
      ADD_ATEXT("\\CStella is free software released under the GNU GPL.");
      ADD_ATEXT("\\CSee manual for further details.");
      break;

    case 2:
      title = "The Stella Team";
      ADD_ATEXT("\\L\\c0""Stephen Anthony");
      ADD_ATEXT("\\L\\c2""  Lead developer, current maintainer for the");
      ADD_ATEXT("\\L\\c2""  Linux/OSX and Windows ports ");
      ADD_ATEXT("\\L\\c0""Christian Speckner");
      ADD_ATEXT("\\L\\c2""  Emulation core development, TIA core");
      ADD_ATEXT("\\L\\c0""Eckhard Stolberg");
      ADD_ATEXT("\\L\\c2""  Emulation core development");
      ADD_ATEXT("\\L\\c0""Thomas Jentzsch");
      ADD_ATEXT("\\L\\c2""  Emulation core development, jack-of-all-trades");
      ADD_ATEXT("\\L\\c0""Brian Watson");
      ADD_ATEXT("\\L\\c2""  Emulation core enhancement, debugger support");
      ADD_ATEXT("\\L\\c0""Bradford W. Mott");
      ADD_ATEXT("\\L\\c2""  Original author of Stella");
      break;

    case 3:
      title = "Contributors";
      ADD_ATEXT("\\L\\c0""See https://stella-emu.github.io/credits.html for");
      ADD_ATEXT("\\L\\c0""people that have contributed to Stella.");
      ADD_ALINE();
      ADD_ATEXT("\\L\\c0""Thanks to the ScummVM project for the GUI code.");
      ADD_ALINE();
      ADD_ATEXT("\\L\\c0""Thanks to Ian Bogost and the Georgia Tech Atari Team");
      ADD_ATEXT("\\L\\c0""for the CRT Simulation effects.");
      break;

    case 4:
      title = "Cast of thousands";
      ADD_ATEXT("\\L\\c0""Special thanks to AtariAge for introducing the");
      ADD_ATEXT("\\L\\c0""Atari 2600 to a whole new generation.");
      ADD_ATEXT("\\L\\c2""  http://www.atariage.com");
      ADD_ALINE();
      ADD_ATEXT("\\L\\c0""Finally, a huge thanks to the original Atari 2600");
      ADD_ATEXT("\\L\\c0""VCS team for giving us the magic, and to the");
      ADD_ATEXT("\\L\\c0""homebrew developers for keeping the magic alive.");
      break;
  }

  while(i < lines)
    ADD_ALINE();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void AboutDialog::displayInfo()
{
  string titleStr;
  updateStrings(myPage, myLinesPerPage, titleStr);

  myTitle->setLabel(titleStr);
  for(int i = 0; i < myLinesPerPage; i++)
  {
    const char* str = myDescStr[i].c_str();
    TextAlign align = TextAlign::Center;
    uInt32 color  = kTextColor;

    while (str[0] == '\\')
    {
      switch (str[1])
      {
        case 'C':
          align = TextAlign::Center;
          break;

        case 'L':
          align = TextAlign::Left;
          break;

        case 'R':
          align = TextAlign::Right;
          break;

        case 'c':
          switch (str[2])
          {
            case '0':
              color = kTextColor;
              break;
            case '1':
              color = kTextColorHi;
              break;
            case '2':
              color = kColor;
              break;
            case '3':
              color = kShadowColor;
              break;
            case '4':
              color = kBGColor;
              break;
            case '5':
              color = kTextColorEm;
              break;
            default:
              break;
          }
          str++;
          break;

        default:
          break;
      }
      str += 2;
    }

    myDesc[i]->setAlign(align);
    myDesc[i]->setTextColor(color);
    myDesc[i]->setLabel(str);
  }

  // Redraw entire dialog
  _dirty = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void AboutDialog::handleCommand(CommandSender* sender, int cmd, int data, int id)
{
  switch(cmd)
  {
    case GuiObject::kNextCmd:
      myPage++;
      if(myPage >= myNumPages)
        myNextButton->clearFlags(WIDGET_ENABLED);
      if(myPage >= 2)
        myPrevButton->setFlags(WIDGET_ENABLED);

      displayInfo();
      break;

    case GuiObject::kPrevCmd:
      myPage--;
      if(myPage <= myNumPages)
        myNextButton->setFlags(WIDGET_ENABLED);
      if(myPage <= 1)
        myPrevButton->clearFlags(WIDGET_ENABLED);

      displayInfo();
      break;

    default:
      Dialog::handleCommand(sender, cmd, data, 0);
  }
}
