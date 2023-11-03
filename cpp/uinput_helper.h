/*
 * @file uinput_helper.hpp
 * @author CK <cwkeller+tourbox@pm.me>
 * @brief Helper functions for working with Linux's `uinput` module.
 *        Based heavily on: https://www.kernel.org/doc/html/latest/input/uinput.html
 * @version 0.1.1
 * @date 2023-20-30
 *
 * @copyright Copyright (c)2023
 *
 *
 * @file uinput_helper.hpp
 * @author Raleigh Littles <raleighlittles@gmail.com>
 * @brief Helper functions for working with Linux's `uinput` module.
 *        Based heavily on: https://www.kernel.org/doc/html/latest/input/uinput.html
 * @version 0.1
 * @date 2022-07-24
 *
 * @copyright Copyright (c) 2022
 *
*/

#include <array>
#include <charconv>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/uinput.h>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <unordered_map>
#include <confuse.h>
#include <utility>


#define	DBL_TOP     0x13
#define	DBL_RING    0x18   // Some keys report double click 
#define	DBL_PINKIE  0x1c
#define	DBL_SIDE    0x21

#define	NINTENDO_B  0x22   // Two small circles near Tourbox logo. 
#define	NINTENDO_A  0x23
#define	MOON        0x2a   // Next to tall knob 
	
#define	RING        0x80
#define	SIDE        0x81   // Various small buttons 
#define	TOP         0x82
#define	PINKIE      0x83  // On bottom right 

#define	WHEEL_DOWN   0x09
#define	WHEEL_PRESS  0x0a
#define	WHEEL_UP     0x49 // Large Mouse wheel 

#define	DPAD_UP     0x90  // Four arrows. 
#define	DPAD_DOWN   0x91
#define	DPAD_LEFT   0x92
#define	DPAD_RIGHT  0x93

#define	DIAL_PRESS   0x38
#define	DIAL_COUNTER 0x4f
#define	DIAL_CLOCK   0x8f  // Large flat disc 

#define	KNOB_PRESS   0x37
#define	KNOB_CLOCK   0x44  // Central knob 
#define	KNOB_COUNTER 0x84
    
struct keyfig {
  bool flag;
  short rela;
  int tkey;
  int kcod;
  std::string dact;
  std::string exes;
};

std::array<keyfig, 24> keyf = 
  {{{ false,  1,  NINTENDO_B,   KEY_CAMERA_ACCESS_DISABLE, "KEY_CAMERA_ACCESS_DISABLE", "0" },
    { false,  1,  NINTENDO_A,   KEY_CAMERA_ACCESS_ENABLE,  "KEY_CAMERA_ACCESS_ENABLE",  "0" },
    { false,  1,  SIDE,         KEY_CALC,                  "KEY_CALC",                  "0" },
    { false,  1,  TOP,          KEY_REFRESH,               "KEY_REFRESH",               "0" },
    { false,  1,  PINKIE,       KEY_FORWARD,               "KEY_FORWARD",               "0" },
    { false,  1,  RING,         KEY_BACK,                  "KEY_BACK",                  "0" },
    { false,  1,  MOON,         KEY_MUTE,                  "KEY_MUTE",                  "0" },
    { false,  1,  WHEEL_UP,     BTN_WHEEL,                 "BTN_WHEEL",                 "0" },
    { false,  1,  WHEEL_DOWN,   BTN_WHEEL,                 "BTN_WHEEL",                 "0" },
    { false,  1,  WHEEL_PRESS,  KEY_HOME,                  "KEY_HOME",                  "0" },
    { false,  1,  DPAD_UP,      KEY_UP,                    "KEY_UP",                    "0" },
    { false,  1,  DPAD_DOWN,    KEY_DOWN,                  "KEY_DOWN",                  "0" },
    { false,  1,  DPAD_LEFT,    KEY_LEFT,                  "KEY_LEFT",                  "0" },
    { false,  1,  DPAD_RIGHT,   KEY_RIGHT,                 "KEY_RIGHT",                 "0" },   
    { false,  1,  DIAL_CLOCK,   KEY_BRIGHTNESSUP,          "KEY_BRIGHTNESSUP",          "0" },
    { false,  1,  DIAL_COUNTER, KEY_BRIGHTNESSDOWN,        "KEY_BRIGHTNESSDOWN",        "0" },
    { false,  1,  DIAL_PRESS,   KEY_MICMUTE,               "KEY_MICMUTE",               "0" },
    { false,  1,  KNOB_CLOCK,   KEY_VOLUMEUP,              "KEY_VOLUMEUP",              "0" },
    { false,  1,  KNOB_COUNTER, KEY_VOLUMEDOWN,            "KEY_VOLUMEDOWN",            "0" },
    { false,  1,  KNOB_PRESS,   KEY_PLAYPAUSE,             "KEY_PLAYPAUSE",             "0" },
    { false,  1,  DBL_RING,     KEY_CAMERA,                "KEY_CAMERA",                "0" },
    { false,  1,  DBL_PINKIE,   KEY_ALL_APPLICATIONS,      "KEY_ALL_APPLICATIONS",      "0" },
    { false,  1,  DBL_SIDE,     KEY_SLEEP,                 "KEY_SLEEP",                 "0" },
    { false,  1,  DBL_TOP,      KEY_SCREENLOCK,            "KEY_SCREENLOCK",            "0" }}};



