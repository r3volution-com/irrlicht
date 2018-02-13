// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_GLFW3_DEVICE_

#include "CIrrDeviceGLFW3.h"
#include "IEventReceiver.h"
#include "irrList.h"
#include "os.h"
#include "CTimer.h"
#include "irrString.h"
#include "Keycodes.h"
#include "COSOperator.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "SIrrCreationParameters.h"

static int GLFW3DeviceInstances = 0;

namespace irr
{
	namespace video
	{
		#ifdef _IRR_COMPILE_WITH_DIRECT3D_9_
		IVideoDriver* createDirectX9Driver(const irr::SIrrlichtCreationParameters& params,
			io::IFileSystem* io, HWND window);
		#endif

		#ifdef _IRR_COMPILE_WITH_OPENGL_
		IVideoDriver* createOpenGLDriver(const SIrrlichtCreationParameters& params,
				io::IFileSystem* io, CIrrDeviceGLFW3* device);
		#endif
	} // end namespace video

} // end namespace irr


namespace irr
{

//! constructor
CIrrDeviceGLFW3::CIrrDeviceGLFW3(const SIrrlichtCreationParameters& param)
	: CIrrDeviceStub(param),
	window((GLFWwindow*)param.WindowId),
	MouseX(0), MouseY(0), MouseButtonStates(0),
	Width(param.WindowSize.Width), Height(param.WindowSize.Height),
	Resizable(false), WindowHasFocus(false), WindowMinimized(false)
{
	#ifdef _DEBUG
	setDebugName("CIrrDeviceGLFW3");
	#endif

	if ( window == nullptr && ++GLFW3DeviceInstances == 1 )
	{
    	if (glfwInit() == GLFW_FALSE){
			os::Printer::log("Unable to initialize GLFW3!");
			Close = true;
		}

    	glfwSetTime(0);

		//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}

	int major, minor, rev;
	glfwGetVersion(&major,&minor,&rev);

	core::stringc GLFW3version = "GLFW3 Version ";
	GLFW3version += major;
	GLFW3version += ".";
	GLFW3version += minor;
	GLFW3version += ".";
	GLFW3version += rev;

	Operator = new COSOperator(GLFW3version);
	if ( GLFW3DeviceInstances == 1 )
	{
		os::Printer::log(GLFW3version.c_str(), ELL_INFORMATION);
	}

	// create keymap
	createKeyMap();

	// enable key to character translation (UNICODE)
	// typedef void(* 	GLFWcharfun) (GLFWwindow *, unsigned int)

	if ( CreationParams.Fullscreen ) {
		// Fullscreen ( http://www.glfw.org/docs/latest/window_guide.html#window_full_screen )
	}

	// create the window, only if we do not use the null device
	createWindow();

	// create cursor control
	CursorControl = new CCursorControl(this);

	// create driver
	createDriver();

	if (VideoDriver)
		createGUIAndScene();
}


//! destructor
CIrrDeviceGLFW3::~CIrrDeviceGLFW3()
{
	if ( --GLFW3DeviceInstances == 0 )
	{
#if defined(_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
	//ToDo
#endif
		glfwTerminate();

		os::Printer::log("Quit GLFW3", ELL_INFORMATION);
	}
}


bool CIrrDeviceGLFW3::createWindow()
{
	if ( Close )
		return false;

	if (CreationParams.DriverType == video::EDT_OPENGL)
	{
		if (window == nullptr){
			if (CreationParams.Bits==16)
			{
				glfwWindowHint(GLFW_RED_BITS, 4);
				glfwWindowHint(GLFW_GREEN_BITS, 4);
				glfwWindowHint(GLFW_BLUE_BITS, 4);
				glfwWindowHint(GLFW_ALPHA_BITS, CreationParams.WithAlphaChannel?1:0);
			}
			else
			{
				glfwWindowHint(GLFW_RED_BITS, 8);
				glfwWindowHint(GLFW_GREEN_BITS, 8);
				glfwWindowHint(GLFW_BLUE_BITS, 8);
				glfwWindowHint(GLFW_ALPHA_BITS, CreationParams.WithAlphaChannel?8:0);
			}

			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
			glfwWindowHint(GLFW_STENCIL_BITS, CreationParams.Stencilbuffer?8:0);
			glfwWindowHint(GLFW_DEPTH_BITS, CreationParams.ZBufferBits?CreationParams.ZBufferBits:24);

			if (CreationParams.Doublebuffer)
				glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

			if (CreationParams.Stereobuffer)
				glfwWindowHint(GLFW_STEREO, GLFW_TRUE);

			if (CreationParams.AntiAlias>1) 
				glfwWindowHint(GLFW_SAMPLES, CreationParams.AntiAlias);
			else
				glfwWindowHint(GLFW_SAMPLES, GLFW_DONT_CARE);

			if (CreationParams.AntiAlias>1)
			{
				while (--CreationParams.AntiAlias>1)
				{
					glfwWindowHint(GLFW_SAMPLES, CreationParams.AntiAlias);
					window = glfwCreateWindow(Width, Height, "Irrlicht GLFW3 window", nullptr, nullptr);
					glfwMakeContextCurrent(window);
					if (window)
						break;
				}
				if (window == nullptr)
				{
					glfwWindowHint(GLFW_SAMPLES, GLFW_DONT_CARE);
					window = glfwCreateWindow(Width, Height, "Irrlicht GLFW3 window", nullptr, nullptr);
					glfwMakeContextCurrent(window);
					if (window)
						os::Printer::log("AntiAliasing disabled due to lack of support!" );
				}
			} 
			else
			{
				window = glfwCreateWindow(Width, Height, "Irrlicht GLFW3 window", nullptr, nullptr);
				glfwMakeContextCurrent(window);
			}
		}

		if (window != nullptr) {
			/*glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			glViewport(0, 0, width, height);
			glfwSwapInterval(0);
			glfwSwapBuffers(window);*/

			if (!CreationParams.IgnoreInput){
				glfwSetWindowUserPointer(window, this);

				glfwSetCursorPosCallback(window,
						[](GLFWwindow *w, double x, double y) {

						CIrrDeviceGLFW3 *obj = (CIrrDeviceGLFW3*)glfwGetWindowUserPointer(w);

						obj->irrevent.EventType = irr::EET_MOUSE_INPUT_EVENT;
						obj->irrevent.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
						obj->MouseX = obj->irrevent.MouseInput.X = x;
						obj->MouseY = obj->irrevent.MouseInput.Y = y;
						obj->irrevent.MouseInput.ButtonStates = obj->MouseButtonStates;

						obj->postEventFromUser(obj->irrevent);
					}
				);

				glfwSetMouseButtonCallback(window,
					[](GLFWwindow *w, int button, int action, int modifiers) {

						CIrrDeviceGLFW3 *obj = (CIrrDeviceGLFW3*)glfwGetWindowUserPointer(w);

						if (action == GLFW_PRESS){
							switch(button){
								case GLFW_MOUSE_BUTTON_LEFT:
									obj->irrevent.MouseInput.Event = irr::EMIE_LMOUSE_PRESSED_DOWN;
									obj->MouseButtonStates |= irr::EMBSM_LEFT;
								break;
								case GLFW_MOUSE_BUTTON_RIGHT:
									obj->irrevent.MouseInput.Event = irr::EMIE_RMOUSE_PRESSED_DOWN;
									obj->MouseButtonStates |= irr::EMBSM_RIGHT;
								break;
								case GLFW_MOUSE_BUTTON_MIDDLE:
									obj->irrevent.MouseInput.Event = irr::EMIE_MMOUSE_PRESSED_DOWN;
									obj->MouseButtonStates |= irr::EMBSM_MIDDLE;
								break;
							}
						} else if (action == GLFW_RELEASE){
							switch(button){
								case GLFW_MOUSE_BUTTON_LEFT:
									obj->irrevent.MouseInput.Event = irr::EMIE_LMOUSE_LEFT_UP;
									obj->MouseButtonStates &= !irr::EMBSM_LEFT;
								break;
								case GLFW_MOUSE_BUTTON_RIGHT:
									obj->irrevent.MouseInput.Event = irr::EMIE_RMOUSE_LEFT_UP;
									obj->MouseButtonStates &= !irr::EMBSM_RIGHT;
								break;
								case GLFW_MOUSE_BUTTON_MIDDLE:
									obj->irrevent.MouseInput.Event = irr::EMIE_MMOUSE_LEFT_UP;
									obj->MouseButtonStates &= !irr::EMBSM_MIDDLE;
								break;
							}
						}

						obj->irrevent.MouseInput.ButtonStates = obj->MouseButtonStates;

						if (obj->irrevent.MouseInput.Event != irr::EMIE_MOUSE_MOVED)
						{
							obj->postEventFromUser(obj->irrevent);

							if ( obj->irrevent.MouseInput.Event >= EMIE_LMOUSE_PRESSED_DOWN && obj->irrevent.MouseInput.Event <= EMIE_MMOUSE_PRESSED_DOWN )
							{
								u32 clicks = obj->checkSuccessiveClicks(obj->irrevent.MouseInput.X, obj->irrevent.MouseInput.Y, obj->irrevent.MouseInput.Event);
								if ( clicks == 2 )
								{
									obj->irrevent.MouseInput.Event = (EMOUSE_INPUT_EVENT)(EMIE_LMOUSE_DOUBLE_CLICK + obj->irrevent.MouseInput.Event-EMIE_LMOUSE_PRESSED_DOWN);
									obj->postEventFromUser(obj->irrevent);
								}
								else if ( clicks == 3 )
								{
									obj->irrevent.MouseInput.Event = (EMOUSE_INPUT_EVENT)(EMIE_LMOUSE_TRIPLE_CLICK + obj->irrevent.MouseInput.Event-EMIE_LMOUSE_PRESSED_DOWN);
									obj->postEventFromUser(obj->irrevent);
								}
							}
						}
					}
				);

				glfwSetScrollCallback(window,
					[](GLFWwindow *w, double x, double y) {
						CIrrDeviceGLFW3 *obj = (CIrrDeviceGLFW3*)glfwGetWindowUserPointer(w);
						if (y < 0){
							obj->irrevent.MouseInput.Event = irr::EMIE_MOUSE_WHEEL;
							obj->irrevent.MouseInput.Wheel = 1.0f;
						}
						else
						{
							obj->irrevent.MouseInput.Event = irr::EMIE_MOUSE_WHEEL;
							obj->irrevent.MouseInput.Wheel = -1.0f;
						}
				}
				);

				glfwSetCharModsCallback(window,
					[](GLFWwindow *w, unsigned int codepoint, int mods) {
						CIrrDeviceGLFW3 *obj = (CIrrDeviceGLFW3*)glfwGetWindowUserPointer(w);
						SKeyMap mp;
						mp.GLFW3Key = codepoint;
						s32 idx = obj->KeyMap.binary_search(mp);

						EKEY_CODE key;
						if (idx == -1)
							key = (EKEY_CODE)0;
						else
							key = (EKEY_CODE)obj->KeyMap[idx].Win32Key;

						obj->irrevent.EventType = irr::EET_KEY_INPUT_EVENT;
						obj->irrevent.KeyInput.Char = codepoint;
						obj->irrevent.KeyInput.Key = key;
						obj->irrevent.KeyInput.PressedDown = true;
						obj->irrevent.KeyInput.Shift = (mods == GLFW_MOD_SHIFT);
						obj->irrevent.KeyInput.Control = (mods == GLFW_MOD_CONTROL);
						obj->postEventFromUser(obj->irrevent);
					}
				);

				glfwSetWindowFocusCallback(window,
					[](GLFWwindow *w, int focused) {
						CIrrDeviceGLFW3 *obj = (CIrrDeviceGLFW3*)glfwGetWindowUserPointer(w);
						obj->WindowHasFocus = (focused == GLFW_TRUE);
					}
				);

				glfwSetWindowIconifyCallback(window,
					[](GLFWwindow *w, int iconified) {
						CIrrDeviceGLFW3 *obj = (CIrrDeviceGLFW3*)glfwGetWindowUserPointer(w);
						obj->WindowMinimized = (iconified == GLFW_TRUE);
					}
				);

				glfwSetWindowSizeCallback(window,
					[](GLFWwindow *win, int w, int h) {
						CIrrDeviceGLFW3 *obj = (CIrrDeviceGLFW3*)glfwGetWindowUserPointer(win);
						if ((w != (int)obj->Width) || (h != (int)obj->Height))
						{
							obj->Width = w;
							obj->Height = h;
							if (obj->VideoDriver)
								obj->VideoDriver->OnResize(core::dimension2d<u32>(obj->Width, obj->Height));
						}
					}
				);

				glfwSetWindowCloseCallback(window,
					[](GLFWwindow *w) {
						CIrrDeviceGLFW3 *obj = (CIrrDeviceGLFW3*)glfwGetWindowUserPointer(w);
						obj->Close = true;
					}
				);

				#if defined(_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
					//ToDo: joystick
				#endif
			}
				
			return true;
		} else {
			os::Printer::log("Unable to initialize GLFW3 window!");
			glfwTerminate();
			return false;
		}
	} else {
		os::Printer::log("Only OpenGL is supported!");
		Close = true;
		return false;
	}
}


//! create the driver
void CIrrDeviceGLFW3::createDriver()
{
	switch(CreationParams.DriverType)
	{
	case video::DEPRECATED_EDT_DIRECT3D8_NO_LONGER_EXISTS:
		os::Printer::log("DIRECT3D8 Driver is no longer supported in Irrlicht. Try another one.", ELL_ERROR);
		break;

	case video::EDT_DIRECT3D9:
		#ifdef _IRR_COMPILE_WITH_DIRECT3D_9_
		os::Printer::log("Only OpenGL is supported!");
		#else
		os::Printer::log("DIRECT3D9 Driver was not compiled into this dll. Try another one.", ELL_ERROR);
		#endif // _IRR_COMPILE_WITH_DIRECT3D_9_

		break;

	case video::EDT_SOFTWARE:
		#ifdef _IRR_COMPILE_WITH_SOFTWARE_
		os::Printer::log("Only OpenGL is supported!");
		#else
		os::Printer::log("No Software driver support compiled in.", ELL_ERROR);
		#endif
		break;

	case video::EDT_BURNINGSVIDEO:
		#ifdef _IRR_COMPILE_WITH_BURNINGSVIDEO_
		os::Printer::log("Only OpenGL is supported!");
		#else
		os::Printer::log("Burning's video driver was not compiled in.", ELL_ERROR);
		#endif
		break;

	case video::EDT_OPENGL:
		#ifdef _IRR_COMPILE_WITH_OPENGL_
		VideoDriver = video::createOpenGLDriver(CreationParams, FileSystem, this);
		#else
		os::Printer::log("No OpenGL support compiled in.", ELL_ERROR);
		#endif
		break;

	case video::EDT_NULL:
		VideoDriver = video::createNullDriver(FileSystem, CreationParams.WindowSize);
		break;

	default:
		os::Printer::log("Unable to create video driver of unknown type.", ELL_ERROR);
		break;
	}
}


//! runs the device. Returns false if device wants to be deleted
bool CIrrDeviceGLFW3::run()
{
	os::Timer::tick();

	glfwPollEvents();

	/*
		irrevent.EventType = irr::EET_USER_EVENT;
		irrevent.UserEvent.UserData1 = reinterpret_cast<uintptr_t>(SDL_event.user.data1);
		irrevent.UserEvent.UserData2 = reinterpret_cast<uintptr_t>(SDL_event.user.data2);

		postEventFromUser(irrevent);
	*/

	return !Close;
}

//! Activate any joysticks, and generate events for them.
bool CIrrDeviceGLFW3::activateJoysticks(core::array<SJoystickInfo> & joystickInfo)
{
#if defined(_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
	return true;
#endif // _IRR_COMPILE_WITH_JOYSTICK_EVENTS_
	return false;
}



//! pause execution temporarily
void CIrrDeviceGLFW3::yield()
{
	glfwWaitEventsTimeout(0.1);
}


//! pause execution for a specified time
void CIrrDeviceGLFW3::sleep(u32 timeMs, bool pauseTimer)
{
	const bool wasStopped = Timer ? Timer->isStopped() : true;
	if (pauseTimer && !wasStopped)
		Timer->stop();

	glfwWaitEventsTimeout(timeMs/1000);

	if (pauseTimer && !wasStopped)
		Timer->start();
}


//! sets the caption of the window
void CIrrDeviceGLFW3::setWindowCaption(const wchar_t* text)
{
	core::stringc textc = text;
	glfwSetWindowTitle(window, textc.c_str( ));
}


//! presents a surface in the client area
bool CIrrDeviceGLFW3::present(video::IImage* surface, void* windowId, core::rect<s32>* srcClip)
{
	GLFWwindow *w = (GLFWwindow*)windowId;

	if (!w) w = window;

	glfwSwapBuffers(w);

	return true;
}


//! notifies the device that it should close itself
void CIrrDeviceGLFW3::closeDevice()
{
	Close = true;
}


//! \return Pointer to a list with all video modes supported
video::IVideoModeList* CIrrDeviceGLFW3::getVideoModeList()
{
	if (!VideoModeList->getVideoModeCount())
	{
		int count;
		const GLFWvidmode* modes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &count);

		for (int i=0; i<count; i++){
			VideoModeList->addMode(core::dimension2d<u32>(modes[i].width, modes[i].height), modes[i].redBits+modes[i].blueBits+modes[i].greenBits);

		}
	}

	return VideoModeList;
}


//! Sets if the window should be resizable in windowed mode.
void CIrrDeviceGLFW3::setResizable(bool resize)
{
	if (resize != Resizable)
	{
		if (resize)
			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		else
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		//Screen = SDL_SetVideoMode( 0, 0, 0, SDL_Flags );
		Resizable = resize;
	}
}


//! Minimizes window if possible
void CIrrDeviceGLFW3::minimizeWindow()
{
	glfwIconifyWindow(window);
}


//! Maximize window
void CIrrDeviceGLFW3::maximizeWindow()
{
	glfwMaximizeWindow(window);
}

//! Get the position of this window on screen
core::position2di CIrrDeviceGLFW3::getWindowPosition()
{
	int x, y;
	glfwGetWindowPos (window, &x, &y);
    return core::position2di(x, y);
}


//! Restore original window size
void CIrrDeviceGLFW3::restoreWindow()
{
	glfwRestoreWindow(window);
}


//! returns if window is active. if not, nothing need to be drawn
bool CIrrDeviceGLFW3::isWindowActive() const
{
	return (WindowHasFocus && !WindowMinimized);
}


//! returns if window has focus.
bool CIrrDeviceGLFW3::isWindowFocused() const
{
	return WindowHasFocus;
}


//! returns if window is minimized.
bool CIrrDeviceGLFW3::isWindowMinimized() const
{
	return WindowMinimized;
}


//! Set the current Gamma Value for the Display
bool CIrrDeviceGLFW3::setGammaRamp( f32 red, f32 green, f32 blue, f32 brightness, f32 contrast )
{
	//glfwSetGammaRamp (glfwGetPrimaryMonitor(), new GLFWgammaramp(red, green, blue, ¿?));
	return false;
}

//! Get the current Gamma Value for the Display
bool CIrrDeviceGLFW3::getGammaRamp( f32 &red, f32 &green, f32 &blue, f32 &brightness, f32 &contrast )
{
/*	GLFWgammaramp *ramp = glfwGetGammaRamp(glfwGetPrimaryMonitor());
	brightness = 0.f;
	contrast = 0.f;
	return (SDL_GetGamma(&ramp.red, &ramp.green, &ramp.blue) != -1);*/
	return false;
}

//! returns color format of the window.
video::ECOLOR_FORMAT CIrrDeviceGLFW3::getColorFormat() const
{
	if (window)
	{
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		if ((mode->redBits+mode->blueBits+mode->greenBits)==16)
		{
				return video::ECF_R5G6B5;
		}
		else
		{
				return video::ECF_R8G8B8;
		}
	}
	else
		return CIrrDeviceStub::getColorFormat();
}


void CIrrDeviceGLFW3::createKeyMap()
{
	// I don't know if this is the best method  to create
	// the lookuptable, but I'll leave it like that until
	// I find a better version.

	KeyMap.reallocate(105);

	// buttons missing

	KeyMap.push_back(SKeyMap(GLFW_KEY_BACKSPACE, KEY_BACK));
	KeyMap.push_back(SKeyMap(GLFW_KEY_TAB , KEY_TAB));
	//KeyMap.push_back(SKeyMap(SDLK_CLEAR, KEY_CLEAR));
	KeyMap.push_back(SKeyMap(GLFW_KEY_ENTER, KEY_RETURN));

	// combined modifiers missing

	KeyMap.push_back(SKeyMap(GLFW_KEY_PAUSE, KEY_PAUSE));
	KeyMap.push_back(SKeyMap(GLFW_KEY_CAPS_LOCK, KEY_CAPITAL));

	// asian letter keys missing

	KeyMap.push_back(SKeyMap(GLFW_KEY_ESCAPE , KEY_ESCAPE));

	// asian letter keys missing

	KeyMap.push_back(SKeyMap(GLFW_KEY_SPACE, KEY_SPACE));
	KeyMap.push_back(SKeyMap(GLFW_KEY_PAGE_UP, KEY_PRIOR));
	KeyMap.push_back(SKeyMap(GLFW_KEY_PAGE_DOWN, KEY_NEXT));
	KeyMap.push_back(SKeyMap(GLFW_KEY_END , KEY_END));
	KeyMap.push_back(SKeyMap(GLFW_KEY_HOME , KEY_HOME));

	KeyMap.push_back(SKeyMap(GLFW_KEY_LEFT , KEY_LEFT));
	KeyMap.push_back(SKeyMap(GLFW_KEY_UP, KEY_UP));
	KeyMap.push_back(SKeyMap(GLFW_KEY_RIGHT, KEY_RIGHT));
	KeyMap.push_back(SKeyMap(GLFW_KEY_DOWN, KEY_DOWN));

	// select missing

	//KeyMap.push_back(SKeyMap(SDLK_PRINT, KEY_PRINT));

	// execute missing

	KeyMap.push_back(SKeyMap(GLFW_KEY_PRINT_SCREEN, KEY_SNAPSHOT));

	KeyMap.push_back(SKeyMap(GLFW_KEY_INSERT, KEY_INSERT));
	KeyMap.push_back(SKeyMap(GLFW_KEY_DELETE , KEY_DELETE));
	//KeyMap.push_back(SKeyMap(SDLK_HELP, KEY_HELP));

	KeyMap.push_back(SKeyMap(GLFW_KEY_0, KEY_KEY_0));
	KeyMap.push_back(SKeyMap(GLFW_KEY_1, KEY_KEY_1));
	KeyMap.push_back(SKeyMap(GLFW_KEY_2, KEY_KEY_2));
	KeyMap.push_back(SKeyMap(GLFW_KEY_3, KEY_KEY_3));
	KeyMap.push_back(SKeyMap(GLFW_KEY_4, KEY_KEY_4));
	KeyMap.push_back(SKeyMap(GLFW_KEY_5, KEY_KEY_5));
	KeyMap.push_back(SKeyMap(GLFW_KEY_6, KEY_KEY_6));
	KeyMap.push_back(SKeyMap(GLFW_KEY_7, KEY_KEY_7));
	KeyMap.push_back(SKeyMap(GLFW_KEY_8, KEY_KEY_8));
	KeyMap.push_back(SKeyMap(GLFW_KEY_9, KEY_KEY_9));

	KeyMap.push_back(SKeyMap(GLFW_KEY_A, KEY_KEY_A));
	KeyMap.push_back(SKeyMap(GLFW_KEY_B, KEY_KEY_B));
	KeyMap.push_back(SKeyMap(GLFW_KEY_C, KEY_KEY_C));
	KeyMap.push_back(SKeyMap(GLFW_KEY_D, KEY_KEY_D));
	KeyMap.push_back(SKeyMap(GLFW_KEY_E, KEY_KEY_E));
	KeyMap.push_back(SKeyMap(GLFW_KEY_F, KEY_KEY_F));
	KeyMap.push_back(SKeyMap(GLFW_KEY_G, KEY_KEY_G));
	KeyMap.push_back(SKeyMap(GLFW_KEY_H, KEY_KEY_H));
	KeyMap.push_back(SKeyMap(GLFW_KEY_I, KEY_KEY_I));
	KeyMap.push_back(SKeyMap(GLFW_KEY_J, KEY_KEY_J));
	KeyMap.push_back(SKeyMap(GLFW_KEY_K, KEY_KEY_K));
	KeyMap.push_back(SKeyMap(GLFW_KEY_L, KEY_KEY_L));
	KeyMap.push_back(SKeyMap(GLFW_KEY_M, KEY_KEY_M));
	KeyMap.push_back(SKeyMap(GLFW_KEY_N, KEY_KEY_N));
	KeyMap.push_back(SKeyMap(GLFW_KEY_O, KEY_KEY_O));
	KeyMap.push_back(SKeyMap(GLFW_KEY_P, KEY_KEY_P));
	KeyMap.push_back(SKeyMap(GLFW_KEY_Q, KEY_KEY_Q));
	KeyMap.push_back(SKeyMap(GLFW_KEY_R, KEY_KEY_R));
	KeyMap.push_back(SKeyMap(GLFW_KEY_S, KEY_KEY_S));
	KeyMap.push_back(SKeyMap(GLFW_KEY_T, KEY_KEY_T));
	KeyMap.push_back(SKeyMap(GLFW_KEY_U, KEY_KEY_U));
	KeyMap.push_back(SKeyMap(GLFW_KEY_V, KEY_KEY_V));
	KeyMap.push_back(SKeyMap(GLFW_KEY_W, KEY_KEY_W));
	KeyMap.push_back(SKeyMap(GLFW_KEY_X, KEY_KEY_X));
	KeyMap.push_back(SKeyMap(GLFW_KEY_Y, KEY_KEY_Y));
	KeyMap.push_back(SKeyMap(GLFW_KEY_Z, KEY_KEY_Z));

	KeyMap.push_back(SKeyMap(GLFW_KEY_LEFT_SUPER , KEY_LWIN));
	KeyMap.push_back(SKeyMap(GLFW_KEY_RIGHT_SUPER , KEY_RWIN));

	// apps missing

	KeyMap.push_back(SKeyMap(GLFW_KEY_MENU , KEY_SLEEP)); //??

	KeyMap.push_back(SKeyMap(GLFW_KEY_KP_0, KEY_NUMPAD0));
	KeyMap.push_back(SKeyMap(GLFW_KEY_KP_1, KEY_NUMPAD1));
	KeyMap.push_back(SKeyMap(GLFW_KEY_KP_2, KEY_NUMPAD2));
	KeyMap.push_back(SKeyMap(GLFW_KEY_KP_3, KEY_NUMPAD3));
	KeyMap.push_back(SKeyMap(GLFW_KEY_KP_4, KEY_NUMPAD4));
	KeyMap.push_back(SKeyMap(GLFW_KEY_KP_5, KEY_NUMPAD5));
	KeyMap.push_back(SKeyMap(GLFW_KEY_KP_6, KEY_NUMPAD6));
	KeyMap.push_back(SKeyMap(GLFW_KEY_KP_7, KEY_NUMPAD7));
	KeyMap.push_back(SKeyMap(GLFW_KEY_KP_8, KEY_NUMPAD8));
	KeyMap.push_back(SKeyMap(GLFW_KEY_KP_9, KEY_NUMPAD9));
	KeyMap.push_back(SKeyMap(GLFW_KEY_KP_MULTIPLY, KEY_MULTIPLY));
	KeyMap.push_back(SKeyMap(GLFW_KEY_KP_ADD, KEY_ADD));
	KeyMap.push_back(SKeyMap(GLFW_KEY_KP_SUBTRACT, KEY_SUBTRACT));
	KeyMap.push_back(SKeyMap(GLFW_KEY_KP_DECIMAL, KEY_DECIMAL));
	KeyMap.push_back(SKeyMap(GLFW_KEY_KP_DIVIDE , KEY_DIVIDE));
//	KeyMap.push_back(SKeyMap(SDLK_KP_, KEY_SEPARATOR));

	KeyMap.push_back(SKeyMap(GLFW_KEY_F1 ,  KEY_F1));
	KeyMap.push_back(SKeyMap(GLFW_KEY_F2 ,  KEY_F2));
	KeyMap.push_back(SKeyMap(GLFW_KEY_F3 ,  KEY_F3));
	KeyMap.push_back(SKeyMap(GLFW_KEY_F4 ,  KEY_F4));
	KeyMap.push_back(SKeyMap(GLFW_KEY_F5 ,  KEY_F5));
	KeyMap.push_back(SKeyMap(GLFW_KEY_F6 ,  KEY_F6));
	KeyMap.push_back(SKeyMap(GLFW_KEY_F7 ,  KEY_F7));
	KeyMap.push_back(SKeyMap(GLFW_KEY_F8 ,  KEY_F8));
	KeyMap.push_back(SKeyMap(GLFW_KEY_F9 ,  KEY_F9));
	KeyMap.push_back(SKeyMap(GLFW_KEY_F10 , KEY_F10));
	KeyMap.push_back(SKeyMap(GLFW_KEY_F11 , KEY_F11));
	KeyMap.push_back(SKeyMap(GLFW_KEY_F12 , KEY_F12));
	KeyMap.push_back(SKeyMap(GLFW_KEY_F13 , KEY_F13));
	KeyMap.push_back(SKeyMap(GLFW_KEY_F14 , KEY_F14));
	KeyMap.push_back(SKeyMap(GLFW_KEY_F15 , KEY_F15));

	// no higher F-keys

	KeyMap.push_back(SKeyMap(GLFW_KEY_NUM_LOCK, KEY_NUMLOCK));
	KeyMap.push_back(SKeyMap(GLFW_KEY_SCROLL_LOCK, KEY_SCROLL));
	KeyMap.push_back(SKeyMap(GLFW_KEY_LEFT_SHIFT, KEY_LSHIFT));
	KeyMap.push_back(SKeyMap(GLFW_KEY_RIGHT_SHIFT, KEY_RSHIFT));
	KeyMap.push_back(SKeyMap(GLFW_KEY_LEFT_CONTROL,  KEY_LCONTROL));
	KeyMap.push_back(SKeyMap(GLFW_KEY_RIGHT_CONTROL,  KEY_RCONTROL));
	KeyMap.push_back(SKeyMap(GLFW_KEY_LEFT_ALT,  KEY_LMENU));
	KeyMap.push_back(SKeyMap(GLFW_KEY_RIGHT_ALT,  KEY_RMENU));

	//KeyMap.push_back(SKeyMap(SDLK_PLUS,   KEY_PLUS));
	KeyMap.push_back(SKeyMap(GLFW_KEY_COMMA,  KEY_COMMA));
	KeyMap.push_back(SKeyMap(GLFW_KEY_MINUS,  KEY_MINUS));
	KeyMap.push_back(SKeyMap(GLFW_KEY_PERIOD , KEY_PERIOD));

	// some special keys missing

	KeyMap.sort();
}

GLFWwindow *CIrrDeviceGLFW3::getWindow(){
	return window;
}

} // end namespace irr

#endif // _IRR_COMPILE_WITH_GLFW3_DEVICE_

