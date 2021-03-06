/***************************************************************************
 * libcompat/W32U: Win32 Unicode Translation Layer. (Mini Version)         *
 * W32U_argv.c: UTF-8 conversion for argv[].                               *
 *                                                                         *
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

#include <config.libcompat.h>

#define __IN_W32U__
#include "W32U_argv.h"

// C includes.
#include <assert.h>
#include <wchar.h>
#include <stdlib.h>

// Windows includes.
#include <windows.h>

// W32U_IsUnicode()
#include "is_unicode.h"

#if defined(_MSC_VER)
#if _MSC_VER < 1900 && !defined(__cplusplus)
// MSVC 2015 added 'inline' in C mode.
// Older versions don't have it, so #define inline here.
#define inline __inline
#endif /* _MSC_VER < 1900 && !defined(__cplusplus) */

#if _MSC_VER >= 1900
// MSVC 2015 removed __wgetmainargs() and __getmainargs().
// Use CommandLineToArgvW() instead.
#include <shellapi.h>
#define USE_COMMANDLINETOARGVW 1
#endif
#endif /* _MSC_VER */

#if !defined(USE_COMMANDLINETOARGVW)
typedef struct {
	int newmode;
} _startupinfo;
_CRTIMP int __cdecl __wgetmainargs(int * _Argc, wchar_t ***_Argv, wchar_t ***_Env, int _DoWildCard, _startupinfo *_StartInfo);
_CRTIMP int __cdecl __getmainargs(int * _Argc, char ***_Argv, char ***_Env, int _DoWildCard, _startupinfo *_StartInfo);
#endif

// Converted parameters are stored here.
static int saved_argcU = 0;
static char **saved_argvU = NULL;
static char **saved_envpU = NULL;

/** ANSI to UTF-16 **/

#ifdef ENABLE_ANSI_WINDOWS
/**
 * Convert UTF-8 arguments to UTF-16.
 * @param argcA		[in] Number of UTF-8 arguments.
 * @param argvA		[in] UTF-8 arguments.
 * @param p_argcW	[out] Number of UTF-16 arguments.
 * @param p_argvW	[out] Newly-allocated UTF-16 arguments.
 * @return 0 on success; non-zero on error.
 * On success, p_argvW will contain a malloc()'d block.
 * Caller must free it when it's done using it.
 */
static int convertArgsAtoW(int argcA, const char *argvA[], int *p_argcW, wchar_t **p_argvW[])
{
	// Temporary variables.
	int ret = -1;
	int i;
	// Block sizes.
	int cbArgvWPtr, cchArgvWStr;
	int cchArgvWStrLeft;
	// UTF-16 data.
	int argcW = 0;
	wchar_t **argvW = NULL;
	// UTF-16 block pointer.
	wchar_t *strW;

	// Determine the total length of the argv block.
	cbArgvWPtr = (int)((argcA + 1) * sizeof(wchar_t*));
	cchArgvWStr = 0;
	for (i = 0; i < argcA; i++) {
		ret = MultiByteToWideChar(CP_ACP, 0, argvA[i], -1, NULL, 0);
		if (ret <= 0) {
			// MultiByteToWideChar() failed!
			// Stop processing.
			ret = GetLastError();
			goto out;
		}
		cchArgvWStr += ret;
	}

	// Allocate the argv block.
	// The first portion of the block is argv[];
	// the second portion contains the actual string data.
	argvW = (wchar_t**)malloc(cbArgvWPtr + (cchArgvWStr * sizeof(wchar_t)));
	strW = (wchar_t*)((char*)argvW + cbArgvWPtr);
	cchArgvWStrLeft = cchArgvWStr;

	// Convert the arguments from ANSI to UTF-16.
	for (i = 0; i < argcA; i++) {
		assert(cchArgvWStrLeft > 0);
		if (cchArgvWStrLeft <= 0) {
			// Out of space in argvW...
			ret = ERROR_INSUFFICIENT_BUFFER;
			goto out;
		}

		ret = MultiByteToWideChar(CP_ACP, 0, argvA[i], -1, strW, cchArgvWStrLeft);
		if (ret <= 0) {
			// MultiByteToWideChar() failed.
			// Stop processing.
			ret = GetLastError();
			goto out;
		}

		argvW[i] = strW;
		strW += ret;
		cchArgvWStrLeft -= ret;
	}

	// Set the last entry in argvA to NULL.
	argcW = argcA;
	argvW[argcW] = NULL;
	ret = 0;

out:
	if (ret != 0) {
		// An error occurred.
		free(argvW);
		return ret;
	}

	*p_argcW = argcW;
	*p_argvW = argvW;
	return 0;
}