cfg_t *parse_conf(const char *filename)
{
  cfg_opt_t opts[] {
    CFG_STR("NINTENDO_B", "KEY_CAMERA_ACCESS_DISABLE", CFGF_NONE),    //       
	  CFG_STR("NINTENDO_A", "KEY_CAMERA_ACCESS_ENABLE", CFGF_NONE),  // Mouse buttons     
	
	  CFG_STR("SIDE", "KEY_CALC", CFGF_NONE),	        // Calculator should pop up
	  CFG_STR("TOP", "KEY_REFRESH", CFGF_NONE),	      // Refresh your browser
	  CFG_STR("PINKIE", "KEY_FORWARD", CFGF_NONE),	  // Browse forwared ->  -
	  CFG_STR("RING", "KEY_BACK", CFGF_NONE),	        // <- Browse back     
	  CFG_STR("MOON", "KEY_FORWARD", CFGF_NONE),         // Mute your speakers


    CFG_STR("WHEEL_UP", "REL_WHEEL", CFGF_NONE),    // Scroll wheel acts like
	  CFG_STR("WHEEL_DOWN", "REL_WHEEL", CFGF_NONE),  // mouse wheel. Press jumps
	  CFG_STR("WHEEL_PRESS", "KEY_HOME", CFGF_NONE),  // to beginning of document.
	
	  CFG_STR("DPAD_UP", "KEY_UP", CFGF_NONE),        // Just arrow keys 
	  CFG_STR("DPAD_DOWN", "KEY_DOWN", CFGF_NONE),    // in case you wanna  
	  CFG_STR("DPAD_LEFT", "KEY_LEFT", CFGF_NONE),    // bounce around a
	  CFG_STR("DPAD_RIGHT", "KEY_RIGHT", CFGF_NONE),  // spreadsheet.

	  CFG_STR("DIAL_CLOCK", "KEY_BRIGHTNESSUP", CFGF_NONE),             // Quick brightness 
	  CFG_STR("DIAL_COUNTER", "KEY_BRIGHTNESSDOWN", CFGF_NONE),    // settings for the  
	  CFG_STR("DIAL_PRESS", "KEY_MICMUTE", CFGF_NONE), // <- for gamers     // discrerning hacker.s
	
	  CFG_STR("KNOB_CLOCK", "KEY_VOLUMEUP", CFGF_NONE),           // Weird bug, the KEY_VOLUMEUP
	  CFG_STR("KNOB_COUNTER", "KEY_VOLUMEDOWN", CFGF_NONE),  // may let you go up well past  
  	CFG_STR("KNOB_PRESS", "KEY_PLAYPAUSE", CFGF_NONE),              // 100%.... like 1,000% 
	
	  CFG_STR("DBL_RING", "KEY_CAMERA", CFGF_NONE),               // These may depend on your
	  CFG_STR("DBL_PINKIE", "KEY_ALL_APPLICATIONS", CFGF_NONE),   // computer's complement of 
    CFG_STR("DBL_SIDE", "KEY_SLEEP", CFGF_NONE),                // hardware doo-dads and 
	  CFG_STR("DBL_TOP", "KEY_SCREENLOCK", CFGF_NONE),
    CFG_END()
  };
  cfg_t *cfg = cfg_init(opts, CFGF_NONE);

  switch (cfg_parse(cfg, filename)) {
	  case CFG_FILE_ERROR:
	    printf("warning: configuration file '%s' could not be found: %s\n", filename, strerror(errno));
	    return 0;
	  case CFG_PARSE_ERROR:
	    printf("warning: configuration file '%s' read error:  %s\n", filename, strerror(errno));
	    return 0;
    case CFG_SUCCESS:
	    break;
  }
  return cfg;
}

