// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
// This device code is based on the original GLFW3 device implementation
// contributed by Shane Parker (sirshane).

#ifndef __C_IRR_DEVICE_GLFW3_H_INCLUDED__
#define __C_IRR_DEVICE_GLFW3_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_GLFW3_DEVICE_

#include "IrrlichtDevice.h"
#include "CIrrDeviceStub.h"
#include "IImagePresenter.h"
#include "ICursorControl.h"

#include <GLFW/glfw3.h>

namespace irr
{

	class CIrrDeviceGLFW3 : public CIrrDeviceStub, video::IImagePresenter
	{
	public:

		//! constructor
		CIrrDeviceGLFW3(const SIrrlichtCreationParameters& param);

		//! destructor
		virtual ~CIrrDeviceGLFW3();

		//! runs the device. Returns false if device wants to be deleted
		virtual bool run() _IRR_OVERRIDE_;

		//! pause execution temporarily
		virtual void yield() _IRR_OVERRIDE_;

		//! pause execution for a specified time
		virtual void sleep(u32 timeMs, bool pauseTimer) _IRR_OVERRIDE_;

		//! sets the caption of the window
		virtual void setWindowCaption(const wchar_t* text) _IRR_OVERRIDE_;

		//! returns if window is active. if not, nothing need to be drawn
		virtual bool isWindowActive() const _IRR_OVERRIDE_;

		//! returns if window has focus.
		bool isWindowFocused() const _IRR_OVERRIDE_;

		//! returns if window is minimized.
		bool isWindowMinimized() const _IRR_OVERRIDE_;

		//! returns color format of the window.
		video::ECOLOR_FORMAT getColorFormat() const _IRR_OVERRIDE_;

		//! presents a surface in the client area
		virtual bool present(video::IImage* surface, void* windowId=0, core::rect<s32>* src=0) _IRR_OVERRIDE_;

		//! notifies the device that it should close itself
		virtual void closeDevice() _IRR_OVERRIDE_;

		//! \return Returns a pointer to a list with all video modes supported
		virtual video::IVideoModeList* getVideoModeList() _IRR_OVERRIDE_;

		//! Sets if the window should be resizable in windowed mode.
		virtual void setResizable(bool resize=false) _IRR_OVERRIDE_;

		//! Minimizes the window.
		virtual void minimizeWindow() _IRR_OVERRIDE_;

		//! Maximizes the window.
		virtual void maximizeWindow() _IRR_OVERRIDE_;

		//! Restores the window size.
		virtual void restoreWindow() _IRR_OVERRIDE_;

		//! Get the position of this window on screen
		virtual core::position2di getWindowPosition() _IRR_OVERRIDE_;

		//! Activate any joysticks, and generate events for them.
		virtual bool activateJoysticks(core::array<SJoystickInfo> & joystickInfo) _IRR_OVERRIDE_;

		//! Set the current Gamma Value for the Display
		virtual bool setGammaRamp( f32 red, f32 green, f32 blue, f32 brightness, f32 contrast ) _IRR_OVERRIDE_;

		//! Get the current Gamma Value for the Display
		virtual bool getGammaRamp( f32 &red, f32 &green, f32 &blue, f32 &brightness, f32 &contrast ) _IRR_OVERRIDE_;

		//! Get the device type
		virtual E_DEVICE_TYPE getType() const _IRR_OVERRIDE_
		{
				return EIDT_GLFW3;
		}

		//get the window
		virtual GLFWwindow *getWindow();

		//! Implementation of the linux cursor control
		class CCursorControl : public gui::ICursorControl
		{
		public:

			CCursorControl(CIrrDeviceGLFW3* dev)
				: Device(dev), IsVisible(true)
			{
			}

