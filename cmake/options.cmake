# Build options.

# LZMA SDK.
OPTION(USE_LZMA "Use the LZMA SDK to decompress 7-Zip archives." 1)

# ZLIB.
# TODO: Make ZLIB optional? (This would disable savestates...)
IF(NOT WIN32)
	OPTION(USE_INTERNAL_ZLIB "Always use the internal copy of ZLIB." 0)
ELSE(NOT WIN32)
	# TODO: Allow use of external zlib on Win32?
	SET(USE_INTERNAL_ZLIB 1)
ENDIF(NOT WIN32)

# Internal libpng.
# NOTE: Also controls internal zlib usage.
# TODO: Separate it so int libpng / ext zlib can be used?
IF(NOT WIN32)
	OPTION(USE_INTERNAL_PNG "Always use the internal copy of libpng." 0)
ELSE(NOT WIN32)
	# TODO: Allow use of external libpng on Win32?
	SET(USE_INTERNAL_PNG 1)
ENDIF(NOT WIN32)

# OpenGL.
# TODO: Make OpenGL optional once other backends have been added.
OPTION(USE_GLEW "Use GLEW for OpenGL extension support." 1)
IF(NOT WIN32)
	OPTION(USE_INTERNAL_GLEW "Always use the internal copy of GLEW." 0)
ELSE(NOT WIN32)
	# Always use the internal GLEW on Win32 regardless.
	# TODO: Allow use of external GLEW on Win32?
	SET(USE_INTERNAL_GLEW 1)
ENDIF(NOT WIN32)

# Internal popt.
# Default is to use external popt if available.
OPTION(USE_INTERNAL_POPT "Always use the internal copy of popt." 0)

# Frontends.
OPTION(ENABLE_GENS_QT4 "Enable the Qt4 UI. (EXPERIMENTAL; has frame dropping issues)" 0)
OPTION(ENABLE_GENS_SDL "Enable the SDL2 UI. (Technical Preview)" 1)

# Additional stuff.
OPTION(BUILD_DOC "Build documentation." 1)

# ANSI Windows support. (32-bit Windows only.)
IF(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
	OPTION(ENABLE_ANSI_WINDOWS "Enable support for ANSI Windows." 1)
ELSE()
	# Either 64-bit Windows or not Windows at all.
	# Don't enable ANSI support.
	SET(ENABLE_ANSI_WINDOWS 0)
ENDIF()

# Link-time optimization.
OPTION(ENABLE_LTO "Enable link-time optimization. (Release builds only)" 0)

# Split debug information into a separate file.
OPTION(SPLIT_DEBUG "Split debug information into a separate file." 1)

# Install the split debug file.
OPTION(INSTALL_DEBUG "Install the split debug file." 1)

# Compress the executable with UPX.
OPTION(COMPRESS_EXE "Compress the executable with UPX." 0)
