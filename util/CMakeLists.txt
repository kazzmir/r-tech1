# -------------------------------------------------------
# util cmake build script for paintown.
# Written by: juvinious
# Modified by: kazzmir
# -------------------------------------------------------

# -------------------------------------------------------
# Source directories containing all the necessary .cpp files
# -------------------------------------------------------
set(UTIL_SRC
argument.cpp
network/network.cpp
network/chat.cpp
network/irc.cpp
token.cpp
resource.cpp
tokenreader.cpp
timedifference.cpp
debug.cpp
timer.cpp
init.cpp
utf.cpp
console.cpp
input/keyboard.cpp
loading.cpp
messages.cpp
graphics/bitmap.cpp
graphics/image.cpp
events.cpp
font.cpp
fire.cpp
ftalleg.cpp
funcs.cpp
directory.cpp
file-system.cpp
graphics/gradient.cpp
ebox.cpp
regex.cpp
language-string.cpp
thread.cpp
input/input.cpp
input/text-input.cpp
input/input-manager.cpp
input/input-source.cpp
input/joystick.cpp
input/allegro5/joystick.cpp
input/allegro/allegro-joystick.cpp
input/sdl/joystick.cpp
input/psp/joystick.cpp
input/wii/joystick.cpp
graphics/sdl/hqx.cpp
graphics/sdl/xbr.cpp
system.cpp
compress.cpp
message-queue.cpp
input/linux_joystick.cpp
load_exception.cpp
windows/funcs.cpp
windows/system.cpp
nacl/module.cpp
nacl/network-system.cpp
exceptions/exception.cpp
menu/actionfactory.cpp
menu/action_speed.cpp
menu/menu.cpp
menu/font-info.cpp
menu/menu_action.cpp
menu/menu_option.cpp
menu/options.cpp
menu/optionfactory.cpp
gui/animation.cpp
gui/box.cpp
gui/container.cpp
gui/context-box.cpp
gui/coordinate.cpp
gui/cutscene.cpp
gui/fadetool.cpp
gui/lineedit.cpp
gui/rectarea.cpp
gui/popup-box.cpp
gui/scroll-list.cpp
gui/select-list.cpp
gui/tabbed-box.cpp
gui/timer.cpp
gui/widget.cpp
sound/sound.cpp
sound/audio.cpp
sound/music-player.cpp
sound/music-renderer.cpp
sound/music.cpp)

# -------------------------------------------------------
# Include directory
# -------------------------------------------------------
#include_directories(include include/internal)

# -------------------------------------------------------
# module
# -------------------------------------------------------
add_library (util_module ${UTIL_SRC})
