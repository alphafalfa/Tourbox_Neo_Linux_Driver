/**
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
#include <map>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

enum class KeyType
{
    SIDE,
    SCROLL_UP,
    SCROLL_DOWN,
    SCROLL_PRESS,
    NINTENDO_B,
    NINTENDO_A,
    TOP,
    PINKIE,
    RING,
    DPAD_UP,
    DPAD_DOWN,
    DPAD_LEFT,
    DPAD_RIGHT,
    IPOD_CLOCKWISE,
    IPOD_COUNTERCLOCKWISE,
    IPOD_PRESS,
    TALL_CLOCKWISE,
    TALL_COUNTERCLOCKWISE,
    TALL_PRESS,
    MOON,

    DBL_RING,
    DBL_PINKIE,
    DBL_SIDE,
    DBL_TOP
};

static std::map<KeyType, int> keyMap = {std::make_pair(KeyType::SIDE, KEY_BACK),
                                        
                                        std::make_pair(KeyType::SCROLL_UP, KEY_PAGEUP),
                                        std::make_pair(KeyType::SCROLL_DOWN, KEY_PAGEDOWN),
                                        std::make_pair(KeyType::SCROLL_PRESS, KEY_HOME),
                                        
                                        std::make_pair(KeyType::TOP, KEY_ENTER),
                                        std::make_pair(KeyType::NINTENDO_B, BTN_LEFT),
                                        std::make_pair(KeyType::NINTENDO_A, BTN_RIGHT),
                                        std::make_pair(KeyType::PINKIE, KEY_DOT),
                                        std::make_pair(KeyType::RING, KEY_MENU),
                                        
                                        std::make_pair(KeyType::DPAD_UP, KEY_UP),
                                        std::make_pair(KeyType::DPAD_DOWN, KEY_DOWN),
                                        std::make_pair(KeyType::DPAD_LEFT, KEY_LEFT),
                                        std::make_pair(KeyType::DPAD_RIGHT, KEY_RIGHT),


                                        std::make_pair(KeyType::IPOD_CLOCKWISE, KEY_BRIGHTNESSUP),
                                        std::make_pair(KeyType::IPOD_COUNTERCLOCKWISE, KEY_BRIGHTNESSDOWN),
                                        std::make_pair(KeyType::IPOD_PRESS, KEY_MUTE),
                                        
                                        std::make_pair(KeyType::TALL_CLOCKWISE, KEY_VOLUMEUP),
                                        std::make_pair(KeyType::TALL_COUNTERCLOCKWISE, KEY_VOLUMEDOWN),
                                        std::make_pair(KeyType::TALL_PRESS, KEY_PAUSE),
                                        
                                        std::make_pair(KeyType::MOON, KEY_ESC),

                                        std::make_pair(KeyType::DBL_RING, KEY_ESC),
                                        std::make_pair(KeyType::DBL_PINKIE, KEY_ESC),
                                        std::make_pair(KeyType::DBL_SIDE, KEY_ESC),
                                        std::make_pair(KeyType::DBL_TOP, KEY_ESC)
                                        };

void emit(int fileDescriptor, int type, int code, int val)
{
    struct input_event ie;

    ie.type = type;
    ie.code = code;
      ie.value = val;
    /* timestamp values below are ignored */
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    write(fileDescriptor, &ie, sizeof(ie));
}

void generateKeyPressEvent(int fileDescriptor, KeyType key)
{
    emit(fileDescriptor, EV_KEY, keyMap[key], 1);
    emit(fileDescriptor, EV_SYN, SYN_REPORT, 0);
    emit(fileDescriptor, EV_KEY, keyMap[key], 0);
    emit(fileDescriptor, EV_SYN, SYN_REPORT, 0);
}

void registerKeyboardEvents(int fileDescriptor)
{
    // Get the the uinput interface ready to register key and mouse events
    ioctl(fileDescriptor, UI_SET_EVBIT, EV_KEY);

    // These are all of the following key types that the Tourbox will be able to produce using this driver

    for (const auto& keyType : keyMap)
    {
        ioctl(fileDescriptor, UI_SET_KEYBIT, keyType.second);
    }
}

void registerMouseEvents(int fileDescriptor)
{
    // Some desktop environments have a bug in them where, to be able to able generate mouse events,
    // you have to reigster a mouse-left and mouse-right handler as well (so that it thinks it's a real mouse)
    // See: https://askubuntu.com/a/742876/895315

    ioctl(fileDescriptor, UI_SET_EVBIT, EV_KEY);
    ioctl(fileDescriptor, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(fileDescriptor, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(fileDescriptor, UI_SET_EVBIT, EV_REL);

    // Tells the uinput interface to allow mouse movements
    ioctl(fileDescriptor, UI_SET_RELBIT, REL_X);
    ioctl(fileDescriptor, UI_SET_RELBIT, REL_Y);
}

int setupUinput(void)
{
    struct uinput_setup usetup;

    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    registerKeyboardEvents(fd);
    registerMouseEvents(fd);

    usleep(1000);

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x2e3c; /* Per 'lsusb -v' */

    usetup.id.product = 0x5740; /* Might be different for you... */
    strcpy(usetup.name, "Custom Tourbox TBG_H Driver (Keyboard/Mouse)");

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    return fd;
}

void destroyUinput(int fileDescriptor)
{
    ioctl(fileDescriptor, UI_DEV_DESTROY);
    close(fileDescriptor);
}