/**
 * Convert ANSI environment to UTF-16.
 * @param envpA		[in] ANSI environment.
 * @param p_envpW	[out] Newly-allocated UTF-16 environment.
 * @return 0 on success; non-zero on error.
 * On success, p_envpW will contain a malloc()'d block.
 * Caller must free it when it's done using it.
 */
static int convertEnvpAtoW(const char *envpA[], wchar_t **p_envpW[])
{
	// Temporary variables.
	int ret = -1;
	// Block sizes.
	int cbEnvpWPtr, cchEnvpWStr;
	int cchEnvpWStrLeft;
	// Temporary envpA pointer.
	const char **ptrA;
	// UTF-16 data.
	wchar_t **envpW = NULL;
	wchar_t **envpW_tmp;
	// UTF-16 block pointer.
	wchar_t *strW;

	// Determine the total length of the argv block.
	cchEnvpWStr = 0;
	for (ptrA = envpA; *ptrA != NULL; ptrA++) {
		ret = MultiByteToWideChar(CP_ACP, 0, *ptrA, -1, NULL, 0);
		if (ret <= 0) {
			// WideCharToMultiByte() failed!
			// Stop processing.
			ret = GetLastError();
			goto out;
		}
		cchEnvpWStr += ret;
	}
	cbEnvpWPtr = (int)((ptrA - envpA + 1) * sizeof(wchar_t*));

	// Allocate the envp block.
	// The first portion of the block is envp[];
	// the second portion contains the actual string data.
	envpW = (wchar_t**)malloc(cbEnvpWPtr + (cchEnvpWStr * sizeof(wchar_t)));
	memset(envpW, 0, cbEnvpWPtr + (cchEnvpWStr * sizeof(wchar_t)));
	strW = (wchar_t*)((char*)envpW + cbEnvpWPtr);
	cchEnvpWStrLeft = cchEnvpWStr;

	// Convert the environment from ANSI to UTF-16.
	for (ptrA = envpA, envpW_tmp = envpW;
	     *ptrA != NULL; ptrA++, envpW_tmp++)
	{
		assert(cchEnvpWStrLeft > 0);
		if (cchEnvpWStrLeft <= 0) {
			// Out of space in envpU...
			ret = ERROR_INSUFFICIENT_BUFFER;
			goto out;
		}

		ret = MultiByteToWideChar(CP_ACP, 0, *ptrA, -1, strW, cchEnvpWStrLeft);
		if (ret <= 0) {
			// WideCharToMultiByte() failed.
			// Stop processing.
			ret = GetLastError();
			goto out;
		}

		*envpW_tmp = strW;
		strW += ret;
		cchEnvpWStrLeft -= ret;
	}

	// Set the last entry in envpW to NULL.
	*envpW_tmp = NULL;
	ret = 0;

out:
	if (ret != 0) {
		// An error occurred.
		free(envpW);
		return ret;
	}

	*p_envpW = envpW;
	return 0;
}

/**
 * Convert ANSI argv/envp to UTF-16.
 * Caller must free the allocated argvW and envpW.
 * @param p_argcW	[out] Pointer to new argc.
 * @param p_argvW	[out] Pointer to new argv.
 * @param p_envpW	[out, opt] Pointer to new envp.
 * @return 0 on success; non-zero on error.
 */
static int getArgvAtoW(int *p_argcW, wchar_t **p_argvW[], wchar_t **p_envpW[])
{
	_startupinfo StartInfo;
	// __getmainargs() returns.
	int argcA = 0;
	char **argvA = NULL;
	char **envpA = NULL;
	// Temporary variables.
	int ret;
	// UTF-16 data.
	int argcW = 0;
	wchar_t **argvW = NULL;
	wchar_t **envpW = NULL;

	// TODO: Get existing newmode from MSVCRT.
	StartInfo.newmode = 0;

	// NOTE: __getmainargs() is in MSVC 2010+.
	// MinGW-w64 should support it as well.
	// (It's present in 2.0.8 and possibly earlier.)
	ret = __getmainargs(&argcA, &argvA, &envpA, 0, &StartInfo);
	if (ret != 0) {
		// ERROR!
		// TODO: What values?
		return ret;
	}

	// NOTE: Empty strings should still take up 1 character,
	// since they're NULL-terminated.

	// Convert argvA from ANSI to UTF-16.
	ret = convertArgsAtoW(argcA, (const char**)argvA, &argcW, &argvW);
	if (ret != 0) {
		// An error occurred.
		goto out;
	}

	// Convert envpA from ANSI to UTF-16.
	ret = convertEnvpAtoW((const char**)envpA, &envpW);
	if (ret != 0) {
		// An error occurred.
		goto out;
	}

out:
	if (ret != 0) {
		// An error occurred.
		free(argvW);
		free(envpW);
		return ret;
	}

	// TODO: Process envp.
	*p_argcW = argcW;
	*p_argvW = argvW;
	if (p_envpW) {
		*p_envpW = envpW;
	}
	return 0;
}
#endif /* ENABLE_ANSI_WINDOWS */

