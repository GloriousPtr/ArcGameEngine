#!/bin/bash
set -e

chmod +x vendor/premake/bin/premake5

# Patch GLFW x11_init.c: insert missing setWindowTitleBar function pointer (NULL stub)
# The upstream GLFW fork is missing this entry in the X11 platform struct, causing
# every function pointer after it to be assigned to the wrong slot.
GLFW_X11_INIT="Arc/vendor/GLFW/src/x11_init.c"
if [ -f "$GLFW_X11_INIT" ] && ! grep -q "setWindowTitleBar" "$GLFW_X11_INIT"; then
    sed -i '/_glfwPostEmptyEventX11,/a\        NULL, // setWindowTitleBar - stub for X11 (not implemented)' "$GLFW_X11_INIT"
    echo "Patched $GLFW_X11_INIT: added missing setWindowTitleBar NULL stub"
fi

./vendor/premake/bin/premake5 gmake2 --cc=clang --dotnet=mono