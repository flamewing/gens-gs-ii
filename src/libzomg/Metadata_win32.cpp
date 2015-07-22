/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Metadata_unix.cpp: Metadata handler. (Windows)                          *
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

#include "Metadata_p.hpp"

// System includes.
#include <windows.h>
// Needed for GetUserNameEx();
#define SECURITY_WIN32
#include <security.h>

// C++ includes.
#include <string>
#include <sstream>
using std::string;
using std::ostringstream;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

namespace LibZomg {

/**
 * Get the current time.
 * This function is implemented in OS-specific files.
 */
void MetadataPrivate::init_ctime(void)
{
	// NOTE: Using FILETIME due to higher accuracy.
	// - SYSTEMTIME only supports milliseconds, and
	//   it's split up into logical date fields.
	// - FILETIME is in units of 100 nanoseconds.
	FILETIME fileTime;
	ULARGE_INTEGER largeInteger;
	GetSystemTimeAsFileTime(&fileTime);
	largeInteger.LowPart = fileTime.dwLowDateTime;
	largeInteger.HighPart = fileTime.dwHighDateTime;

	// Convert to Unix time format.
	// Note that the Windows epoch is January 1, 1601 00:00 UTC.
	// References:
	// - https://msdn.microsoft.com/en-us/library/windows/desktop/ms724284%28v=vs.85%29.aspx
	// - http://stackoverflow.com/questions/19709580/c-convert-filetime-to-seconds
	ctime.seconds = (largeInteger.QuadPart / 10000000ULL) - 11644473600ULL;
	ctime.nano = (largeInteger.QuadPart % 10000000ULL) * 100;
}

/**
 * Get the Windows username. (Unicode version)
 * The display name is checked first.
 * If it's empty or invalid, the username is checked.
 * @return Windows username.
 */
static inline string getUserName_unicode(void)
{
	string ret;
	wchar_t username[256];
	DWORD cchUsername = ARRAY_SIZE(username);
	if (!GetUserNameExW(NameDisplay, username, &cchUsername)) {
		// Error retrieving display name.
		if (!GetUserNameW(username, &cchUsername)) {
			// Error retrieving username.
			// TODO: Check Registered Owner in the registry?
			cchUsername = 0;
		}
	}

	if (cchUsername > 0) {
		// Try to convert from UTF-16 to UTF-8.
		// FIXME: If this fails, do a naive low-byte conversion?

		// Next, try to convert from UTF-16 to UTF-8.
		int cbMbs = WideCharToMultiByte(CP_UTF8, 0, username, cchUsername, nullptr, 0, nullptr, nullptr);
		if (cbMbs > 0) {
			char *mbs = (char*)malloc(cbMbs);
			WideCharToMultiByte(CP_UTF8, 0, username, cchUsername, mbs, cbMbs, nullptr, nullptr);

			// Save the UTF-8 string.
			ret = string(mbs, cbMbs);
			free(mbs);
		}
	}

	return ret;
}

/**
 * Get the Windows username. (ANSI version)
 * The display name is checked first.
 * If it's empty or invalid, the username is checked.
 * @return Windows username.
 */
static inline string getUserName_ansi(void)
{
	string ret;
	char username[256];
	DWORD cbUsername = ARRAY_SIZE(username);
	if (!GetUserNameExA(NameDisplay, username, &cbUsername)) {
		// Error retrieving display name.
		if (!GetUserNameA(username, &cbUsername)) {
			// Error retrieving username.
			// TODO: Check Registered Owner in the registry?
			cbUsername = 0;
		}
	}

	if (cbUsername > 0) {
		// Try to convert from ANSI to UTF-8.

		// First, convert from ANSI to UTF-16
		int cchWcs = MultiByteToWideChar(CP_ACP, 0, username, cbUsername, nullptr, 0);
		if (cchWcs > 0) {
			wchar_t *wcs = (wchar_t*)malloc(cchWcs * sizeof(wchar_t));
			MultiByteToWideChar(CP_ACP, 0, username, cbUsername, wcs, cchWcs);

			// Next, try to convert from UTF-16 to UTF-8.
			int cbMbs = WideCharToMultiByte(CP_UTF8, 0, wcs, cchWcs, nullptr, 0, nullptr, nullptr);
			if (cbMbs > 0) {
				char *mbs = (char*)malloc(cbMbs);
				WideCharToMultiByte(CP_UTF8, 0, wcs, cchWcs, mbs, cbMbs, nullptr, nullptr);

				// Save the UTF-8 string.
				ret = string(mbs, cbMbs);
				free(mbs);
			}
			free(wcs);
		}
	}

	return ret;
}

/**
 * Initialize system-specific metadata.
 * This function is implemented in OS-specific files.
 */
void MetadataPrivate::InitSystemMetadata(void)
{
	// OS version.
	// FIXME: This will return "Windows 8" on 8.1+.
	// Potential workaround is NetWkstaGetInfo().
	// There should also be a kernel function...
	// http://www.codeproject.com/Articles/678606/Part-Overcoming-Windows-s-deprecation-of-GetVe
	ostringstream oss;
	OSVERSIONINFOEXA osVersionInfo;
	osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
	if (GetVersionExA((LPOSVERSIONINFOA)&osVersionInfo) != 0) {
		// Version info retrieved.
		// Check if the product name is available in the registry.
		//reg query "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion" /v ProductName
		HKEY hKey;
		LONG ret = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey);
		if (ret == ERROR_SUCCESS) {
			// Key opened. Get the ProductName.
			// TODO: Needs testing on 9x, 2000, and XP.
			char productName[256];
			DWORD cbProductName = sizeof(productName);
			// 256 bytes should always be enough for the product name.
			ret = RegQueryValueExA(hKey, "ProductName", nullptr, nullptr, (LPBYTE)productName, &cbProductName);
			if (ret == ERROR_SUCCESS) {
				// Product name retrieved.
				// Make sure it's null-terminated.
				if (cbProductName < sizeof(productName)) {
					productName[cbProductName] = 0;
				}
				oss << string(productName);
			} else {
				// Could not retrieve product name.
				// Use a generic name.
				switch (osVersionInfo.dwPlatformId) {
					case VER_PLATFORM_WIN32s:
						// Yeah, right, like this is ever going to happen...
						oss << "Windows 3.1";
						break;
					case VER_PLATFORM_WIN32_WINDOWS:
						oss << "Windows 9x";
						break;
					case VER_PLATFORM_WIN32_NT:
					default:
						switch (osVersionInfo.wProductType) {
							case VER_NT_WORKSTATION:
							default:
								oss << "Windows NT";
								break;
							case VER_NT_DOMAIN_CONTROLLER:
							case VER_NT_SERVER:
								oss << "Windows NT Server";
								break;
						}
						break;
				}
			}
		}

		// Append service pack version.
		char spver[32];
		if (osVersionInfo.wServicePackMajor > 0 || osVersionInfo.wServicePackMinor > 0) {
			// Service Pack.
			if (osVersionInfo.wServicePackMinor > 0) {
				snprintf(spver, sizeof(spver), " SP%d.%d",
					osVersionInfo.wServicePackMajor,
					osVersionInfo.wServicePackMinor);
			} else {
				snprintf(spver, sizeof(spver), " SP%d",
					osVersionInfo.wServicePackMajor);
			}
			oss << string(spver);
		}

		// Append version number.
		snprintf(spver, sizeof(spver), " (%d.%d.%d)",
			osVersionInfo.dwMajorVersion,
			osVersionInfo.dwMinorVersion,
			osVersionInfo.dwBuildNumber);
		oss << string(spver);
	}
	sysInfo.osVersion = oss.str();

	// Get the username.
	if (GetModuleHandleW(nullptr)) {
		// OS supports Unicode.
		sysInfo.username = getUserName_unicode();
	} else {
		// OS does not support Unicode.
		sysInfo.username = getUserName_ansi();
	}

	// TODO: CPU information.
}

}