void emit(const int &fd, const int &type, const int &code, const int &val)
{
    struct input_event ie;

    ie.type = type;
    ie.code = code;
    ie.value = val;
    // timestamp values below are ignored 
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    write(fd, &ie, sizeof(ie));
}

void generateKeyPressEvent(const int &fd, const int &key, cfg_t *cfg, std::unordered_map<int, keyfig> &keyf)
{    
  char *kstring = cfg_getstr(cfg, keyString[key].c_str());
  if((keyf.second.find(kstring) != keyf.end()) && keyf.second.find(kstring)->second.fmod) {
  std::cout << kstring << "\t" << sizeof(keyf) << std::endl ;
    if (WHEEL_DOWN == key)                // The mouse wheel has special 
      emit(fd, EV_REL, REL_WHEEL, -1);  // relative properties which 
    else if (WHEEL_UP == key)             // we implement here. 
      emit(fd, EV_REL, REL_WHEEL, +1);  // (+ show for clarity)
    else{
      emit(fd, EV_KEY, keyMap[key], 1); // Otherwise it's simple binary -
      emit(fd, EV_SYN, SYN_REPORT, 0);   // Button down and button up.
      emit(fd, EV_KEY, keyMap[key], 0);
    }
    emit(fd, EV_SYN, SYN_REPORT, 0);     // Let's the kernel know you're done.
  }
}





int setupUinput(void)
{
    usleep(1000);
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fd){
      ioctl(fd, UI_SET_EVBIT, EV_KEY);     // Regular buttons
	    for (const auto& keyType : keyMap)               // Keyboard 
	     ioctl(fd, UI_SET_KEYBIT, keyType.second); // Mouse 
      ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);  // /Clicky*
      ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT); // /Clicky* !!
    
      ioctl(fd, UI_SET_EVBIT, EV_REL);      // Relative buttons
      ioctl(fd, UI_SET_RELBIT, REL_WHEEL);  // Vertical Wheel
      ioctl(fd, UI_SET_RELBIT, REL_HWHEEL); // Horizontal Wheel

      usleep(1000);

      struct uinput_setup usetup;
      memset(&usetup, 0, sizeof(usetup));
      usetup.id.bustype = BUS_USB;
      usetup.id.vendor = 0x2e3c; // Per 'lsusb -v' 

      usetup.id.product = 0x5740; // Might be different for you... 
      strcpy(usetup.name, "Tourbox Neo Virtual Device Userland Driver (Keyboard/Mouse)");

      ioctl(fd, UI_DEV_SETUP, &usetup);
      ioctl(fd, UI_DEV_CREATE);
    }
    else {
      fprintf(stderr, "Unable to open /dev/uinput: %i\n\n", fd);
    }
    return fd;
}

void destroyUinput(int fd)
{
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
}
