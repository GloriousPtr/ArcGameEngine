#include "arcpch.h"

#ifdef ARC_PLATFORM_LINUX
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <backends/imgui_impl_opengl3.cpp>
#include <backends/imgui_impl_glfw.cpp>
#endif
#ifdef ARC_PLATFORM_WINDOWS
#include <backends/imgui_impl_win32.cpp>
#include <backends/imgui_impl_dx12.cpp>
#endif
