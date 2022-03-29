#ifndef GL_GLCONFIG_H
#define GL_GLCONFIG_H

// Ok, we use glew which includes itself GL.h
#include <GL/glew.h>

// Comment out to wrap all functions in GL:: namespace
#define GL_NAMESPACE

// Default output stream for error messages
#define GLUTILS_CERR std::cerr
// Default output stream for log messages
#define GLUTILS_COUT std::cout

#endif