/** UTF-16 to UTF-8 **/

/**
 * Convert UTF-16 arguments to UTF-8.
 * @param argcW		[in] Number of UTF-16 arguments.
 * @param argvW		[in] UTF-16 arguments.
 * @param p_argcU	[out] Number of UTF-8 arguments.
 * @param p_argvU	[out] Newly-allocated UTF-8 arguments.
 * @return 0 on success; non-zero on error.
 * On success, p_argvU will contain a malloc()'d block.
 * Caller must free it when it's done using it.
 */
static int convertArgsWtoU(int argcW, const wchar_t *argvW[], int *p_argcU, char **p_argvU[])
{
	// Temporary variables.
	int ret = -1;
	int i;
	// Block sizes.
	int cbArgvUPtr, cbArgvUStr;
	int cbArgvUStrLeft;
	// UTF-8 data.
	int argcU = 0;
	char **argvU = NULL;
	// UTF-8 block pointer.
	char *strU;

	// Determine the total length of the argv block.
	cbArgvUPtr = (int)((argcW + 1) * sizeof(char*));
	cbArgvUStr = 0;
	for (i = 0; i < argcW; i++) {
		ret = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, NULL, 0, NULL, NULL);
		if (ret <= 0) {
			// WideCharToMultiByte() failed!
			// Stop processing.
			ret = GetLastError();
			goto out;
		}
		cbArgvUStr += ret;
	}

	// Allocate the argv block.
	// The first portion of the block is argv[];
	// the second portion contains the actual string data.
	argvU = (char**)malloc(cbArgvUPtr + (cbArgvUStr * sizeof(char)));
	strU = (char*)argvU + cbArgvUPtr;
	cbArgvUStrLeft = cbArgvUStr;

	// Convert the arguments from UTF-16 to UTF-8.
	for (i = 0; i < argcW; i++) {
		assert(cbArgvUStrLeft > 0);
		if (cbArgvUStrLeft <= 0) {
			// Out of space in argvU...
			ret = ERROR_INSUFFICIENT_BUFFER;
			goto out;
		}

		ret = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, strU, cbArgvUStrLeft, NULL, NULL);
		if (ret <= 0) {
			// WideCharToMultiByte() failed.
			// Stop processing.
			ret = GetLastError();
			goto out;
		}

		argvU[i] = strU;
		strU += ret;
		cbArgvUStrLeft -= ret;
	}

	// Set the last entry in argvU to NULL.
	argcU = argcW;
	argvU[argcU] = NULL;
	ret = 0;

out:
	if (ret != 0) {
		// An error occurred.
		free(argvU);
		return ret;
	}

	*p_argcU = argcU;
	*p_argvU = argvU;
	return 0;
}

/**
 * Convert UTF-16 environment to UTF-8.
 * @param envpW		[in] UTF-16 environment.
 * @param p_envpU	[out] Newly-allocated UTF-8 environment.
 * @return 0 on success; non-zero on error.
 * On success, p_envpU will contain a malloc()'d block.
 * Caller must free it when it's done using it.
 */
