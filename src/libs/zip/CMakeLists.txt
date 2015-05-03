set(MINIZIP_SRC
ioapi.c
unzip.c
zip.c)

# -------------------------------------------------------
# Include directory
# -------------------------------------------------------
# include_directories(include include/internal)

# -------------------------------------------------------
# module
# -------------------------------------------------------
add_library (minizip_module ${MINIZIP_SRC})
