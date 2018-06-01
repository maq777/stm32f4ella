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

#ifndef FRAMEBUFFER_SDL2_HXX
#define FRAMEBUFFER_SDL2_HXX

#include "SDL_lib.hxx"

class OSystem;
class FBSurfaceSDL2;

#include "bspf.hxx"
#include "FrameBuffer.hxx"

/**
  This class implements a standard SDL2 2D, hardware accelerated framebuffer.
  Behind the scenes, it may be using Direct3D, OpenGL(ES), etc.

  @author  Stephen Anthony
*/
class FrameBufferSDL2 : public FrameBuffer
{
  friend class FBSurfaceSDL2;

  public:
    /**
      Creates a new SDL2 framebuffer
    */
    FrameBufferSDL2(OSystem& osystem);
    virtual ~FrameBufferSDL2();

    //////////////////////////////////////////////////////////////////////
    // The following are derived from public methods in FrameBuffer.hxx
    //////////////////////////////////////////////////////////////////////

    /**
      Updates window title.

      @param title  The title of the application / window
    */
    void setTitle(const string& title) override;

    /**
      Shows or hides the cursor based on the given boolean value.
    */
    void showCursor(bool show) override;

    /**
      Answers if the display is currently in fullscreen mode.
    */
    bool fullScreen() const override;

    /**
      This method is called to retrieve the R/G/B data from the given pixel.

      @param pixel  The pixel containing R/G/B data
      @param r      The red component of the color
      @param g      The green component of the color
      @param b      The blue component of the color
    */
    inline void getRGB(uInt32 pixel, uInt8* r, uInt8* g, uInt8* b) const override
      { SDL_GetRGB(pixel, myPixelFormat, r, g, b); }

    /**
      This method is called to map a given R/G/B triple to the screen palette.

      @param r  The red component of the color.
      @param g  The green component of the color.
      @param b  The blue component of the color.
    */
    inline uInt32 mapRGB(uInt8 r, uInt8 g, uInt8 b) const override
      { return SDL_MapRGB(myPixelFormat, r, g, b); }

    /**
      This method is called to get a copy of the specified ARGB data from the
      viewable FrameBuffer area.  Note that this isn't the same as any
      internal surfaces that may be in use; it should return the actual data
      as it is currently seen onscreen.

      @param buffer  A copy of the pixel data in ARGB8888 format
      @param pitch   The pitch (in bytes) for the pixel data
      @param rect    The bounding rectangle for the buffer
    */
    void readPixels(uInt8* buffer, uInt32 pitch, const GUI::Rect& rect) const override;

    /**
      Clear the frame buffer
    */
    void clear() override;

  protected:
    //////////////////////////////////////////////////////////////////////
    // The following are derived from protected methods in FrameBuffer.hxx
    //////////////////////////////////////////////////////////////////////
    /**
      This method is called to query and initialize the video hardware
      for desktop and fullscreen resolution information.
    */
    void queryHardware(vector<GUI::Size>& displays, VariantList& renderers) override;

    /**
      This method is called to query the video hardware for the index
      of the display the current window is displayed on

      @return  the current display index or a negative value if no
               window is displayed
    */
    Int32 getCurrentDisplayIndex() override;

    /**
      This method is called to change to the given video mode.

      @param title The title for the created window
      @param mode  The video mode to use

      @return  False on any errors, else true
    */
    bool setVideoMode(const string& title, const VideoMode& mode) override;

    /**
      This method is called to invalidate the contents of the entire
      framebuffer (ie, mark the current content as invalid, and erase it on
      the next drawing pass).
    */
    void invalidate() override;

    /**
      This method is called to create a surface with the given attributes.

      @param w     The requested width of the new surface.
      @param h     The requested height of the new surface.
      @param data  If non-null, use the given data values as a static surface
    */
    unique_ptr<FBSurface> createSurface(uInt32 w, uInt32 h, const uInt32* data)
        const override;

    /**
      Grabs or ungrabs the mouse based on the given boolean value.
    */
    void grabMouse(bool grab) override;

    /**
      Set the icon for the main SDL window.
    */
    void setWindowIcon() override;

    /**
      This method is called to provide information about the FrameBuffer.
    */
    string about() const override;

    /**
      This method is called after any drawing is done (per-frame).
    */
    void postFrameUpdate() override;

  private:
    // The SDL video buffer
    SDL_Window* myWindow;
    SDL_Renderer* myRenderer;

    // Used by mapRGB (when palettes are created)
    SDL_PixelFormat* myPixelFormat;

    // Indicates that the renderer has been modified, and should be redrawn
    bool myDirtyFlag;

  private:
    // Following constructors and assignment operators not supported
    FrameBufferSDL2() = delete;
    FrameBufferSDL2(const FrameBufferSDL2&) = delete;
    FrameBufferSDL2(FrameBufferSDL2&&) = delete;
    FrameBufferSDL2& operator=(const FrameBufferSDL2&) = delete;
    FrameBufferSDL2& operator=(FrameBufferSDL2&&) = delete;
};

#endif
