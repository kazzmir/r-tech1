# -------------------------------------------------------
# r-tech1 cmake build script. Creates makefiles for unix
# based systems, cygwin(sp?) or mingw
#
# Written by: juvinious
# -------------------------------------------------------

# -------------------------------------------------------
# Ensure that we are doing an out of source build
# Prevents any mishaps
# Also remove the makefile to regenerate the filelist
# -------------------------------------------------------
cmake_minimum_required(VERSION 2.6)
if(EXISTS ${CMAKE_SOURCE_DIR}/CMakeCache.txt)
	file(REMOVE ${CMAKE_SOURCE_DIR}/CMakeCache.txt)
	file(REMOVE_RECURSE ${CMAKE_SOURCE_DIR}/CMakeFiles)
endif(EXISTS ${CMAKE_SOURCE_DIR}/CMakeCache.txt)
if(EXISTS ${CMAKE_BINARY_DIR}/Makefile)
	file(REMOVE ${CMAKE_BINARY_DIR}/Makefile)
endif(EXISTS ${CMAKE_BINARY_DIR}/Makefile)
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	if(UNIX AND NOT CYGWIN)
		message(FATAL_ERROR "Oops. Don't do an in-source build. Create an extra directory change into it and run cmake pointing to the base directory. IE: \nmkdir mybuild && cd mybuild && cmake ../ && make\nYou may need to remove CMakeCache.txt and the CMakeFiles directory in ${CMAKE_SOURCE_DIR} if you can't get rid of this error.")
	else(UNIX AND NOT CYGWIN)
		message(FATAL_ERROR "Oops. Don't do an in-source build. Create an extra directory change into it and run cmake pointing to the base directory. IE: \nmkdir mybuild; cd mybuild; cmakesetup ../\nYou may need to remove CMakeCache.txt and the CMakeFiles directory in  ${CMAKE_SOURCE_DIR} if you can't get rid of this error.")
	endif(UNIX AND NOT CYGWIN)
