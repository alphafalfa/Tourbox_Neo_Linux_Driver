/**
  * @file main.cpp
 * @author Raleigh Littles <raleighlittles@gmail.com>
 * @brief Entrypoint for Tourbox driver
 * @version 0.1.1
 * @date 2023-10-30
 *
 * @copyright Copyright (c) 2022-2023
 *
 */

#include <array>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <sys/types.h>
#include <time.h>
#include <iostream>
#include <signal.h>
#include <stdint.h>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <confuse.h>
// #include <FL/Fl.H>
// #include <FL/Fl_Window.H>
// #include <FL/Fl_Box.H>
// Local
#include "uinput_helper.h"
// Remember, can't pass data to signals
static int gUinputFileDescriptor = 0;

void sigint_handler(sig_atomic_t s)
{
    const int error = gUinputFileDescriptor;
    std::cout << "\n\nNuked.\n\n" << s << std::endl;
    destroyUinput(gUinputFileDescriptor);
    exit(error);
}


int main(int argc, char *argv[])
{
  /* === For libconfuse to handle config files === */
    /* Localize messages & types according to environment, since v2.9 */
#ifdef LC_MESSAGES 
    setlocale(LC_MESSAGES, "");
    setlocale(LC_CTYPE, "");
#endif
    /* === it's no longer confused, it's libconfused=== */

    /* === Tentatively planning on FLTK as a lightweight GUI */
    // Fl_Window *window = new Fl_Window(999,444);
    // Fl_Box *box = new Fl_Box(320,240,300,100,"Hello, World!");
    // box->box(FL_UP_BOX);
    // box->labelfont(FL_BOLD+FL_ITALIC);
    // box->labelsize(36);
    // box->labeltype(FL_SHADOW_LABEL);
    // window->end();
    // window->show(argc, argv);
    // return Fl::run();

    const char *filename = (char *)"tourbox.conf";
    cfg_t *cfg = parse_conf(filename);

    if(!cfg)
    {
        std::cerr << "Failed to open config file: " << filename <<  std::endl;
        exit(1);
    }

    std::cout << "Nintendo A: " << cfg_getstr(cfg, "NINTENDO_A") << std::endl << std::endl; 
    // Figure out how to poll for device later...
    std::string ss = "/dev/tty";
    if (argc > 1 && 4 >= strlen(argv[1]))   /* Allow one to change device if necessary */
      ss.append(argv[1],0,strlen(argv[1])); /* But we don't want huge arbitrary amounts of data. */
    else                                    /* Will have other options in the future. */
      ss.append("ACM0");  /* USB0 is another potential option, for example */

    // Setup and open a serial port 
    const int serialPortFileDescriptor = open(ss.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serialPortFileDescriptor == -1)
    {
        std::cerr << "Failed to open serial port: " << ss << std::endl;
        std::cerr << "Did you forget to plug in the TourBox?"  << std::endl;
        exit(serialPortFileDescriptor);
    }

    struct termios term_options;
    memset(&term_options, 0, sizeof(struct termios));

    term_options.c_cflag = B115200 | CS8 | CREAD;

    if ((tcsetattr(serialPortFileDescriptor, TCSANOW, &term_options)) != 0)
    {
        std::cerr << "Failed to set termios settings";
        close(serialPortFileDescriptor);
        exit(2);
    }

    if ((tcflush(serialPortFileDescriptor, TCIOFLUSH)) != 0)
    {
        std::cerr << "Failed to flush termios settings";
        close(serialPortFileDescriptor);
        exit(3);
    }

    std::array<uint8_t, 1> readBuffer;

    /// Setup the virtual driver
    gUinputFileDescriptor = setupUinput();

    // Register signal handler to make sure virtual device gets cleaned up
    signal(SIGINT, sigint_handler);

    struct timespec tim, tim2;
    tim.tv_sec = 0;          // 0 seconds, plus
    tim.tv_nsec = 25000000L; // 25 milliseconds
    
    int key;
    ssize_t bytesRead = read(serialPortFileDescriptor, readBuffer.begin(),1);
    while(0 <= bytesRead)   /* If there's a error accessing the buffer we dip out gracefuly...*/
    {
      bytesRead = read(serialPortFileDescriptor, readBuffer.begin(),1); /* get some data */
      if(0 < bytesRead) /* make sure they pressed a button (or nano sleep below) */
      {
        key = readBuffer[0];  /* use our KeyType to make it easier to read */
        if (PINKIE == key      
         || RING == key        // We are
         || SIDE == key        // looking for    // looking for
         || TOP == key){       // double clicks.
            nanosleep(&tim, &tim2);     // Give them about 25ms to double click
            bytesRead = read(serialPortFileDescriptor, readBuffer.begin(),1);
            if(0 != bytesRead)                
              key = readBuffer[0]; // Double click returns a different code from device.
          }                                  // Only four of the buttons have this feature.
          generateKeyPressEvent(gUinputFileDescriptor, cfg, key);
      }                           // ..slow..down...
      else                        //...........the.. 
        nanosleep(&tim, &tim2);   // ............read..  
    }                             // ..............buffer....
    destroyUinput(gUinputFileDescriptor);

    return 0;
}
