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

#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/uinput.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string>
#include <map>
#include <confuse.h>


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

#define	DIAL_PRESS            0x38
#define	DIAL_COUNTERCLOCKWISE 0x4f
#define	DIAL_CLOCKWISE        0x8f  // Large flat disc 

#define	KNOB_PRESS            0x37
#define	KNOB_CLOCKWISE        0x44  // Central knob 
#define	KNOB_COUNTERCLOCKWISE 0x84

// === Default buttons for the device === 
static std::map<int, int> keyMap = {      // Grouped primarily for readibility 
  std::make_pair(NINTENDO_B, BTN_LEFT),   // These two are      
	std::make_pair(NINTENDO_A, BTN_RIGHT),  // Mouse buttons.     
	
	std::make_pair(SIDE, KEY_CALC),	        // Calculator should pop up
	std::make_pair(TOP, KEY_REFRESH),	      // Refresh your browser
	std::make_pair(PINKIE, KEY_FORWARD),	  // Browse forwared ->  
	std::make_pair(RING, KEY_BACK),	        // <- Browse back     
	std::make_pair(MOON, KEY_MUTE),         // Mute your speakers

	
  std::make_pair(WHEEL_UP, REL_WHEEL),    // Scroll wheel acts like
	std::make_pair(WHEEL_DOWN, REL_WHEEL),  // mouse wheel. Press jumps
	std::make_pair(WHEEL_PRESS, KEY_HOME),  // to beginning of document.
	
	std::make_pair(DPAD_UP, KEY_UP),        // Just arrow keys 
	std::make_pair(DPAD_DOWN, KEY_DOWN),    // in case you wanna  
	std::make_pair(DPAD_LEFT, KEY_LEFT),    // bounce around a
	std::make_pair(DPAD_RIGHT, KEY_RIGHT),  // spreadsheet.


	std::make_pair(DIAL_CLOCKWISE, KEY_BRIGHTNESSUP),             // Quick brightness 
	std::make_pair(DIAL_COUNTERCLOCKWISE, KEY_BRIGHTNESSDOWN),    // settings for the  
	std::make_pair(DIAL_PRESS, KEY_MICMUTE), // <- for gamers     // discrerning hacker.
	
	std::make_pair(KNOB_CLOCKWISE, KEY_VOLUMEUP),           // Weird bug, the KEY_VOLUMEUP
	std::make_pair(KNOB_COUNTERCLOCKWISE, KEY_VOLUMEDOWN),  // may let you go up well past  
	std::make_pair(KNOB_PRESS, KEY_PLAYPAUSE),              // 100%.... like 1,000% 
	
	std::make_pair(DBL_RING, KEY_CAMERA),               // These may depend on your
	std::make_pair(DBL_PINKIE, KEY_ALL_APPLICATIONS),   // computer's complement of 
  std::make_pair(DBL_SIDE, KEY_SLEEP),                // hardware doo-dads and 
	std::make_pair(DBL_TOP, KEY_SCREENLOCK)             // accessories.
};


cfg_t *parse_conf(const char *filename)
{
  cfg_opt_t opts[] = {
	CFG_STR("NINTENDO_A", "BTN_LEFT", CFGF_NONE),
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

void generateKeyPressEvent(const int &fd, const int &key)
{    
 
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

int setupUinput(void)
{
    struct uinput_setup usetup;

    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    ioctl(fd, UI_SET_EVBIT, EV_KEY);     // Regular buttons
	  for (const auto& keyType : keyMap)               // Keyboard 
	    ioctl(fd, UI_SET_KEYBIT, keyType.second); // Mouse 
    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);  // /Clicky*
    ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT); // /Clicky* !!
    
    ioctl(fd, UI_SET_EVBIT, EV_REL);      // Relative buttons
    ioctl(fd, UI_SET_RELBIT, REL_WHEEL);  // Vertical Wheel
    ioctl(fd, UI_SET_RELBIT, REL_HWHEEL); // Horizontal Wheel

    usleep(1000);

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x2e3c; // Per 'lsusb -v' 

    usetup.id.product = 0x5740; // Might be different for you... 
    strcpy(usetup.name, "Tourbox Neo Virtual Device Userland Driver (Keyboard/Mouse)");

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    return fd;
}

void destroyUinput(int fd)
{
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
}
