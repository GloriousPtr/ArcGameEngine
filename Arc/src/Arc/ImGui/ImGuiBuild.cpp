#include "arcpch.h"

#if defined(__clang__) || defined(__llvm__)
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Weverything"
#elif defined(_MSC_VER)
#	pragma warning(push, 0)
#endif

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <backends/imgui_impl_opengl3.cpp>
#include <backends/imgui_impl_glfw.cpp>

#if defined(__clang__) || defined(__llvm__)
#	pragma clang diagnostic pop
#elif defined(_MSC_VER)
#	pragma warning(pop)
#endif
