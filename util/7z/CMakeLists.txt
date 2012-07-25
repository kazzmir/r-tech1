# -------------------------------------------------------
# util cmake build script for paintown.
# Written by: juvinious
# Modified by: kazzmir
# -------------------------------------------------------

# -------------------------------------------------------
# Source directories containing all the necessary .cpp files
# -------------------------------------------------------
set(x7Z_SRC
7zAlloc.c
7zBuf.c
7zBuf2.c
7zCrc.c
7zCrcOpt.c
7zDec.c
7zIn.c
CpuArch.c
LzmaDec.c
Lzma2Dec.c
Bra.c
Bra86.c
Bcj2.c
7zFile.c
7zStream.c)

# -------------------------------------------------------
# Include directory
# -------------------------------------------------------
#include_directories(include include/internal)

# -------------------------------------------------------
# module
# -------------------------------------------------------
add_library (x7z_module ${x7Z_SRC})
