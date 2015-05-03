#include "irc-client.h"

#include <iostream>
#include <vector>
#include <stdexcept>

#include "util/graphics/bitmap.h"
#include "util/font.h"
#include "util/debug.h"
#include "util/exceptions/load_exception.h"
#include "util/token_exception.h"
#include "util/input/input.h"
#include "util/input/input-manager.h"
#include "util/sound/sound.h"
#include "util/exceptions/exception.h"
#include "util/network/network.h"
#include "util/network/chat.h"
#include "util/network/irc.h"
#include "util/thread.h"
#include "util/pointer.h"
#include "util/system.h"

#include "mugen/util.h"
#include "mugen/search.h"
#include "mugen/exception.h"
#include "mugen/options.h"

#include "mugen/widgets.h"

#include <queue>