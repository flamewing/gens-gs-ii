# Check for OpenGL and GLEW.
FIND_PACKAGE(OpenGL REQUIRED)
IF(OPENGL_LIBRARIES)
	# Found OpenGL.
	SET(HAVE_OPENGL 1)
	MESSAGE(STATUS "Found OpenGL: ${OPENGL_gl_LIBRARY}")
	IF(WIN32)
		# Always use internal GLEW on Win32.
		SET(HAVE_GLEW 1)
		SET(GLEW_LIBRARY GLEW)
		SET(USE_INTERNAL_GLEW 1)
		SET(GLEW_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extlib/glew/include/")
		MESSAGE(STATUS "Win32: using internal GLEW")
	ELSE(WIN32)
		# Find GLEW.
		SET(OLD_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
		SET(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${OPENGL_gl_LIBRARY})
		SET(CHECK_GLEW_LIBRARY "GLEW")
		CHECK_LIBRARY_EXISTS(${CHECK_GLEW_LIBRARY} glewInit "" HAVE_GLEW)
		IF(HAVE_GLEW)
			SET(GLEW_FOUND 1)
			SET(GLEW_LIBRARY "-l${CHECK_GLEW_LIBRARY}")
		ELSE(HAVE_GLEW)
			SET(HAVE_GLEW 1)
			SET(GLEW_LIBRARY glew)
			SET(GLEW_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extlib/glew/include/")
			SET(USE_INTERNAL_GLEW 1)
			MESSAGE(STATUS "GLEW library not found; using internal GLEW.")
		ENDIF(HAVE_GLEW)
		UNSET(CHECK_GLEW_LIBRARY)
		SET(CMAKE_REQUIRED_LIBRARIES ${OLD_CMAKE_REQUIRED_LIBRARIES})
		UNSET(OLD_CMAKE_REQUIRED_LIBRARIES)
	ENDIF(WIN32)
ENDIF(OPENGL_LIBRARIES)