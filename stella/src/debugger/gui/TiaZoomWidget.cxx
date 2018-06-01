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

#include "OSystem.hxx"
#include "Console.hxx"
#include "TIA.hxx"
#include "FrameBuffer.hxx"
#include "FBSurface.hxx"
#include "Widget.hxx"
#include "GuiObject.hxx"
#include "ContextMenu.hxx"

#include "TiaZoomWidget.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TiaZoomWidget::TiaZoomWidget(GuiObject* boss, const GUI::Font& font,
                             int x, int y, int w, int h)
  : Widget(boss, font, x, y, 16, 16),
    CommandSender(boss)
{
  _flags = WIDGET_ENABLED | WIDGET_CLEARBG |
           WIDGET_RETAIN_FOCUS | WIDGET_TRACK_MOUSE;
  _bgcolor = _bgcolorhi = kDlgColor;

  // Use all available space, up to the maximum bounds of the TIA image
  _w = std::min(w, 320);
  _h = std::min(h, 260);

  addFocusWidget(this);

  // Initialize positions
  myZoomLevel = 2;
  myNumCols = ((_w - 4) >> 1) / myZoomLevel;
  myNumRows = (_h - 4) / myZoomLevel;
  myXOff = myYOff = 0;

  myMouseMoving = false;
  myXClick = myYClick = 0;

  // Create context menu for zoom levels
  VariantList l;
  VarList::push_back(l, "2x zoom", "2");
  VarList::push_back(l, "4x zoom", "4");
  VarList::push_back(l, "8x zoom", "8");
  myMenu = make_unique<ContextMenu>(this, font, l);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TiaZoomWidget::loadConfig()
{
  setDirty();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TiaZoomWidget::setPos(int x, int y)
{
  // Center on given x,y point
  myXOff = (x >> 1) - (myNumCols >> 1);
  myYOff = y - (myNumRows >> 1);

  recalc();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TiaZoomWidget::zoom(int level)
{
  if(myZoomLevel == level)
    return;

  myZoomLevel = level;
  myNumCols = ((_w - 4) >> 1) / myZoomLevel;
  myNumRows = (_h - 4) / myZoomLevel;

  recalc();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TiaZoomWidget::recalc()
{
  const int tw = instance().console().tia().width(),
            th = instance().console().tia().height();

  // Don't go past end of framebuffer
  myXOff = BSPF::clamp(myXOff, 0, tw - myNumCols);
  myYOff = BSPF::clamp(myYOff, 0, th - myNumRows);

  setDirty();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TiaZoomWidget::handleMouseDown(int x, int y, MouseButton b, int clickCount)
{
  // Button 1 is for 'drag'/movement of the image
  // Button 2 is for context menu
  if(b == MouseButton::LEFT)
  {
    // Indicate mouse drag started/in progress
    myMouseMoving = true;
    myXClick = x;
    myYClick = y;
  }
  else if(b == MouseButton::RIGHT)
  {
    // Add menu at current x,y mouse location
    myMenu->show(x + getAbsX(), y + getAbsY());
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TiaZoomWidget::handleMouseUp(int x, int y, MouseButton b, int clickCount)
{
  myMouseMoving = false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TiaZoomWidget::handleMouseWheel(int x, int y, int direction)
{
  if(direction > 0)
    handleEvent(Event::UIDown);
  else
    handleEvent(Event::UIUp);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TiaZoomWidget::handleMouseMoved(int x, int y)
{
  // TODO: Not yet working - finish for next release
#if 0
  if(myMouseMoving)
  {
    if(x < 0 || y < 0 || x > _w || y > _h)
      return;

    int diffx = ((x - myXClick) >> 1);// / myZoomLevel;
    int diffy = (y - myYClick);// / myZoomLevel;
//    myXClick = x;
//    myYClick = y;



//cerr << diffx << " " << diffy << endl;


    myXOff -= diffx;
    myYOff -= diffy;

    recalc();
//    cerr << x << ", " << y << endl;
  }
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TiaZoomWidget::handleMouseEntered()
{
  setFlags(WIDGET_HILITED);
  setDirty();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TiaZoomWidget::handleMouseLeft()
{
  clearFlags(WIDGET_HILITED);
  setDirty();
  myMouseMoving = false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool TiaZoomWidget::handleEvent(Event::Type event)
{
  bool handled = true;

  switch(event)
  {
    case Event::UIUp:
      myYOff -= 4;
      break;

    case Event::UIDown:
      myYOff += 4;
      break;

    case Event::UILeft:
      myXOff -= 2;
      break;

    case Event::UIRight:
      myXOff += 2;
      break;

    case Event::UIPgUp:
      myYOff = 0;
      break;

    case Event::UIPgDown:
      myYOff = _h;
      break;

    case Event::UIHome:
      myXOff = 0;
      break;

    case Event::UIEnd:
      myXOff = _w;
      break;

    default:
      handled = false;
      break;
  }

  if(handled)
    recalc();

  return handled;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TiaZoomWidget::handleCommand(CommandSender* sender, int cmd, int data, int id)
{
  switch(cmd)
  {
    case ContextMenu::kItemSelectedCmd:
    {
      int level = myMenu->getSelectedTag().toInt();
      if(level > 0)
        zoom(level);
      break;
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TiaZoomWidget::drawWidget(bool hilite)
{
//cerr << "TiaZoomWidget::drawWidget\n";
  FBSurface& s = dialog().surface();

  s.fillRect(_x+1, _y+1, _w-2, _h-2, kBGColor);
  s.frameRect(_x, _y, _w, _h, hilite ? kWidColorHi : kColor);

  // Draw the zoomed image
  // This probably isn't as efficient as it can be, but it's a small area
  // and I don't have time to make it faster :)
  const uInt8* currentFrame  = instance().console().tia().frameBuffer();
  const int width = instance().console().tia().width(),
            wzoom = myZoomLevel << 1,
            hzoom = myZoomLevel;

  // Get current scanline position
  // This determines where the frame greying should start
  uInt32 scanx, scany, scanoffset;
  instance().console().tia().electronBeamPos(scanx, scany);
  scanoffset = width * scany + scanx;

  int x, y, col, row;
  for(y = myYOff, row = 0; y < myNumRows+myYOff; ++y, row += hzoom)
  {
    for(x = myXOff, col = 0; x < myNumCols+myXOff; ++x, col += wzoom)
    {
      uInt32 idx = y*width + x;
      uInt32 color = currentFrame[idx] | (idx > scanoffset ? 1 : 0);
      s.fillRect(_x + col + 1, _y + row + 1, wzoom, hzoom, color);
    }
  }
}
