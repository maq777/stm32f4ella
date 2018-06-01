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
//
//   Based on code from ScummVM - Scumm Interpreter
//   Copyright (C) 2002-2004 The ScummVM project
//============================================================================

#ifndef DIALOG_HXX
#define DIALOG_HXX

class FBSurface;
class OSystem;
class DialogContainer;
class TabWidget;
class CommandSender;

#include "Stack.hxx"
#include "Widget.hxx"
#include "GuiObject.hxx"
#include "StellaKeys.hxx"
#include "EventHandlerConstants.hxx"
#include "bspf.hxx"

/**
  This is the base class for all dialog boxes.

  @author  Stephen Anthony
*/
class Dialog : public GuiObject
{
  friend class DialogContainer;

  public:
    Dialog(OSystem& instance, DialogContainer& parent,
           int x = 0, int y = 0, int w = 0, int h = 0);
    Dialog(OSystem& instance, DialogContainer& parent, const GUI::Font& font,
           const string& title = "", int x = 0, int y = 0, int w = 0, int h = 0);

    virtual ~Dialog();

    void open(bool refresh = true);
    void close(bool refresh = true);

    bool isVisible() const override { return _visible; }

    virtual void center();
    virtual void drawDialog();
    virtual void loadConfig()  { }
    virtual void saveConfig()  { }
    virtual void setDefaults() { }

    void addFocusWidget(Widget* w) override;
    void addToFocusList(WidgetArray& list) override;
    void addToFocusList(WidgetArray& list, TabWidget* w, int tabId);
    void addBGroupToFocusList(WidgetArray& list) { _buttonGroup = list; }
    void addTabWidget(TabWidget* w);
    void addDefaultWidget(Widget* w) { _defaultWidget = w; }
    void addOKWidget(Widget* w)     { _okWidget = w;     }
    void addCancelWidget(Widget* w) { _cancelWidget = w; }
    void setFocus(Widget* w);

    /** Returns the base surface associated with this dialog. */
    FBSurface& surface() const { return *_surface; }

    /**
      Adds a surface to this dialog, which is rendered on top of the
      base surface whenever the base surface is re-rendered.  Since
      the surface render() call will always occur in such a case, the
      surface should call setVisible() to enable/disable its output.
    */
    void addSurface(shared_ptr<FBSurface> surface);

    void setFlags(int flags) { _flags |= flags;  setDirty(); }
    void clearFlags(int flags) { _flags &= ~flags; setDirty(); }
    int  getFlags() const { return _flags; }

    void setTitle(const string& title);
    bool hasTitle() { return !_title.empty(); }

    /** Determine the maximum bounds based on the given width and height
        Returns whether or not a large font can be used within these bounds.
    */
    bool getResizableBounds(uInt32& w, uInt32& h) const;

  protected:
    virtual void draw() override { }
    void releaseFocus() override;

    virtual void handleText(char text);
    virtual void handleKeyDown(StellaKey key, StellaMod modifiers);
    virtual void handleKeyUp(StellaKey key, StellaMod modifiers);
    virtual void handleMouseDown(int x, int y, MouseButton b, int clickCount);
    virtual void handleMouseUp(int x, int y, MouseButton b, int clickCount);
    virtual void handleMouseWheel(int x, int y, int direction);
    virtual void handleMouseMoved(int x, int y);
    virtual bool handleMouseClicks(int x, int y, MouseButton b);
    virtual void handleJoyDown(int stick, int button);
    virtual void handleJoyUp(int stick, int button);
    virtual void handleJoyAxis(int stick, int axis, int value);
    virtual bool handleJoyHat(int stick, int hat, JoyHat value);
    virtual void handleCommand(CommandSender* sender, int cmd, int data, int id) override;

    Widget* findWidget(int x, int y) const; // Find the widget at pos x,y if any

    void addOKCancelBGroup(WidgetArray& wid, const GUI::Font& font,
                           const string& okText = "OK",
                           const string& cancelText = "Cancel",
                           bool focusOKButton = true,
                           int buttonWidth = 0);

    void addDefaultsOKCancelBGroup(WidgetArray& wid, const GUI::Font& font,
                                   const string& okText = "OK",
                                   const string& cancelText = "Cancel",
                                   const string& defaultsText = "Defaults",
                                   bool focusOKButton = true);

    void processCancelWithoutWidget(bool state) { _processCancel = state; }

  private:
    void buildCurrentFocusList(int tabID = -1);
    bool handleNavEvent(Event::Type e);
    void getTabIdForWidget(Widget* w);
    bool cycleTab(int direction);

  protected:
    const GUI::Font& _font;

    Widget* _mouseWidget;
    Widget* _focusedWidget;
    Widget* _dragWidget;
    Widget* _defaultWidget;
    Widget* _okWidget;
    Widget* _cancelWidget;

    bool    _visible;
    bool    _processCancel;
    string  _title;
    int     _th;

    Common::FixedStack<shared_ptr<FBSurface>> mySurfaceStack;

  private:
    struct Focus {
      Widget* widget;
      WidgetArray list;

      Focus(Widget* w = nullptr) : widget(w) { }
      virtual ~Focus() = default;

      Focus(const Focus&) = default;
      Focus& operator=(const Focus&) = default;
    };
    using FocusList = vector<Focus>;

    struct TabFocus {
      TabWidget* widget;
      FocusList focus;
      uInt32 currentTab;

      TabFocus(TabWidget* w = nullptr) : widget(w), currentTab(0) { }
      virtual ~TabFocus() = default;

      TabFocus(const TabFocus&) = default;
      TabFocus& operator=(const TabFocus&) = default;

      void appendFocusList(WidgetArray& list);
      void saveCurrentFocus(Widget* w);
      Widget* getNewFocus();
    };
    using TabFocusList = vector<TabFocus>;

    Focus        _myFocus;    // focus for base dialog
    TabFocusList _myTabList;  // focus for each tab (if any)

    WidgetArray _buttonGroup;
    shared_ptr<FBSurface> _surface;

    int _tabID;
    int _flags;

  private:
    // Following constructors and assignment operators not supported
    Dialog() = delete;
    Dialog(const Dialog&) = delete;
    Dialog(Dialog&&) = delete;
    Dialog& operator=(const Dialog&) = delete;
    Dialog& operator=(Dialog&&) = delete;
};

#endif