			//! Changes the visible state of the mouse cursor.
			virtual void setVisible(bool visible) _IRR_OVERRIDE_
			{
				IsVisible = visible;
				if ( visible )
					glfwSetInputMode(Device->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
				else
					glfwSetInputMode(Device->window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			}

			//! Returns if the cursor is currently visible.
			virtual bool isVisible() const _IRR_OVERRIDE_
			{
				return IsVisible;
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(const core::position2d<f32> &pos) _IRR_OVERRIDE_
			{
				setPosition(pos.X, pos.Y);
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(f32 x, f32 y) _IRR_OVERRIDE_
			{
				setPosition((s32)(x*Device->Width), (s32)(y*Device->Height));
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(const core::position2d<s32> &pos) _IRR_OVERRIDE_
			{
				setPosition(pos.X, pos.Y);
			}

			//! Sets the new position of the cursor.
			virtual void setPosition(s32 x, s32 y) _IRR_OVERRIDE_
			{
				glfwSetCursorPos (Device->window, x, y);
			}

			//! Returns the current position of the mouse cursor.
			virtual const core::position2d<s32>& getPosition() _IRR_OVERRIDE_
			{
				updateCursorPos();
				return CursorPos;
			}

			//! Returns the current position of the mouse cursor.
			virtual core::position2d<f32> getRelativePosition() _IRR_OVERRIDE_
			{
				updateCursorPos();
				return core::position2d<f32>(CursorPos.X / (f32)Device->Width,
					CursorPos.Y / (f32)Device->Height);
			}

			//
			virtual void setReferenceRect(core::rect<s32>* rect=0) _IRR_OVERRIDE_
			{
				//Nothing to do here
			}

			//! Sets the active cursor icon
			virtual void setActiveIcon(gui::ECURSOR_ICON iconId) _IRR_OVERRIDE_
			{
				GLFWcursor* cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR); //ToDo: tipo de cursor
				glfwSetCursor(Device->window, cursor);
			}

			//! Gets the currently active icon
			//virtual gui::ECURSOR_ICON getActiveIcon() const _IRR_OVERRIDE_ = 0;

			//! Add a custom sprite as cursor icon.
			//virtual gui::ECURSOR_ICON addIcon(const gui::SCursorSprite& icon) _IRR_OVERRIDE_ = 0;
			/*{
				unsigned char pixels[16 * 16 * 4];
				memset(pixels, 0xff, sizeof(pixels));
				GLFWimage image;
				image.width = 16;
				image.height = 16;
				image.pixels = pixels;
				GLFWcursor* cursor = glfwCreateCursor(&image, 0, 0);
			}*/

			//! replace the given cursor icon.
			//virtual void changeIcon(gui::ECURSOR_ICON iconId, const gui::SCursorSprite& icon) _IRR_OVERRIDE_ = 0;

			//! Return a system-specific size which is supported for cursors. Larger icons will fail, smaller icons might work.
			//virtual core::dimension2di getSupportedIconSize() const _IRR_OVERRIDE_ = 0;

		private:

			void updateCursorPos()
			{
				CursorPos.X = Device->MouseX;
				CursorPos.Y = Device->MouseY;

				if (CursorPos.X < 0)
					CursorPos.X = 0;
				if (CursorPos.X > (s32)Device->Width)
					CursorPos.X = Device->Width;
				if (CursorPos.Y < 0)
					CursorPos.Y = 0;
				if (CursorPos.Y > (s32)Device->Height)
					CursorPos.Y = Device->Height;
			}

			CIrrDeviceGLFW3* Device;
			core::position2d<s32> CursorPos;
			bool IsVisible;
		};

	private:

		//! create the driver
		void createDriver();

		bool createWindow();

		void createKeyMap();

		GLFWwindow* window;
		
#if defined(_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
		core::array<int> Joysticks;
#endif

		SEvent irrevent;

		s32 MouseX, MouseY;
		u32 MouseButtonStates;

		u32 Width, Height;

		bool Resizable;
		bool WindowHasFocus;
		bool WindowMinimized;

		struct SKeyMap
		{
			SKeyMap() {}
			SKeyMap(s32 x11, s32 win32)
				: GLFW3Key(x11), Win32Key(win32)
			{
			}

			s32 GLFW3Key;
			s32 Win32Key;

			bool operator<(const SKeyMap& o) const
			{
				return GLFW3Key < o.GLFW3Key; //ToDo: revisar
			}
		};

		core::array<SKeyMap> KeyMap;
	};
	
	//ToDo: joysticks

} // end namespace irr

#endif // _IRR_COMPILE_WITH_GLFW3_DEVICE_
#endif // __C_IRR_DEVICE_GLFW3_H_INCLUDED__

