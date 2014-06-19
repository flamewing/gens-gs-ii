# Microsoft Visual C++

# Remove some default CFLAGS/CXXFLAGS.
STRING(REPLACE "/GR" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
STRING(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
STRING(REPLACE "/Zi" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
STRING(REPLACE "/Zi" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Disable C++ RTTI.
SET(GENS_CXX_NO_RTTI_CXXFLAG "-GR-")

# Disable asynchronous exceptions.
# (MSVC doesn't allow disabling C++ exceptions entirely.)
SET(GENS_CXX_NO_EXCEPTIONS_CXXFLAG "-EHsc")

# Disable useless warnings:
# - MSVC "logo" messages
# - C4355: 'this' used in base member initializer list (used for Qt Dpointer pattern)
# - MSVCRT "deprecated" functions
SET(GENS_CFLAGS_COMMON "-nologo -wd4355 -D_CRT_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE")
SET(GENS_CXXFLAGS_COMMON "${GENS_CFLAGS_COMMON}")
SET(GENS_LDFLAGS_COMMON "-nologo /manifest:no")

# Disable the RC and MASM "logo".
SET(CMAKE_RC_FLAGS "-nologo")
SET(CMAKE_ASM_MASM_FLAGS "-nologo")

# Check for link-time optimization.
IF(ENABLE_LTO)
	SET(GENS_CFLAGS_LTO "-GL")
	SET(GENS_LDFLAGS_LTO "-LTCG")
ENDIF(ENABLE_LTO)

# Debug/release flags.
# NOTE: REMOVE ITERATOR DEBUGGING LATER
SET(GENS_CFLAGS_COMPILER_DEBUG "-Od -Z7 -MDd -RTC1")
SET(GENS_CXXFLAGS_COMPILER_DEBUG "-Od -Z7 -MDd -RTC1")
SET(GENS_CFLAGS_COMPILER_RELEASE "-Ox -Z7 -MD")
SET(GENS_CXXFLAGS_COMPILER_RELEASE "-Ox -Z7 -MD")
SET(GENS_LDFLAGS_COMPILER_DEBUG "-DEBUG")
SET(GENS_LDFLAGS_COMPILER_RELEASE "-DEBUG")
