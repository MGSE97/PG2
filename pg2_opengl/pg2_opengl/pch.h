#ifndef PCH_H
#define PCH_H

#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

// Print errors to console
#define VERBOSE false

// std libs
#include <stdio.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <random>
#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <functional>
#include <fstream>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

// Glad - multi-Language GL/GLES/EGL/GLX/WGL loader-generator based on the official specs
#include <glad/glad.h>

// GLFW - simple API for creating windows, receiving input and events
#include <GLFW/glfw3.h>

// Free Image
#include <FreeImage.h>

#endif //PCH_H
