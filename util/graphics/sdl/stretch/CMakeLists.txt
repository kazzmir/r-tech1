# -------------------------------------------------------
# SDL Image cmake build script for paintown.
# Written by: juvinious
# -------------------------------------------------------

# -------------------------------------------------------
# MSVC FLAGS
# -------------------------------------------------------
if(NOT MSVC)
	set(CFLAGS "-W -O2 -Wwrite-strings -Wstrict-prototypes -Wall -funroll-loops -Wmissing-declarations -fomit-frame-pointer -Wno-missing-declarations")
else(NOT MSVC)
	set(CFLAGS "-O2")
endif(NOT MSVC)

remove_definitions(${CXXFLAGS})
add_definitions(${CFLAGS} -DSDL_STRETCH_DISABLE_ASM)

# -------------------------------------------------------
# Source directories containing all the necessary .cpp files
# -------------------------------------------------------
set(SRC
sdlstretch.c
sdlstretchcode.c)

# -------------------------------------------------------
# Include directory
# -------------------------------------------------------
include_directories(include ./)

# -------------------------------------------------------
# module
# -------------------------------------------------------
add_library (sdl_stretch_module ${SRC})

