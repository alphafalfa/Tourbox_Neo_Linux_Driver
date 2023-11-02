/**
 * @file uinput_helper.hpp
 * @author CK <cwkeller+tourbox@pm.me>
 * @brief Helper functions for working with Linux's `uinput` module.
 *	    Based heavily on: https://www.kernel.org/doc/html/latest/input/uinput.html
 * @version 0.1.1
 * @date 2023-20-30
 *
 * @copyright Copyright (c) 2022-2023
 *
 *
 * @file uinput_helper.hpp
 * @author Raleigh Littles <raleighlittles@gmail.com>
 * @brief Helper functions for working with Linux's `uinput` module.
 *	    Based heavily on: https://www.kernel.org/doc/html/latest/input/uinput.html
 * @version 0.1
 * @date 2022-07-24
 *
 * @copyright Copyright (c) 2022
 *
 */


#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/uinput.h>
#include <map>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <confuse.h>

enum class KeyType : int
{
	DBL_TOP = 0x13,
	DBL_RING = 0x18,         /* Some keys report double click */
	DBL_PINKIE = 0x1c,
	DBL_SIDE = 0x21,

	NINTENDO_B = 0x22,       /* Two small circles near Tourbox logo. */
	NINTENDO_A = 0x23,
	MOON= 0x2a,             /* Next to tall knob */
	
	RING = 0x80,
	SIDE = 0x81,             /* Various small buttons */
	TOP = 0x82,
	PINKIE = 0x83,           /* Pair on bottom right */

	SCROLL_DOWN = 0x09,
	SCROLL_PRESS = 0x0a,
	SCROLL_UP = 0x49,        /* Large Mouse wheel */

	DPAD_UP = 0x90,          /* Four arrows. */
	DPAD_DOWN = 0x91,
	DPAD_LEFT = 0x92,
	DPAD_RIGHT = 0x93,

	IPOD_PRESS = 0x38,
	IPOD_COUNTERCLOCKWISE = 0x4f,
	IPOD_CLOCKWISE = 0x8f,   /* Large flat disc */

	TALL_PRESS = 0x37,
	TALL_CLOCKWISE = 0x44,   /* Central knob */
	TALL_COUNTERCLOCKWISE = 0x84
};

/* Default buttons for the device: */
std::map<KeyType, int> keyMap = {  /* Grouped primarily for readibility */
  std::make_pair(KeyType::NINTENDO_B, BTN_LEFT),    /* These two are      */
	std::make_pair(KeyType::NINTENDO_A, BTN_RIGHT), /* Mouse buttons.     */
	
	std::make_pair(KeyType::SIDE, KEY_CALC),	      /* Everything else is */
	std::make_pair(KeyType::TOP, KEY_REFRESH),	    /* a Keyboard key.    */
	std::make_pair(KeyType::PINKIE, KEY_FORWARD),	  /* (Including special */ 
	std::make_pair(KeyType::RING, KEY_BACK),	      /* media keys).       */
	std::make_pair(KeyType::MOON, KEY_MUTE),

	
	std::make_pair(KeyType::SCROLL_UP, KEY_SCROLLUP),
	std::make_pair(KeyType::SCROLL_DOWN, KEY_SCROLLDOWN),
	std::make_pair(KeyType::SCROLL_PRESS, KEY_HOME),
	
	std::make_pair(KeyType::DPAD_UP, KEY_SCROLLUP),
	std::make_pair(KeyType::DPAD_DOWN, KEY_SCROLLDOWN),
	std::make_pair(KeyType::DPAD_LEFT, KEY_LEFT),
	std::make_pair(KeyType::DPAD_RIGHT, KEY_RIGHT),


	std::make_pair(KeyType::IPOD_CLOCKWISE, KEY_BRIGHTNESSUP),
	std::make_pair(KeyType::IPOD_COUNTERCLOCKWISE, KEY_BRIGHTNESSDOWN),
	std::make_pair(KeyType::IPOD_PRESS, KEY_MICMUTE),
	
	std::make_pair(KeyType::TALL_CLOCKWISE, KEY_VOLUMEUP),
	std::make_pair(KeyType::TALL_COUNTERCLOCKWISE, KEY_VOLUMEDOWN),
	std::make_pair(KeyType::TALL_PRESS, KEY_PLAYPAUSE),
	
	std::make_pair(KeyType::DBL_RING, KEY_CAMERA),
	std::make_pair(KeyType::DBL_PINKIE, KEY_ALL_APPLICATIONS),
  std::make_pair(KeyType::DBL_SIDE, KEY_SLEEP),
	std::make_pair(KeyType::DBL_TOP, KEY_SCREENLOCK)
};

std::map<KeyType, std::string>  keyString = { 
	std::make_pair(KeyType::NINTENDO_B, "BTN_LEFT"),
	std::make_pair(KeyType::NINTENDO_A, "BTN_RIGHT"),
	
	std::make_pair(KeyType::SIDE, "KEY_BACK"),
	std::make_pair(KeyType::TOP, "KEY_REFRESH"),
	std::make_pair(KeyType::PINKIE, "KEY_FORWARD"),
	std::make_pair(KeyType::RING, "KEY_BACK"),
	std::make_pair(KeyType::MOON, "KEY_ESC"),

	
	std::make_pair(KeyType::SCROLL_UP, "KEY_UP"),
	std::make_pair(KeyType::SCROLL_DOWN, "KEY_DOWN"),
	std::make_pair(KeyType::SCROLL_PRESS, "KEY_HOME"),
	
	std::make_pair(KeyType::DPAD_UP, "KEY_SCROLLUP"),
	std::make_pair(KeyType::DPAD_DOWN, "KEY_SCROLLDOWN"),
	std::make_pair(KeyType::DPAD_LEFT, "KEY_LEFT"),
	std::make_pair(KeyType::DPAD_RIGHT, "KEY_RIGHT"),


	std::make_pair(KeyType::IPOD_CLOCKWISE, "KEY_BRIGHTNESSUP"),
	std::make_pair(KeyType::IPOD_COUNTERCLOCKWISE, "KEY_BRIGHTNESSDOWN"),
	std::make_pair(KeyType::IPOD_PRESS, "KEY_MUTE"),
	
	std::make_pair(KeyType::TALL_CLOCKWISE, "KEY_VOLUMEUP"),
	std::make_pair(KeyType::TALL_COUNTERCLOCKWISE, "KEY_VOLUMEDOWN"),
	std::make_pair(KeyType::TALL_PRESS, "KEY_PAUSE"),
	
	std::make_pair(KeyType::DBL_RING, "KEY_ESC"),
	std::make_pair(KeyType::DBL_PINKIE, "KEY_ESC"),
	std::make_pair(KeyType::DBL_SIDE, "KEY_FORWARD"),
	std::make_pair(KeyType::DBL_TOP, "KEY_SCREENLOCK")
};



cfg_t *parse_conf(const char *filename)
{
  cfg_opt_t opts[] = {
	CFG_BOOL("testing...", cfg_false, CFGF_NONE),
	CFG_BOOL("more testing...", cfg_true, CFGF_NONE),
	CFG_INT("to test....", 1, CFGF_NONE),
	CFG_END()
  };
 
  cfg_t *cfg = cfg_init(opts, CFGF_NONE);
  switch (cfg_parse(cfg, filename)) {
	case CFG_FILE_ERROR:
	    printf("warning: configuration file '%s' could not be read: %s\n", filename, strerror(errno));
	    return 0;
	case CFG_PARSE_ERROR:
	    return 0;
	case CFG_SUCCESS:
	    break;
  }
  return cfg;
}
