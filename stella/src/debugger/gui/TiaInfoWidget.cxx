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

#include "Base.hxx"
#include "Font.hxx"
#include "OSystem.hxx"
#include "Debugger.hxx"
#include "TIADebug.hxx"
#include "Widget.hxx"
#include "EditTextWidget.hxx"
#include "GuiObject.hxx"

#include "TiaInfoWidget.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TiaInfoWidget::TiaInfoWidget(GuiObject* boss, const GUI::Font& lfont,
                             const GUI::Font& nfont,
                             int x, int y, int max_w)
  : Widget(boss, lfont, x, y, 16, 16),
    CommandSender(boss)
{
  bool longstr = 34 * lfont.getMaxCharWidth() <= max_w;

  x += 5;
  const int lineHeight = lfont.getLineHeight();
  int xpos = x, ypos = y + 10;
  int lwidth = lfont.getStringWidth(longstr ? "Frame Cycle " : "F. Cycle ");
  int fwidth = 5 * lfont.getMaxCharWidth() + 4;

  // Add frame info
  new StaticTextWidget(boss, lfont, xpos, ypos, lwidth, lineHeight,
                       longstr ? "Frame Count " : "Frame ",
                       TextAlign::Left);
  xpos += lwidth;
  myFrameCount = new EditTextWidget(boss, nfont, xpos, ypos-1, fwidth, lineHeight, "");
  myFrameCount->setEditable(false, true);

  xpos = x;  ypos += lineHeight + 5;
  new StaticTextWidget(boss, lfont, xpos, ypos, lwidth, lineHeight,
                       longstr ? "Frame Cycle " : "F. Cycle ",
                       TextAlign::Left);
  xpos += lwidth;
  myFrameCycles = new EditTextWidget(boss, nfont, xpos, ypos-1, fwidth, lineHeight, "");
  myFrameCycles->setEditable(false, true);

  xpos = x + 20;  ypos += lineHeight + 8;
  myVSync = new CheckboxWidget(boss, lfont, xpos, ypos-3, "VSync", 0);
  myVSync->setEditable(false);

  xpos = x + 20;  ypos += lineHeight + 5;
  myVBlank = new CheckboxWidget(boss, lfont, xpos, ypos-3, "VBlank", 0);
  myVBlank->setEditable(false);

  xpos = x + lwidth + myFrameCycles->getWidth() + 8;  ypos = y + 10;
  lwidth = lfont.getStringWidth(longstr ? "Color Clock " : "Pixel Pos ");
  fwidth = 3 * lfont.getMaxCharWidth() + 4;

  new StaticTextWidget(boss, lfont, xpos, ypos,
    lfont.getStringWidth(longstr ? "Scanline" : "Scn Ln"), lineHeight,
    longstr ? "Scanline" : "Scn Ln", TextAlign::Left);

  myScanlineCountLast = new EditTextWidget(boss, nfont, xpos+lwidth, ypos-1, fwidth,
                                       lineHeight, "");
  myScanlineCountLast->setEditable(false, true);

  myScanlineCount = new EditTextWidget(boss, nfont,
        xpos+lwidth - myScanlineCountLast->getWidth() - 2, ypos-1, fwidth,
        lineHeight, "");
  myScanlineCount->setEditable(false, true);

  ypos += lineHeight + 5;
  new StaticTextWidget(boss, lfont, xpos, ypos, lwidth, lineHeight,
                       longstr ? "Scan Cycle " : "Scn Cycle", TextAlign::Left);

  myScanlineCycles = new EditTextWidget(boss, nfont, xpos+lwidth, ypos-1, fwidth,
                                        lineHeight, "");
  myScanlineCycles->setEditable(false, true);

  ypos += lineHeight + 5;
  new StaticTextWidget(boss, lfont, xpos, ypos, lwidth, lineHeight,
                       "Pixel Pos ", TextAlign::Left);

  myPixelPosition = new EditTextWidget(boss, nfont, xpos+lwidth, ypos-1, fwidth,
                                       lineHeight, "");
  myPixelPosition->setEditable(false, true);

  ypos += lineHeight + 5;
  new StaticTextWidget(boss, lfont, xpos, ypos, lwidth, lineHeight,
                       longstr ? "Color Clock " : "Color Clk ", TextAlign::Left);

  myColorClocks = new EditTextWidget(boss, nfont, xpos+lwidth, ypos-1, fwidth,
                                     lineHeight, "");
  myColorClocks->setEditable(false, true);

  // Calculate actual dimensions
  _w = myColorClocks->getAbsX() + myColorClocks->getWidth() - x;
  _h = ypos + lineHeight;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TiaInfoWidget::handleMouseDown(int x, int y, MouseButton b, int clickCount)
{
//cerr << "TiaInfoWidget button press: x = " << x << ", y = " << y << endl;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TiaInfoWidget::handleCommand(CommandSender* sender, int cmd, int data, int id)
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TiaInfoWidget::loadConfig()
{
  Debugger& dbg = instance().debugger();
  TIADebug& tia = dbg.tiaDebug();
  const TiaState& oldTia = static_cast<const TiaState&>(tia.getOldState());

  myFrameCount->setText("  " + Common::Base::toString(tia.frameCount(), Common::Base::F_10),
                        tia.frameCount() != oldTia.info[0]);
  myFrameCycles->setText("  " + Common::Base::toString(tia.frameCycles(), Common::Base::F_10),
                         tia.frameCycles() != oldTia.info[1]);

  myVSync->setState(tia.vsync(), tia.vsyncAsInt() != oldTia.info[2]);
  myVBlank->setState(tia.vblank(), tia.vblankAsInt() != oldTia.info[3]);

  int clk = tia.clocksThisLine();
  myScanlineCount->setText(Common::Base::toString(tia.scanlines(), Common::Base::F_10),
                           tia.scanlines() != oldTia.info[4]);
  myScanlineCountLast->setText(
    Common::Base::toString(tia.scanlinesLastFrame(), Common::Base::F_10),
    tia.scanlinesLastFrame() != oldTia.info[5]);
  myScanlineCycles->setText(Common::Base::toString(clk/3, Common::Base::F_10),
                            clk != oldTia.info[6]);
  myPixelPosition->setText(Common::Base::toString(clk-68, Common::Base::F_10),
                           clk != oldTia.info[6]);
  myColorClocks->setText(Common::Base::toString(clk, Common::Base::F_10),
                         clk != oldTia.info[6]);
}
