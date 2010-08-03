/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * gqt4_main.hpp: Main UI code.                                            *
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

#ifndef __GENS_QT4_GQT4_MAIN_HPP__
#define __GENS_QT4_GQT4_MAIN_HPP__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Win32 uses WinMain(), so we need to call the
 * main function gqt4_main(). It's renamed to main
 * on non-Win32 platforms.
 */
#ifndef _WIN32
#define gqt4_main main
#endif

/**
 * gqt4_main(): Main entry point.
 * @param argc argc (0 on Win32)
 * @param argv argv (NULL on Win32)
 * @return Return value.
 */
int gqt4_main(int argc, char *argv[]);

/**
 * gqt4_log_msg_critical(): LOG_MSG() critical error handler.
 * @param channel Debug channel.
 * @param msg Message. (Preformatted)
 */
void gqt4_log_msg_critical(const char *channel, const char *msg);

#ifdef __cplusplus
}
#endif

namespace GensQt4
{
	void QuitGens(void);
}

#endif /* __GENS_QT4_GQT4_MAIN_HPP__ */