static int convertEnvpWtoU(const wchar_t *envpW[], char **p_envpU[])
{
	// Temporary variables.
	int ret = -1;
	// Block sizes.
	int cbEnvpUPtr, cbEnvpUStr;
	int cbEnvpUStrLeft;
	// Temporary envpW pointer.
	const wchar_t **ptrW;
	// UTF-8 data.
	char **envpU = NULL;
	char **envpU_tmp;
	// UTF-8 block pointer.
	char *strU;

	// Determine the total length of the argv block.
	cbEnvpUStr = 0;
	for (ptrW = envpW; *ptrW != NULL; ptrW++) {
		ret = WideCharToMultiByte(CP_UTF8, 0, *ptrW, -1, NULL, 0, NULL, NULL);
		if (ret <= 0) {
			// WideCharToMultiByte() failed!
			// Stop processing.
			ret = GetLastError();
			goto out;
		}
		cbEnvpUStr += ret;
	}
	cbEnvpUPtr = (int)((ptrW - envpW + 1) * sizeof(char*));

	// Allocate the envp block.
	// The first portion of the block is envp[];
	// the second portion contains the actual string data.
	envpU = (char**)malloc(cbEnvpUPtr + (cbEnvpUStr * sizeof(char)));
	strU = (char*)envpU + cbEnvpUPtr;
	cbEnvpUStrLeft = cbEnvpUStr;

	// Convert the arguments from UTF-16 to UTF-8.
	for (ptrW = envpW, envpU_tmp = envpU;
	     *ptrW != NULL; ptrW++, envpU_tmp++)
	{
		assert(cbEnvpUStrLeft > 0);
		if (cbEnvpUStrLeft <= 0) {
			// Out of space in envpU...
			ret = ERROR_INSUFFICIENT_BUFFER;
			goto out;
		}

		ret = WideCharToMultiByte(CP_UTF8, 0, *ptrW, -1, strU, cbEnvpUStrLeft, NULL, NULL);
		if (ret <= 0) {
			// WideCharToMultiByte() failed.
			// Stop processing.
			ret = GetLastError();
			goto out;
		}

		*envpU_tmp = strU;
		strU += ret;
		cbEnvpUStrLeft -= ret;
	}

	// Set the last entry in envpU to NULL.
	*envpU_tmp = NULL;
	ret = 0;

out:
	if (ret != 0) {
		// An error occurred.
		free(envpU);
		return ret;
	}

	*p_envpU = envpU;
	return 0;
}

/** External functions **/

typedef enum _AllocType {
	ALLOC_NONE	= 0,
	ALLOC_MALLOC,
	ALLOC_LOCALALLOC,
} AllocType;

static inline void special_free(void *buf, AllocType allocType)
{
	switch (allocType) {
		case ALLOC_NONE:
		default:
			// No allocation.
			break;
		case ALLOC_MALLOC:
			// malloc()'d buffer - use free().
			free(buf);
			break;
		case ALLOC_LOCALALLOC:
			// LocalAlloc()'d buffer - use LocalFree().
			LocalFree(buf);
			break;
	}
}

#ifdef USE_COMMANDLINETOARGVW
/**
 * Convert a Windows environment strings block to envpW.
 * The allocated envpW contains pointers to the original
 * environment strings, so it should not be freed or modified
 * until envpW is freed.
 * @param envStringsW Windows environment strings. (Use GetEnvironmentStringsW().)
 * @return Allocated envpW(). (malloc()'d; use free() when finished.)
 */
static wchar_t **EnvironmentStringsToEnvpW(wchar_t *envStringsW)
{
	// NOTE: envStringsW and p_cur are NOT const.
	// envpW isn't const, so in order to avoid lots of casting,
	// we'll just use non-const variables.
	wchar_t *p_cur;
	wchar_t **envpW;
	int envpW_cnt = 0;
	int i;

	// Convert envStringsW to envpW.
	// - envStringsW = multiple null-terminated strings in a single buffer.
	// - envpW = array of pointers pointing to the start of each string.

	// Determine how many strings are in the environment.
	// TODO: Optimize this?
	for (p_cur = envStringsW; ; p_cur++) {
		if (*p_cur == 0) {
			// Found a NULL byte.
			envpW_cnt++;
			if (*(p_cur+1) == 0) {
				// End of environment block.
				break;
			}
		}
	}

	// Allocate envpW.
	envpW = (wchar_t**)malloc((envpW_cnt+1) * sizeof(envpW));
	if (!envpW)
		return NULL;

	// Store pointers to the actual environment data in envpW.
	p_cur = envStringsW;
	for (i = 0; i < envpW_cnt; i++) {
		envpW[i] = p_cur;
		// Find the next string.
		for (; ; p_cur++) {
			if (*p_cur == 0) {
				// Found a NULL byte.
				p_cur++;
				break;
			}
		}
	}

	// Final entry is NULL.
	envpW[envpW_cnt] = NULL;
	return envpW;
}

#endif /* USE_COMMANDLINETOARGVW */

/**
 * Convert the Windows command line to UTF-8.
 * @param p_argc	[out] Pointer to new argc.
 * @param p_argv	[out] Pointer to new argv.
 * @param p_envp	[out, opt] Pointer to new envp.
 * @return 0 on success; non-zero on error.
 */
