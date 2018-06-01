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

#include <cassert>

#include <ctime>
#ifdef HAVE_GETTIMEOFDAY
  #include <sys/time.h>
#endif

#include "bspf.hxx"

#include "MediaFactory.hxx"
#include "Sound.hxx"

#ifdef DEBUGGER_SUPPORT
  #include "Debugger.hxx"
#endif

#ifdef CHEATCODE_SUPPORT
  #include "CheatManager.hxx"
#endif

#include "FSNode.hxx"
#include "MD5.hxx"
#include "Cart.hxx"
#include "CartDetector.hxx"
#include "FrameBuffer.hxx"
#include "TIASurface.hxx"
#include "Settings.hxx"
#include "PropsSet.hxx"
#include "EventHandler.hxx"
#include "Menu.hxx"
#include "CommandMenu.hxx"
#include "Launcher.hxx"
#include "TimeMachine.hxx"
#include "PNGLibrary.hxx"
#include "Widget.hxx"
#include "Console.hxx"
#include "Random.hxx"
#include "SerialPort.hxx"
#include "StateManager.hxx"
#include "Version.hxx"

#include "OSystem.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
OSystem::OSystem()
  : myLauncherUsed(false),
    myQuitLoop(false)
{
  // Calculate startup time
  myMillisAtStart = uInt32(time(nullptr) * 1000);

  // Get built-in features
  #ifdef SOUND_SUPPORT
    myFeatures += "Sound ";
  #endif
  #ifdef JOYSTICK_SUPPORT
    myFeatures += "Joystick ";
  #endif
  #ifdef DEBUGGER_SUPPORT
    myFeatures += "Debugger ";
  #endif
  #ifdef CHEATCODE_SUPPORT
    myFeatures += "Cheats";
  #endif

  // Get build info
  ostringstream info;
  SDL_version ver;
  SDL_GetVersion(&ver);

  info << "Build " << STELLA_BUILD << ", using SDL " << int(ver.major)
       << "." << int(ver.minor) << "."<< int(ver.patch)
       << " [" << BSPF::ARCH << "]";
  myBuildInfo = info.str();

  mySettings = MediaFactory::createSettings(*this);
  myRandom = make_unique<Random>(*this);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
OSystem::~OSystem()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool OSystem::create()
{
  // Get updated paths for all configuration files
  setConfigPaths();
  ostringstream buf;
  buf << "Stella " << STELLA_VERSION << endl
      << "  Features: " << myFeatures << endl
      << "  " << myBuildInfo << endl << endl
      << "Base directory:       '"
      << FilesystemNode(myBaseDir).getShortPath() << "'" << endl
      << "Configuration file:   '"
      << FilesystemNode(myConfigFile).getShortPath() << "'" << endl
      << "User game properties: '"
      << FilesystemNode(myPropertiesFile).getShortPath() << "'" << endl;
  logMessage(buf.str(), 1);

  // NOTE: The framebuffer MUST be created before any other object!!!
  // Get relevant information about the video hardware
  // This must be done before any graphics context is created, since
  // it may be needed to initialize the size of graphical objects
  try        { myFrameBuffer = MediaFactory::createVideo(*this); }
  catch(...) { return false; }
  if(!myFrameBuffer->initialize())
    return false;

  // Create the event handler for the system
  myEventHandler = MediaFactory::createEventHandler(*this);
  myEventHandler->initialize();

  // Create a properties set for us to use and set it up
  myPropSet = make_unique<PropertiesSet>(propertiesFile());

#ifdef CHEATCODE_SUPPORT
  myCheatManager = make_unique<CheatManager>(*this);
  myCheatManager->loadCheatDatabase();
#endif

  // Create menu and launcher GUI objects
  myMenu = make_unique<Menu>(*this);
  myCommandMenu = make_unique<CommandMenu>(*this);
  myTimeMachine = make_unique<TimeMachine>(*this);
  myLauncher = make_unique<Launcher>(*this);
  myStateManager = make_unique<StateManager>(*this);

  // Create the sound object; the sound subsystem isn't actually
  // opened until needed, so this is non-blocking (on those systems
  // that only have a single sound device (no hardware mixing)
  createSound();

  // Create the serial port object
  // This is used by any controller that wants to directly access
  // a real serial port on the system
  mySerialPort = MediaFactory::createSerialPort();

  // Re-initialize random seed
  myRandom->initSeed();

  // Create PNG handler
  myPNGLib = make_unique<PNGLibrary>(*this);

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void OSystem::loadConfig()
{
  mySettings->loadConfig();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void OSystem::saveConfig()
{
  // Ask all subsystems to save their settings
  if(myFrameBuffer)
    myFrameBuffer->tiaSurface().ntsc().saveConfig(*mySettings);

  if(mySettings)
    mySettings->saveConfig();

  if(myPropSet)
    myPropSet->save(myPropertiesFile);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void OSystem::setConfigPaths()
{
  // Paths are saved with special characters preserved ('~' or '.')
  // We do some error checking here, so the rest of the codebase doesn't
  // have to worry about it
  FilesystemNode node;
  string s;

  validatePath(myStateDir, "statedir", myBaseDir + "state");
  validatePath(mySnapshotSaveDir, "snapsavedir", defaultSaveDir());
  validatePath(mySnapshotLoadDir, "snaploaddir", defaultLoadDir());
  validatePath(myNVRamDir, "nvramdir", myBaseDir + "nvram");
  validatePath(myCfgDir, "cfgdir", myBaseDir + "cfg");

  s = mySettings->getString("cheatfile");
  if(s == "") s = myBaseDir + "stella.cht";
  node = FilesystemNode(s);
  myCheatFile = node.getPath();
  mySettings->setValue("cheatfile", node.getShortPath());

  s = mySettings->getString("palettefile");
  if(s == "") s = myBaseDir + "stella.pal";
  node = FilesystemNode(s);
  myPaletteFile = node.getPath();
  mySettings->setValue("palettefile", node.getShortPath());

  s = mySettings->getString("propsfile");
  if(s == "") s = myBaseDir + "stella.pro";
  node = FilesystemNode(s);
  myPropertiesFile = node.getPath();
  mySettings->setValue("propsfile", node.getShortPath());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
PropertiesSet& OSystem::propSet(const string& md5)
{
  FilesystemNode node = FilesystemNode();

  return propSet(md5, node);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
PropertiesSet& OSystem::propSet(const string& md5, const FilesystemNode& node)
{
  if(md5 == EmptyString)
    return *myPropSet;
  else if(md5 == myGamePropSetMD5)
    return *myGamePropSet;
  else if (!node.exists())
    return *myPropSet;

  // Get a valid set of game specific properties
  Properties props;
  string path = myBaseDir + node.getNameWithExt(".pro");

  // Create a properties set based on ROM name
  FilesystemNode propNode = FilesystemNode(path);
  myGamePropertiesFile = propNode.getPath();

  myGamePropSet = make_unique<PropertiesSet>(myGamePropertiesFile);

  // Check if game specific property file exists and has matching md5
  if(myGamePropSet->size() && myGamePropSet->getMD5(md5, props))
  {
    myGamePropSetMD5 = md5;
    return *myGamePropSet;
  }
  else
  {
    myGamePropSetMD5 = "";
    return *myPropSet;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void OSystem::saveGamePropSet(const string& md5)
{
  if(myGamePropSet->size() && md5 == myGamePropSetMD5)
  {
    myGamePropSet->save(myGamePropertiesFile);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void OSystem::setBaseDir(const string& basedir)
{
  FilesystemNode node(basedir);
  if(!node.isDirectory())
    node.makeDir();

  myBaseDir = node.getPath();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void OSystem::setConfigFile(const string& file)
{
  myConfigFile = FilesystemNode(file).getPath();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void OSystem::setFramerate(float framerate)
{
  if(framerate > 0.0)
  {
    myDisplayFrameRate = framerate;
    myTimePerFrame = uInt32(1000000.0 / myDisplayFrameRate);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
FBInitStatus OSystem::createFrameBuffer()
{
  // Re-initialize the framebuffer to current settings
  FBInitStatus fbstatus = FBInitStatus::FailComplete;
  switch(myEventHandler->state())
  {
    case EventHandlerState::EMULATION:
    case EventHandlerState::PAUSE:
    case EventHandlerState::OPTIONSMENU:
    case EventHandlerState::CMDMENU:
    case EventHandlerState::TIMEMACHINE:
      if((fbstatus = myConsole->initializeVideo()) != FBInitStatus::Success)
        return fbstatus;
      break;

    case EventHandlerState::LAUNCHER:
      if((fbstatus = myLauncher->initializeVideo()) != FBInitStatus::Success)
        return fbstatus;
      break;

    case EventHandlerState::DEBUGGER:
  #ifdef DEBUGGER_SUPPORT
      if((fbstatus = myDebugger->initializeVideo()) != FBInitStatus::Success)
        return fbstatus;
  #endif
      break;

    case EventHandlerState::NONE:  // Should never happen
      logMessage("ERROR: Unknown emulation state in createFrameBuffer()", 0);
      break;
  }
  return fbstatus;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void OSystem::createSound()
{
  if(!mySound)
    mySound = MediaFactory::createAudio(*this);
#ifndef SOUND_SUPPORT
  mySettings->setValue("sound", false);
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string OSystem::createConsole(const FilesystemNode& rom, const string& md5sum,
                              bool newrom)
{
  bool showmessage = false;

  // If same ROM has been given, we reload the current one (assuming one exists)
  if(!newrom && rom == myRomFile)
  {
    showmessage = true;  // we show a message if a ROM is being reloaded
  }
  else
  {
    myRomFile = rom;
    myRomMD5  = md5sum;

    // Each time a new console is loaded, we simulate a cart removal
    // Some carts need knowledge of this, as they behave differently
    // based on how many power-cycles they've been through since plugged in
    mySettings->setValue("romloadcount", 0);
  }

  // Create an instance of the 2600 game console
  ostringstream buf;
  try
  {
    closeConsole();
    myConsole = openConsole(myRomFile, myRomMD5);
  }
  catch(const runtime_error& e)
  {
    buf << "ERROR: Couldn't create console (" << e.what() << ")";
    logMessage(buf.str(), 0);
    return buf.str();
  }

  if(myConsole)
  {
  #ifdef DEBUGGER_SUPPORT
    myDebugger = make_unique<Debugger>(*this, *myConsole);
    myDebugger->initialize();
    myConsole->attachDebugger(*myDebugger);
  #endif
  #ifdef CHEATCODE_SUPPORT
    myCheatManager->loadCheats(myRomMD5);
  #endif
    myEventHandler->reset(EventHandlerState::EMULATION);
    myEventHandler->setMouseControllerMode(mySettings->getString("usemouse"));
    if(createFrameBuffer() != FBInitStatus::Success)  // Takes care of initializeVideo()
    {
      logMessage("ERROR: Couldn't create framebuffer for console", 0);
      myEventHandler->reset(EventHandlerState::LAUNCHER);
      return "ERROR: Couldn't create framebuffer for console";
    }
    myConsole->initializeAudio();

    if(showmessage)
    {
      const string& id = myConsole->cartridge().multiCartID();
      if(id == "")
        myFrameBuffer->showMessage("New console created");
      else
        myFrameBuffer->showMessage("Multicart " +
          myConsole->cartridge().detectedType() + ", loading ROM" + id);
    }
    buf << "Game console created:" << endl
        << "  ROM file: " << myRomFile.getShortPath() << endl << endl
        << getROMInfo(*myConsole) << endl;
    logMessage(buf.str(), 1);

    // Update the timing info for a new console run
    resetLoopTiming();

    myFrameBuffer->setCursorState();

    // Also check if certain virtual buttons should be held down
    // These must be checked each time a new console is being created
    myEventHandler->handleConsoleStartupEvents();
  }
  return EmptyString;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool OSystem::reloadConsole()
{
  return createConsole(myRomFile, myRomMD5, false) == EmptyString;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool OSystem::hasConsole() const
{
  return myConsole != nullptr &&
         myEventHandler->state() != EventHandlerState::LAUNCHER;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool OSystem::createLauncher(const string& startdir)
{
  closeConsole();

  if(mySound)
    mySound->close();

  mySettings->setValue("tmpromdir", startdir);
  bool status = false;

  myEventHandler->reset(EventHandlerState::LAUNCHER);
  if(createFrameBuffer() == FBInitStatus::Success)
  {
    myLauncher->reStack();
    myFrameBuffer->setCursorState();

    setFramerate(30);
    resetLoopTiming();
    status = true;
  }
  else
    logMessage("ERROR: Couldn't create launcher", 0);

  myLauncherUsed = myLauncherUsed || status;
  return status;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string OSystem::getROMInfo(const FilesystemNode& romfile)
{
  unique_ptr<Console> console;
  try
  {
    string md5;
    console = openConsole(romfile, md5);
  }
  catch(const runtime_error& e)
  {
    ostringstream buf;
    buf << "ERROR: Couldn't get ROM info (" << e.what() << ")";
    return buf.str();
  }

  return getROMInfo(*console);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void OSystem::logMessage(const string& message, uInt8 level)
{
  if(level == 0)
  {
    cout << message << endl << std::flush;
    myLogMessages += message + "\n";
  }
  else if(level <= uInt8(mySettings->getInt("loglevel")))
  {
    if(mySettings->getBool("logtoconsole"))
      cout << message << endl << std::flush;
    myLogMessages += message + "\n";
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
unique_ptr<Console> OSystem::openConsole(const FilesystemNode& romfile, string& md5)
{
  unique_ptr<Console> console;

  // Open the cartridge image and read it in
  BytePtr image;
  uInt32 size  = 0;
  if((image = openROM(romfile, md5, size)) != nullptr)
  {
    // Get a valid set of properties, including any entered on the commandline
    // For initial creation of the Cart, we're only concerned with the BS type
    Properties props;

    // Load and use game specific props if existing
    FilesystemNode node = FilesystemNode(romfile);

    string path = myBaseDir + node.getNameWithExt(".pro");
    PropertiesSet& propset = propSet(md5, romfile);
    propset.getMD5(md5, props);

    // Local helper method
    auto CMDLINE_PROPS_UPDATE = [&](const string& name, PropertyType prop)
    {
      const string& s = mySettings->getString(name);
      if(s != "") props.set(prop, s);
    };

    CMDLINE_PROPS_UPDATE("bs", Cartridge_Type);
    CMDLINE_PROPS_UPDATE("type", Cartridge_Type);

    // Now create the cartridge
    string cartmd5 = md5;
    const string& type = props.get(Cartridge_Type);
    unique_ptr<Cartridge> cart =
      CartDetector::create(image, size, cartmd5, type, *this);

    // It's possible that the cart created was from a piece of the image,
    // and that the md5 (and hence the cart) has changed
    if(props.get(Cartridge_MD5) != cartmd5)
    {
      if(!propset.getMD5(cartmd5, props))
      {
        // Cart md5 wasn't found, so we create a new props for it
        props.set(Cartridge_MD5, cartmd5);
        props.set(Cartridge_Name, props.get(Cartridge_Name)+cart->multiCartID());
        propset.insert(props, false);
      }
    }

    CMDLINE_PROPS_UPDATE("channels", Cartridge_Sound);
    CMDLINE_PROPS_UPDATE("ld", Console_LeftDifficulty);
    CMDLINE_PROPS_UPDATE("rd", Console_RightDifficulty);
    CMDLINE_PROPS_UPDATE("tv", Console_TelevisionType);
    CMDLINE_PROPS_UPDATE("sp", Console_SwapPorts);
    CMDLINE_PROPS_UPDATE("lc", Controller_Left);
    CMDLINE_PROPS_UPDATE("rc", Controller_Right);
    const string& s = mySettings->getString("bc");
    if(s != "") { props.set(Controller_Left, s); props.set(Controller_Right, s); }
    CMDLINE_PROPS_UPDATE("cp", Controller_SwapPaddles);
    CMDLINE_PROPS_UPDATE("ma", Controller_MouseAxis);
    CMDLINE_PROPS_UPDATE("format", Display_Format);
    CMDLINE_PROPS_UPDATE("ystart", Display_YStart);
    CMDLINE_PROPS_UPDATE("height", Display_Height);
    CMDLINE_PROPS_UPDATE("pp", Display_Phosphor);
    CMDLINE_PROPS_UPDATE("ppblend", Display_PPBlend);

    // Finally, create the cart with the correct properties
    if(cart)
      console = make_unique<Console>(*this, cart, props);
  }

  return console;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void OSystem::closeConsole()
{
  if(myConsole)
  {
  #ifdef CHEATCODE_SUPPORT
    // If a previous console existed, save cheats before creating a new one
    myCheatManager->saveCheats(myConsole->properties().get(Cartridge_MD5));
  #endif
    myConsole.reset();
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BytePtr OSystem::openROM(const FilesystemNode& rom, string& md5, uInt32& size)
{
  // This method has a documented side-effect:
  // It not only loads a ROM and creates an array with its contents,
  // but also adds a properties entry if the one for the ROM doesn't
  // contain a valid name

  BytePtr image;
  if((size = rom.read(image)) == 0)
    return nullptr;

  // If we get to this point, we know we have a valid file to open
  // Now we make sure that the file has a valid properties entry
  // To save time, only generate an MD5 if we really need one
  if(md5 == "")
    md5 = MD5::hash(image, size);

  // Some games may not have a name, since there may not
  // be an entry in stella.pro.  In that case, we use the rom name
  // and reinsert the properties object
  Properties props;
  myPropSet->getMD5WithInsert(rom, md5, props);

  return image;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string OSystem::getROMInfo(const Console& console)
{
  const ConsoleInfo& info = console.about();
  ostringstream buf;

  buf << "  Cart Name:       " << info.CartName << endl
      << "  Cart MD5:        " << info.CartMD5 << endl
      << "  Controller 0:    " << info.Control0 << endl
      << "  Controller 1:    " << info.Control1 << endl
      << "  Display Format:  " << info.DisplayFormat << endl
      << "  Bankswitch Type: " << info.BankSwitch << endl;

  return buf.str();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void OSystem::resetLoopTiming()
{
  myTimingInfo.start = myTimingInfo.virt = getTicks();
  myTimingInfo.current = 0;
  myTimingInfo.totalTime = 0;
  myTimingInfo.totalFrames = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void OSystem::validatePath(string& path, const string& setting,
                           const string& defaultpath)
{
  const string& s = mySettings->getString(setting) == "" ? defaultpath :
                    mySettings->getString(setting);
  FilesystemNode node(s);
  if(!node.isDirectory())
    node.makeDir();

  path = node.getPath();
  mySettings->setValue(setting, node.getShortPath());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt64 OSystem::getTicks() const
{
#ifdef HAVE_GETTIMEOFDAY
  // Gettimeofday natively refers to the UNIX epoch (a set time in the past)
  timeval now;
  gettimeofday(&now, nullptr);

  return uInt64(now.tv_sec) * 1000000 + now.tv_usec;
#else
  // We use SDL_GetTicks, but add in the time when the application was
  // initialized.  This is necessary, since SDL_GetTicks only measures how
  // long SDL has been running, which can be the same between multiple runs
  // of the application.
  return uInt64(SDL_GetTicks() + myMillisAtStart) * 1000;
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void OSystem::mainLoop()
{
  if(mySettings->getString("timing") == "sleep")
  {
    // Sleep-based wait: good for CPU, bad for graphical sync
    for(;;)
    {
      myTimingInfo.start = getTicks();
      myEventHandler->poll(myTimingInfo.start);
      if(myQuitLoop) break;  // Exit if the user wants to quit
      myFrameBuffer->update();
      myTimingInfo.current = getTicks();
      myTimingInfo.virt += myTimePerFrame;

      // Timestamps may periodically go out of sync, particularly on systems
      // that can have 'negative time' (ie, when the time seems to go backwards)
      // This normally results in having a very large delay time, so we check
      // for that and reset the timers when appropriate
      if((myTimingInfo.virt - myTimingInfo.current) > (myTimePerFrame << 1))
      {
        myTimingInfo.current = myTimingInfo.virt = getTicks();
      }

      if(myTimingInfo.current < myTimingInfo.virt)
        SDL_Delay(uInt32(myTimingInfo.virt - myTimingInfo.current) / 1000);

      myTimingInfo.totalTime += (getTicks() - myTimingInfo.start);
      myTimingInfo.totalFrames++;
    }
  }
  else
  {
    // Busy-wait: bad for CPU, good for graphical sync
    for(;;)
    {
      myTimingInfo.start = getTicks();
      myEventHandler->poll(myTimingInfo.start);
      if(myQuitLoop) break;  // Exit if the user wants to quit
      myFrameBuffer->update();
      myTimingInfo.virt += myTimePerFrame;

      while(getTicks() < myTimingInfo.virt)
        ;  // busy-wait

      myTimingInfo.totalTime += (getTicks() - myTimingInfo.start);
      myTimingInfo.totalFrames++;
    }
  }

  // Cleanup time
#ifdef CHEATCODE_SUPPORT
  if(myConsole)
    myCheatManager->saveCheats(myConsole->properties().get(Cartridge_MD5));

  myCheatManager->saveCheatDatabase();
#endif
}