endif(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

# -------------------------------------------------------
# Directory in which extra macros can be found
# -------------------------------------------------------
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# -------------------------------------------------------
# Directory in which extra macros can be found
# -------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# -------------------------------------------------------
# Add in uninstall target
# -------------------------------------------------------
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# -------------------------------------------------------
# project name
# -------------------------------------------------------
project (r-tech1 C CXX)

# -------------------------------------------------------
# Including needed macros
# -------------------------------------------------------
include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
find_package(ZLIB)
find_package(PNG)
find_package(FREETYPE)
find_package(Threads)
#find_package(GNUREGEX)
find_package(VORBIS)
find_package(OGG)
find_package(MPG123)

# -------------------------------------------------------
# Include Source
# -------------------------------------------------------
mark_as_advanced(CLEAR ZLIB_INCLUDE_DIR)
mark_as_advanced(CLEAR ZLIB_LIBRARY)

mark_as_advanced(CLEAR PNG_PNG_INCLUDE_DIR)
mark_as_advanced(CLEAR PNG_LIBRARY)

# -------------------------------------------------------
# User setable options
# -------------------------------------------------------
option(DEBUG "Compile r-tech1 with debug information?" OFF)

if($ENV{DEBUG})
    set(DEBUG ON)
elseif(NOT $ENV{DEBUG})
    set(DEBUG OFF)
endif($ENV{DEBUG})

if(DEBUG)
    add_definitions(-DDEBUG)
    set(CMAKE_CXX_FLAGS "-Wall -Wno-unused-variable -Wno-unused-function -funroll-loops -fexpensive-optimizations -Woverloaded-virtual -g3 -ggdb")
    set(CMAKE_C_FLAGS "-Wall -Wno-unused-variable -Wno-unused-function -funroll-loops -fexpensive-optimizations -g3 -ggdb")
else(DEBUG)
    set(CMAKE_CXX_FLAGS "-O2 -Wall -Wno-unused-variable -Wno-unused-function -funroll-loops -fexpensive-optimizations -Woverloaded-virtual")
    set(CMAKE_C_FLAGS "-O2 -Wall -Wno-unused-variable -Wno-unused-function -funroll-loops -fexpensive-optimizations")
endif(DEBUG)
    
# FIXME: not sure if this is in the right place
add_definitions(-DHAVE_NETWORKING)

option(USE_ALLEGRO5 "Compile r-tech1 with ALLEGRO 5 instead of SDL or Allegro 4.x?" ON)

if($ENV{ALLEGRO5})
    set(USE_ALLEGRO5 ON)
elseif(NOT $ENV{ALLEGRO5})
    set(USE_ALLEGRO5 OFF)
endif($ENV{ALLEGRO5})

if(OGG_FOUND)
    add_definitions(-DHAVE_OGG)
    set(OGG_LIBS ${OGG_LIBRARY} ${VORBIS_LIBRARIES})
    set(OGG_INCLUDES ${OGG_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR})
else(OGG_FOUND)
    set(OGG_LIBS "")
    set(OGG_INCLUDES "")
endif(OGG_FOUND)

# Allegro5
find_package(ALLEGRO5)
if(NOT ALLEGRO_FOUND)
message(FATAL_ERROR "Couldn't find Allegro 5 and/or a required Allegro 5 module")
endif(NOT ALLEGRO_FOUND)
set(GRAPHICS_LIBRARIES ${ALLEGRO_LIBRARIES})
set(GRAPHICS_INCLUDES ${ALLEGRO_INCLUDE_DIR})

if(MPG123_FOUND)
    add_definitions(-DHAVE_MP3_MPG123)
    set(MP3_LIBS ${MPG123_LIBRARY})
    set(MP3_INCLUDES ${MPG123_INCLUDE_DIR})
else(MPG123_FOUND)
    if (SDL_FOUND)
	# Try looking for libmad instead only if SDL is available
	find_package(MAD)
	if (MAD_FOUND)
	    add_definitions(-DHAVE_MP3_MAD)
	    set(MP3_LIBS ${MAD_LIBRARY})
	    set(MP3_INCLUDES ${MAD_INCLUDE_DIR})
	else(MAD_FOUND)
	    set(MP3_LIBS "")
	    set(MP3_INCLUDES "")
	endif(MAD_FOUND)
    endif (SDL_FOUND)
endif(MPG123_FOUND)

# freetype
if(NOT FREETYPE_FOUND)
	message(FATAL_ERROR "Couldn't find freetype")
endif(NOT FREETYPE_FOUND)

# -------------------------------------------------------
# Paths and system setup
# -------------------------------------------------------

# -------------------------------------------------------
# MINGW
# -------------------------------------------------------

if(MINGW OR CYGWIN)
	# -------------------------------------------------------
	# Take those user options and set the necessary compile time preprocessors
	# -------------------------------------------------------
	add_definitions(-DWINDOWS)
	set(WIN_LIB -lgdi32 -luser32 -lwsock32)
	 set(CMAKE_THREAD_LIBS_INIT -lpthreadGC2)
else(MINGW OR CYGWIN)
	 set(CMAKE_THREAD_LIBS_INIT -lpthread)
endif(MINGW OR CYGWIN)

# -------------------------------------------------------
# Apple OSX
# -------------------------------------------------------

if(APPLE)
	# -------------------------------------------------------
    # Added stuff for osx
	# -------------------------------------------------------
	set(WIN_LIB "-framework Carbon")
	add_definitions(-DMACOSX)
        
        # Groups
        set(GROUP_START "-Wl,-all_load")
        set(GROUP_END "")
else(APPLE)
        # Groups
        set(GROUP_START "-Wl,--start-group")
        set(GROUP_END "-Wl,--end-group")
endif(APPLE)

# -------------------------------------------------------
# Include directory
# -------------------------------------------------------
include_directories(src src/libs/hawknl src/libs/dumb/include include ${GRAPHICS_INCLUDES} ${FREETYPE_INCLUDE_DIR} ${OGG_INCLUDES} ${MP3_INCLUDES})

# -------------------------------------------------------
# Put the linked libraries together
# -------------------------------------------------------
set(REQUIRED_LIBS ${GRAPHICS_LIBRARIES}  ${CMAKE_THREAD_LIBS_INIT} ${FREETYPE_LIBRARIES} ${PNG_LIBRARY} ${ZLIB_LIBRARY} ${OGG_LIBS} ${MP3_LIBS} ${WIN_LIB} ${WINSOCK})

# -------------------------------------------------------
# Source files
# -------------------------------------------------------

#Base Source
add_subdirectory(${CMAKE_SOURCE_DIR}/src)

# 7z
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/7z)

#Dumb
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/dumb)

#Hawknl
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/hawknl)

#Minizip
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/zip)

# SFL
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/sfl)

# GME Library
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/gme)

# PCRE
add_subdirectory(${CMAKE_SOURCE_DIR}/src/libs/pcre)

# Create r-tech1 library
add_library(r-tech1 ${SFL_SRC} ${UTIL_SRC} ${PCRE_SRC} ${x7Z_SRC} ${DUMB_SRC} ${MINIZIP_SRC} ${HAWKNL_SRC} ${GME_SRC})

# Init test
add_executable(init-test tests/init/main.cpp)
target_link_libraries(init-test r-tech1 ${REQUIRED_LIBS})