int W32U_GetArgvU(int *p_argc, char **p_argv[], char **p_envp[])
{
	// __wgetmainargs() returns.
	int argcW = 0;
	wchar_t **argvW = NULL;
	wchar_t **envpW = NULL;
	AllocType argvW_allocType = ALLOC_NONE;
	AllocType envpW_allocType = ALLOC_NONE;
	// Temporary variables.
	int ret = -1;
	// UTF-8 data.
	int argcU = 0;
	char **argvU = NULL;
	char **envpU = NULL;
	// Is the system Unicode?
	int isUnicode;

	// Check if argv has already been converted.
	if (saved_argcU > 0 || saved_argvU || saved_envpU) {
		// argv has already been converted.
		*p_argc = saved_argcU;
		*p_argv = saved_argvU;
		if (p_envp) {
			*p_envp = saved_envpU;
		}
		return 0;
	}

	isUnicode = W32U_IsUnicode();
	if (!isUnicode) {
#ifdef ENABLE_ANSI_WINDOWS
		// ANSI. Use __getmainargs().
		argvW_allocType = ALLOC_MALLOC;
		envpW_allocType = ALLOC_MALLOC;
		ret = getArgvAtoW(&argcW, &argvW, &envpW);
		if (ret != 0) {
			// ERROR!
			// TODO: What values?
			goto out;
		}
#else /* !ENABLE_ANSI_WINDOWS */
		// ANSI is not supported in this build.
		return ERROR_CALL_NOT_IMPLEMENTED;
#endif /* ENABLE_ANSI_WINDOWS */
	} else {
		// Unicode.
#ifdef USE_COMMANDLINETOARGVW
		wchar_t *envStringsW;
		// Use CommandLineToArgvW().
		argvW = CommandLineToArgvW(GetCommandLineW(), &argcW);
		// TODO: Verify argcW.
		if (!argvW) {
			// ERROR!
			// TODO: What values?
			goto out;
		}
		argvW_allocType = ALLOC_LOCALALLOC;

		// NOTE: GetEnvironmentStringsW() is XP+.
		// MSVC 2015 only targets XP+, so GetProcAddress()
		// isn't necessary here.
		envStringsW = GetEnvironmentStringsW();
		if (!envStringsW) {
			// ERROR!
			// TODO: What values?
			goto out;
		}

		// Convert envStringsW to envpW.
		envpW = EnvironmentStringsToEnvpW(envStringsW);
		if (!envpW) {
			// ERROR!
			// TODO: What values?
			goto out;
		}
		envpW_allocType = ALLOC_MALLOC;
#else
		// Use __wgetmainargs().
		// TODO: Get existing newmode from MSVCRT.
		_startupinfo StartInfo;
		StartInfo.newmode = 0;
		argvW_allocType = ALLOC_NONE;
		envpW_allocType = ALLOC_NONE;

		// NOTE: __wgetmainargs() is in MSVC 2010+.
		// MinGW-w64 should support it as well.
		// (It's present in 2.0.8 and possibly earlier.)
		ret = __wgetmainargs(&argcW, &argvW, &envpW, 0, &StartInfo);
		if (ret != 0) {
			// ERROR!
			// TODO: What values?
			goto out;
		}
#endif
	}

	// NOTE: Empty strings should still take up 1 character,
	// since they're NULL-terminated.

	// Convert argvW from UTF-16 to UTF-8.
	ret = convertArgsWtoU(argcW, (const wchar_t**) argvW, &argcU, &argvU);
	if (ret != 0) {
		// An error occurred.
		goto out;
	}

	// Convert envpW from UTF-16 to UTF-8.
	if (envpW) {
		ret = convertEnvpWtoU((const wchar_t**)envpW, &envpU);
		if (ret != 0) {
			// An error occurred.
			goto out;
		}
	}

out:
	// Free the Unicode variables depending on
	// how they were allocated.
	special_free(argvW, argvW_allocType);
	special_free(envpW, envpW_allocType);
	
	if (ret != 0) {
		// An error occurred.
		free(argvU);
		free(envpU);
		return ret;
	}

	// Save arguments and environment.
	saved_argcU = argcU;
	saved_argvU = argvU;
	saved_envpU = envpU;

	*p_argc = argcU;
	*p_argv = argvU;
	if (p_envp) {
		*p_envp = envpU;
	}
	return 0;
}
