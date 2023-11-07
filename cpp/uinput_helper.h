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
#include <sys/select.h>
#define DEBUG

#ifdef DEBUG
 #define D(x) x
#else 
 #define D(x)
#endif



#include <array>
#include <asm-generic/ioctl.h>
#include <charconv>
#include <climits>
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
    
using namespace std;

static cfg_t *cfg;

struct keyfigure {
  bool flag;
  short rel;
  int tcode;
  std::string tstr;
  int kcode;
  std::string kstr;
  std::string exec;
};

static array<keyfigure, 24> keyfig = 
{{{ false,  1,  NINTENDO_B,      "NINTENDO_B",      KEY_CAMERA_ACCESS_DISABLE, "KEY_CAMERA_ACCESS_DISABLE", "0" },
  { false,  1,  NINTENDO_A,      "NINTENDO_A",      KEY_CAMERA_ACCESS_ENABLE,  "KEY_CAMERA_ACCESS_ENABLE",  "0" },
  { false,  1,  SIDE,            "SIDE",            KEY_CALC,                  "KEY_CALC",                  "0" },
  { false,  1,  TOP,             "TOP",             KEY_REFRESH,               "KEY_REFRESH",               "0" },
  { false,  1,  PINKIE,          "PINKIE",          KEY_FORWARD,               "KEY_FORWARD",               "0" },
  { false,  1,  RING,            "RING",            KEY_BACK,                  "KEY_BACK",                  "0" },
  { false,  1,  MOON,            "MOON",            KEY_FORWARD,               "KEY_FORWARD",               "0" },
  { false,  1,  WHEEL_UP,        "WHEEL_UP",        REL_WHEEL,                 "REL_WHEEL",                 "0" },
  { false,  1,  WHEEL_DOWN,      "WHEEL_DOWN",      REL_WHEEL,                 "REL_WHEEL",                 "0" },
  { false,  1,  WHEEL_PRESS,     "WHEEL_PRESS",     KEY_HOME,                  "KEY_HOME",                  "0" },
  { false,  1,  DPAD_UP,         "DPAD_UP",         KEY_UP,                    "KEY_UP",                    "0" },
  { false,  1,  DPAD_DOWN,       "DPAD_DOWN",       KEY_DOWN,                  "KEY_DOWN",                  "0" },
  { false,  1,  DPAD_LEFT,       "DPAD_LEFT",       KEY_LEFT,                  "KEY_LEFT",                  "0" },
  { false,  1,  DPAD_RIGHT,      "DPAD_RIGHT",      KEY_RIGHT,                 "KEY_RIGHT",                 "0" },
  { false,  1,  DIAL_CLOCK,      "DIAL_CLOCK",      KEY_BRIGHTNESSUP,          "KEY_BRIGHTNESSUP",          "0" },
  { false,  1,  DIAL_COUNTER,    "DIAL_COUNTER",    KEY_BRIGHTNESSDOWN,        "KEY_BRIGHTNESSDOWN",        "0" },
  { false,  1,  DIAL_PRESS,      "DIAL_PRESS",      KEY_MICMUTE,               "KEY_MICMUTE",               "0" },
  { false,  1,  KNOB_CLOCK,      "KNOB_CLOCK",      KEY_VOLUMEUP,              "KEY_VOLUMEUP",              "0" },
  { false,  1,  KNOB_COUNTER,    "KNOB_COUNTER",    KEY_VOLUMEDOWN,            "KEY_VOLUMEDOWN",            "0" },
  { false,  1,  KNOB_PRESS,      "KNOB_PRESS",      KEY_PLAYPAUSE,             "KEY_PLAYPAUSE",             "0" },
  { false,  1,  DBL_RING,        "DBL_RING",        KEY_CAMERA,                "KEY_CAMERA",                "0" },
  { false,  1,  DBL_PINKIE,      "DBL_PINKIE",      KEY_ALL_APPLICATIONS,      "KEY_ALL_APPLICATIONS",      "0" },
  { false,  1,  DBL_SIDE,        "DBL_SIDE",        KEY_SLEEP,                 "KEY_SLEEP",                 "0" },
  { false,  1,  DBL_TOP,         "DBL_TOP",         KEY_SCREENLOCK,            "KEY_SCREENLOCK",            "0" }}};


