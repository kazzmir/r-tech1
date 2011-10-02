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
add_definitions(${CFLAGS} -DLOAD_PNG -DLOAD_PCX -DLOAD_GIF)

# -------------------------------------------------------
# Source directories containing all the necessary .cpp files
# -------------------------------------------------------
set(SRC
IMG.c 
IMG_jpg.c 
IMG_pnm.c 
IMG_xpm.c 
IMG_ImageIO.c 
IMG_lbm.c 
IMG_tga.c 
IMG_xv.c 
IMG_bmp.c 
IMG_pcx.c 
IMG_tif.c 
IMG_xxx.c 
IMG_gif.c 
IMG_png.c 
IMG_savepng.c
IMG_xcf.c)

# -------------------------------------------------------
# Include directory
# -------------------------------------------------------
include_directories(include ./)

# -------------------------------------------------------
# module
# -------------------------------------------------------
add_library (image_module ${SRC})

