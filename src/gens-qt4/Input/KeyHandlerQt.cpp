/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * KeyHandlerQt.cpp: Qt key remapping handler.                             *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "KeyHandlerQt.hpp"

// LibGens includes.
#include "libgens/IO/KeyManager.hpp"
#include "libgens/IO/GensKey_t.h"

// Qt includes
#include <qglobal.h>

// Native virtual keycodes.
#if defined(Q_WS_X11)
#include <X11/keysym.h>
#elif defined(Q_WS_WIN)
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace GensQt4
{

/**
 * KeyPressEvent(): Key press handler.
 * @param event Key event.
 */
void KeyHandlerQt::KeyPressEvent(QKeyEvent *event)
{
	// TODO: Move effects keypresses from GensQGLWidget to KeyHandlerQt.
	// TODO: Multiple keyboard support?
	int gensKey = QKeyEventToKeyVal(event);
	if (gensKey > KEYV_UNKNOWN)
		LibGens::KeyManager::KeyPressEvent(gensKey);
}


/**
 * KeyPressEvent(): Key release handler.
 * @param event Key event.
 */
void KeyHandlerQt::KeyReleaseEvent(QKeyEvent *event)
{
	// TODO: Multiple keyboard support?
	int gensKey = QKeyEventToKeyVal(event);
	if (gensKey > KEYV_UNKNOWN)
		LibGens::KeyManager::KeyReleaseEvent(gensKey);
}


/**
 * MouseMoveEvent(): Mouse movement handler.
 * TODO: This function is broken!
 * @param event Mouse event.
 */
void KeyHandlerQt::MouseMoveEvent(QMouseEvent *event)
{
	// TODO
#if 0
	if (!gqt4_emuThread)
	{
		m_lastMousePosValid = false;
		return;
	}
	
	if (!m_lastMousePosValid)
	{
		// Last mouse movement event was invalid.
		m_lastMousePos = event->pos();
		m_lastMousePosValid = true;
		return;
	}
	
	// Calculate the relative movement.
	QPoint posDiff = (event->pos() - m_lastMousePos);
	m_lastMousePos = event->pos();
	
	// Forward the relative movement to the I/O devices.
	// NOTE: Port E isn't forwarded, since it isn't really usable as a controller.
	LibGens::EmuMD::m_port1->mouseMove(posDiff.x(), posDiff.y());
	LibGens::EmuMD::m_port2->mouseMove(posDiff.x(), posDiff.y());
#endif
}


/**
 * MousePressEvent(): Mouse button press handler.
 * @param event Mouse event.
 */
void KeyHandlerQt::MousePressEvent(QMouseEvent *event)
{
	int gensButton;
	switch (event->button())
	{
		case Qt::NoButton:	return;
		case Qt::LeftButton:	gensButton = MBTN_LEFT; break;
		case Qt::MidButton:	gensButton = MBTN_MIDDLE; break;
		case Qt::RightButton:	gensButton = MBTN_RIGHT; break;
		case Qt::XButton1:	gensButton = MBTN_X1; break;
		case Qt::XButton2:	gensButton = MBTN_X2; break;
		default:		gensButton = MBTN_UNKNOWN; break;
	}
	
	LibGens::KeyManager::MousePressEvent(gensButton);
}


/**
 * MouseReleaseEvent(): Mouse button release handler.
 * @param event Mouse event.
 */
void KeyHandlerQt::MouseReleaseEvent(QMouseEvent *event)
{
	int gensButton;
	switch (event->button())
	{
		case Qt::NoButton:	return;
		case Qt::LeftButton:	gensButton = MBTN_LEFT; break;
		case Qt::MidButton:	gensButton = MBTN_MIDDLE; break;
		case Qt::RightButton:	gensButton = MBTN_RIGHT; break;
		case Qt::XButton1:	gensButton = MBTN_X1; break;
		case Qt::XButton2:	gensButton = MBTN_X2; break;
		default:		gensButton = MBTN_UNKNOWN; break;
	}
	
	LibGens::KeyManager::MouseReleaseEvent(gensButton);
}


/**
 * QKeyEventToKeyVal(): Convert a QKeyEvent to a LibGens key value.
 * TODO: Move somewhere else?
 * @param event QKeyEvent.
 * @return LibGens key value. (0 for unknown; -1 for unhandled left/right modifier key.)
 */
int KeyHandlerQt::QKeyEventToKeyVal(QKeyEvent *event)
{
	using namespace LibGens;
	
	// Table of Qt::Keys in range 0x00-0x7F.
	// (Based on Qt 4.6.3)
	static const int QtKey_Ascii[0x80] =
	{
		// 0x00
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		
		// 0x20
		KEYV_SPACE, KEYV_EXCLAIM, KEYV_QUOTEDBL, KEYV_HASH,
		KEYV_DOLLAR, KEYV_PERCENT, KEYV_AMPERSAND, KEYV_QUOTE,
		KEYV_LEFTPAREN, KEYV_RIGHTPAREN, KEYV_ASTERISK, KEYV_PLUS,
		KEYV_COMMA, KEYV_MINUS, KEYV_PERIOD, KEYV_SLASH,
		
		// 0x30
		KEYV_0, KEYV_1, KEYV_2, KEYV_3, KEYV_4, KEYV_5, KEYV_6, KEYV_7,
		KEYV_8, KEYV_9, KEYV_COLON, KEYV_SEMICOLON,
		KEYV_LESS, KEYV_EQUALS, KEYV_GREATER, KEYV_QUESTION,
		
		// 0x40
		KEYV_AT, KEYV_a, KEYV_b, KEYV_c, KEYV_d, KEYV_e, KEYV_f, KEYV_g,
		KEYV_h, KEYV_i, KEYV_j, KEYV_k, KEYV_l, KEYV_m, KEYV_n, KEYV_o,
		KEYV_p, KEYV_q, KEYV_r, KEYV_s, KEYV_t, KEYV_u, KEYV_v, KEYV_w,
		KEYV_x, KEYV_y, KEYV_z, KEYV_LEFTBRACKET,
		KEYV_BACKSLASH, KEYV_RIGHTBRACKET, KEYV_CARET, KEYV_UNDERSCORE,
		
		// 0x60
		KEYV_BACKQUOTE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, KEYV_BRACELEFT,
		KEYV_BAR, KEYV_BRACERIGHT, KEYV_TILDE, KEYV_DELETE,
	};
	
	// Table of Qt::Keys in range 0x01000000-0x0100007F.
	// (Based on Qt 4.6.3)
	// TODO: Check how numpad keys act with numlock on/off!
	// NOTE: Keys with a value of -1 aren't handled by Qt. (left/right modifiers)
	// NOTE: Media keys are not included.
	static const int QtKey_Extended[0x80] =
	{
		// 0x01000000
		KEYV_ESCAPE, KEYV_TAB, KEYV_TAB, KEYV_BACKSPACE,
		KEYV_RETURN, KEYV_KP_ENTER, KEYV_INSERT, KEYV_DELETE,
		KEYV_PAUSE, KEYV_PRINT, KEYV_SYSREQ, KEYV_CLEAR,
		0, 0, 0, 0,
		
		// 0x01000010
		KEYV_HOME, KEYV_END, KEYV_LEFT, KEYV_UP,
		KEYV_RIGHT, KEYV_DOWN, KEYV_PAGEUP, KEYV_PAGEDOWN,
		0, 0, 0, 0, 0, 0, 0, 0,
		
		// 0x01000020
		-1, -1, -1, -1, KEYV_CAPSLOCK, KEYV_NUMLOCK, KEYV_SCROLLLOCK, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		
		// 0x01000030
		KEYV_F1, KEYV_F2, KEYV_F3, KEYV_F4, KEYV_F5, KEYV_F6, KEYV_F7, KEYV_F8,
		KEYV_F9, KEYV_F10, KEYV_F11, KEYV_F12, KEYV_F13, KEYV_F14, KEYV_F15, KEYV_F16,
		KEYV_F17, KEYV_F18, KEYV_F19, KEYV_F20, KEYV_F21, KEYV_F22, KEYV_F23, KEYV_F24,
		KEYV_F25, KEYV_F26, KEYV_F27, KEYV_F28, KEYV_F29, KEYV_F30, KEYV_F31, KEYV_F32,
		
		// 0x01000050
		0, 0, 0, -1, -1, KEYV_MENU, -1, -1,
		KEYV_HELP, 0, 0, 0, 0, 0, 0, 0,
		
		// 0x01000060
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	
	int key = event->key();
	switch (key & ~0x7F)
	{
		case 0x00000000:
			// ASCII key.
			// TODO: Make sure Qt doesn't apply shifting!
			return QtKey_Ascii[key];
		
		case 0x01000000:
		{
			// Extended key.
			int gensKey = QtKey_Extended[key & 0x7F];
			if (gensKey >= 0)
				return gensKey;
			
			// Extended key is not handled by Qt.
			// This happens with e.g. left/right shift.
			// (Qt reports both keys as the same.)
			return NativeModifierToKeyVal(event);
		}
			
		default:
			// Other key.
			switch (key)
			{
				case Qt::Key_AltGr:
					return KEYV_MODE;
				case Qt::Key_Multi_key:
					return KEYV_COMPOSE;
				default:
					return 0;
			}
	}
}


/**
 * NativeModifierToKeyVal(): Convert a native virtual key for a modifier to a LibGens key value.
 * TODO: Move somewhere else?
 * @param event QKeyEvent.
 * @return LibGens key value. (0 for unknown)
 */
#include <stdio.h>
int KeyHandlerQt::NativeModifierToKeyVal(QKeyEvent *event)
{
	using namespace LibGens;
	
#if defined(Q_WS_X11)
	// X11 keysym.
	switch (event->nativeVirtualKey())
	{
		case XK_Shift_L:	return KEYV_LSHIFT;
		case XK_Shift_R:	return KEYV_RSHIFT;
		case XK_Control_L:	return KEYV_LCTRL;
		case XK_Control_R:	return KEYV_RCTRL;
		case XK_Meta_L:		return KEYV_LMETA;
		case XK_Meta_R:		return KEYV_RMETA;
		case XK_Alt_L:		return KEYV_LALT;
		case XK_Alt_R:		return KEYV_RALT;
		case XK_Super_L:	return KEYV_LSUPER;
		case XK_Super_R:	return KEYV_RSUPER;
		case XK_Hyper_L:	return KEYV_LHYPER;
		case XK_Hyper_R:	return KEYV_RHYPER;
		default:		break;
	}
#elif defined(Q_WS_WIN)
	// Win32 virtual key.
	// NOTE: Shift, Control, and Alt are NOT tested here.
	// WM_KEYDOWN/WM_KEYUP report VK_SHIFT, VK_CONTORL, and VK_MENU (Alt).
	// These are useless for testing left/right keys.
	// Instead, GetAsyncKeyState() is used in GensWindow::emuFrameDone().
	switch (event->nativeVirtualKey())
	{
		case VK_LWIN:		return KEYV_LSUPER;
		case VK_RWIN:		return KEYV_RSUPER;
		default:		break;
	}
#elif defined(Q_WS_MAC)
	/**
	 * FIXME: Mac OS X doesn't allow user applications to
	 * determine if left or right modifier keys are pressed.
	 * (There should be flag somewhere that enables it,
	 * since there are constants defined for both left
	 * and right keys, but I can't seem to find them...)
	 * 
	 * For now, just use Left keys. (default handler)
	 */
#else
	// Unhandled system.
	#warning Unhandled system; modifier keys will fall back to Left variants!!
#endif
	
	// Unhandled key. Return left key by default.
	switch (event->key())
	{
		case Qt::Key_Shift:	return KEYV_LSHIFT;
#ifdef Q_OS_MAC
		// Qt/Mac remaps some keys:
		// Qt::Key_Control == Command
		// Qt::Key_Meta == Control
		case Qt::Key_Control:	return KEYV_LSUPER;
		case Qt::Key_Meta:	return KEYV_LCTRL;
#else
		case Qt::Key_Control:	return KEYV_LCTRL;
		case Qt::Key_Meta:	return KEYV_LMETA;
#endif /* Q_OS_MAC */
		case Qt::Key_Alt:	return KEYV_LALT;
		case Qt::Key_Super_L:	return KEYV_LSUPER;
		case Qt::Key_Super_R:	return KEYV_RSUPER;
		case Qt::Key_Hyper_L:	return KEYV_LHYPER;
		case Qt::Key_Hyper_R:	return KEYV_RHYPER;
		default:		return KEYV_UNKNOWN;
	}
}

}
