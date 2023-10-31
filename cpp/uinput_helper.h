/**
 * @file uinput_helper.hpp
 * @author CK <cwkeller+tourbox@pm.me>
 * @brief Helper functions for working with Linux's `uinput` module.
 *        Based heavily on: https://www.kernel.org/doc/html/latest/input/uinput.html
 * @version 0.1.1
 * @date 2023-20-30
 *
 * @copyright Copyright (c) 2022-2023
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
// #include <map>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "tourbox_keys.h"
#include <iostream>

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
    printf("\nKeyType: %i\nKeyMap: %i\n\n", (int )key, keyMap[key]);
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
    ioctl(fileDescriptor, UI_SET_KEYBIT, BTN_4);
    ioctl(fileDescriptor, UI_SET_KEYBIT, BTN_5); /* Additional mouse buttons */
    ioctl(fileDescriptor, UI_SET_KEYBIT, BTN_6);
    ioctl(fileDescriptor, UI_SET_KEYBIT, BTN_7);
    ioctl(fileDescriptor, UI_SET_KEYBIT, BTN_8);
    ioctl(fileDescriptor, UI_SET_KEYBIT, BTN_9);
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
    strcpy(usetup.name, "Tourbox Neo Virtual Device Userland Driver (Keyboard/Mouse)");

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    return fd;
}

void destroyUinput(int fileDescriptor)
{
    ioctl(fileDescriptor, UI_DEV_DESTROY);
    close(fileDescriptor);
}