std::string parse_conf(const char *filename) 
{
  D(printf("filename: %s\n", (char *)filename);)

  cfg_opt_t key[] = {
    CFG_BOOL("flag", cfg_false, CFGT_NONE),
    CFG_INT("rel", 1, CFGT_NONE),
    CFG_STR("exec", 0, CFGT_NONE),
    CFG_END()
  };

/*  cfg_opt_t keys[] = {
    CFG_SEC("NINTENDO_B", key_opts, CFGF_NONE),
    CFG_SEC("NINTENDO_A", key_opts, CFGF_NONE),
    CFG_SEC("SIDE", key_opts, CFGF_NONE),
    CFG_SEC("TOP", key_opts, CFGF_NONE),
    CFG_SEC("PINKIE", key_opts, CFGF_NONE),
    CFG_SEC("RING", key_opts, CFGF_NONE),
    CFG_SEC("MOON", key_opts, CFGF_NONE),
    CFG_SEC("WHEEL_UP", key_opts, CFGF_NONE),
    CFG_SEC("WHEEL_DOWN", key_opts, CFGF_NONE),
    CFG_SEC("WHEEL_PRESS", key_opts, CFGF_NONE),
    CFG_SEC("DPAD_UP", key_opts, CFGF_NONE),
    CFG_SEC("DPAD_DOWN", key_opts, CFGF_NONE),
    CFG_SEC("DPAD_LEFT", key_opts, CFGF_NONE),
    CFG_SEC("DPAD_RIGHT", key_opts, CFGF_NONE),
    CFG_SEC("DIAL_CLOCK", key_opts, CFGF_NONE),
    CFG_SEC("DIAL_COUNTER", key_opts, CFGF_NONE),
    CFG_SEC("DIAL_PRESS", key_opts, CFGF_NONE),
    CFG_SEC("KNOB_CLOCK", key_opts, CFGF_NONE),
    CFG_SEC("KNOB_COUNTER", key_opts, CFGF_NONE),
    CFG_SEC("KNOB_PRESS", key_opts, CFGF_NONE),
    CFG_SEC("DBL_RING", key_opts, CFGF_NONE),
    CFG_SEC("DBL_PINKIE", key_opts, CFGF_NONE),
    CFG_SEC("DBL_SIDE", key_opts, CFGF_NONE),
    CFG_SEC("DBL_TOP", key_opts, CFGF_NONE), 
    CFG_END()
  };
*/
  cfg_opt_t opts[] = {
    CFG_FLOAT("VERSION", 0.0, CFGF_NONE),
    CFG_STR("tty", "ACM1", CFGF_NONE),
    CFG_SEC("key", key, CFGF_MULTI | CFGF_TITLE),
    CFG_END()
  };

  D(printf("opts created.\ncfg_init:...");)

  cfg_t* cfg = cfg_init(opts, CFGF_NONE);
  D(printf("cfg_getfloat cfg, \"VERSION\"\nsuccess.\ncfg_parse:");)
  switch (cfg_parse(cfg, filename)) {
	  case CFG_FILE_ERROR:
	    printf("warning: configuration file '%s' could not be found: %s\n", filename, strerror(errno));
	    return "0";
	  case CFG_PARSE_ERROR:
	    printf("warning: configuration file '%s' read error:  %s\n", filename, strerror(errno));
	    return "0";
    case CFG_SUCCESS:
	    break;
  }
  unsigned int i;
  cfg_t *sec;
 /* Iterate over the sections and print fields from each section. */
	for (i = 0; i < cfg_size(cfg, "key"); i++) {
		sec = cfg_getnsec(cfg, "key", i);

			printf("group title:  '%s'\n", cfg_title(sec));
			printf("group number:  %i\n", (int )cfg_getbool(sec, "flag"));
			printf("group total:   %i\n", (int )cfg_getint(sec, "rel"));
			printf("\n");
	}

  D(printf("cfg_getfloat cfg, \"VERSION\": %f\n...success.\n\ncfg_parse: ", cfg_getfloat(cfg, "VERSION"));)

  unsigned int j = 0;
  std::string keystr, titstr;
  for (i = 0; i < cfg_size(cfg,"key"); i++) {
    sec = cfg_getnsec(cfg, "key", i);
    if(cfg_getbool(sec, "flag")){
      while(keyfig[j].tstr != cfg_title(sec)){
        keystr = keyfig[j].tstr;
        titstr = cfg_title(sec);
        printf("j: %i\tkeyfig: %s\ttitle: %s", j, keystr.c_str(), titstr.c_str());
        j++;
      }

      std::string keystr = keyfig[j].tstr;
      std::string titstr = cfg_title(sec);
    	printf("j: %i\tkeyfig: %s\ttitle: %s", j, keystr.c_str(), titstr.c_str());	
      keyfig[j].flag = true;
      keyfig[j].rel = cfg_getint(sec, "rel");
      keyfig[j].exec = cfg_getstr(sec, "exec");
    }
  }

  return cfg_getstr(cfg, "tty");
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

void generateKeyPressEvent(const int &fd, int key)
{    

  keyfigure kstring = keyfig[1];
  cfg_t *sub = cfg_getnsec(cfg, "NINTENDO_A", 0);

  D(printf("\ngenerateKeyPressEvent #221\t%i\n", key);)
  D(printf("cfg_getstr(sub, \"exec\")\t key: %s\n", cfg_getstr(sub, "exec"));)
  D(printf("kstring.kstr \t%s\t%lu\n", kstring.kstr.c_str(), sizeof(kstring));)
  if (WHEEL_DOWN == kstring.tcode)                // The mouse wheel has special re
      emit(fd, EV_REL, REL_WHEEL, -1);  // relative properties which 
  else if (WHEEL_UP == kstring.tcode)             // we implement here. 
      emit(fd, EV_REL, REL_WHEEL, +1);  // (+ show for clarity)
  else{
      emit(fd, EV_KEY, kstring.kcode, 1); // Otherwise it's simple binary -
      emit(fd, EV_SYN, SYN_REPORT, 0);   // Button down and button up.
      emit(fd, EV_KEY, kstring.kcode, 0);
    }
    emit(fd, EV_SYN, SYN_REPORT, 0);     // Let's the kernel know you're done.
}

int setupUinput(void)
{
    usleep(1000);
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fd){
      ioctl(fd, UI_SET_EVBIT, EV_REP);     // Regular buttons
	    for (int i = 0 ; i < 24 ; i++){
 //      D(printf("keyfig[%i].kode: %i\n",i,keyfig[i].kcode);) 
        ioctl(fd, UI_SET_KEYBIT, keyfig[i].kcode); // Keyboard 
      }
	    // ioctl(fd, UI_SET_KEYBIT, keyType.second); // Mouse 
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
      fprintf(stderr, "Unable to open /dev/uinput: %i\n", fd);
    }
    return fd;
}

void destroyUinput(int fd)
{
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
}
