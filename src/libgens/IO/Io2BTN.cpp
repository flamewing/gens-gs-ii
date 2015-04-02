/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Io2BTN.cpp: Sega Master System 2-button controller.                     *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

#include "Io2BTN.hpp"

namespace LibGens { namespace IO {

Io2BTN::Io2BTN()
	: Device()
{ }

// Device type.
// Should be overridden by subclasses.
IoManager::IoType_t Io2BTN::type(void) const
{
	return IoManager::IOT_2BTN;
}

/**
 * Update the I/O device.
 * Runs the internal device update.
 */
void Io2BTN::update(void)
{
	// Update the tristate input cache.
	this->updateTristateInputCache();

	/**
	 * Data format: (x == tristate value)
	 * - xxCBRLDU
	 * B == button 1
	 * C == button 2
	 */
	this->deviceData = (0xC0 | (this->buttons & 0x3F));
}

} }
