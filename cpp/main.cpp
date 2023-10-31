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
// #include <iomanip>
#include <time.h>
#include <iostream>
#include <signal.h>
#include <stdint.h>
#include <string>
#include <termios.h>
#include <unistd.h>
#include "/usr/include/confuse.h"

// Local
#include "uinput_helper.h"

// Remember, can't pass data to signals
int gUinputFileDescriptor = 0;

void sigint_handler(sig_atomic_t s)
{
    std::cout << "\n\nNuked.\n\n" << s << std::endl;
    destroyUinput(gUinputFileDescriptor);
    exit(1);
}

cfg_t *parse_conf(const char *filename)
{
  cfg_opt_t opts[] = {
    CFG_BOOL("passive-mode", cfg_false, CFGF_NONE),
    CFG_BOOL("remote-completion", cfg_true, CFGF_NONE),
    CFG_INT("auto-create-bookmark", 1, CFGF_NONE),
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

int main(int argc, char *argv[])
{
    cfg_t *cfg; /* 'confuse' library for config files */
    /* Localize messages & types according to environment, since v2.9 */
#ifdef LC_MESSAGES /* confuse requests these locale setting */
    setlocale(LC_MESSAGES, "");
    setlocale(LC_CTYPE, "");
#endif

    const char *filename = "tourbox.conf";
    cfg = parse_conf(filename);
    if(!cfg)
    {
        std::cerr << "Failed to open config file: " << filename <<  std::endl;
        return 1;
    }

    // Figure out how to poll for device later...
    std::string ss = "/dev/tty";
    if (argc > 1) /* Allow one to change device if necessary */
      ss.append(std::string(argv[1])); /* "/dev/ttyXXXX" */
    else /* This might be a security concern, later...'' */
      ss.append("ACM0");  /* USB0 is another option */
    
    // Setup and open a serial port 
    const std::string serialPortFile = ss;    
    const int serialPortFileDescriptor = open(serialPortFile.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (serialPortFileDescriptor == -1)
    {
        std::cerr << "Failed to open serial port file " << serialPortFileDescriptor << std::endl;
        return 1;
    }

    struct termios term_options;
    memset(&term_options, 0, sizeof(struct termios));

    term_options.c_cflag = B115200 | CS8 | CREAD;

    if ((tcsetattr(serialPortFileDescriptor, TCSANOW, &term_options)) != 0)
    {
        std::cerr << "Failed to set termios settings";
        close(serialPortFileDescriptor);
        return 1;
    }

    if ((tcflush(serialPortFileDescriptor, TCIOFLUSH)) != 0)
    {
        std::cerr << "Failed to flush termios settings";
        close(serialPortFileDescriptor);
        return 1;
    }

    std::array<uint8_t, 1> readBuffer;

    /// ---------- ///
    /// Setup the virtual driver

    gUinputFileDescriptor = setupUinput();

    // Register signal handler to make sure virtual device gets cleaned up
    signal(SIGINT, sigint_handler);

    while (true)
    {
      int xsleep = 0;
      struct timespec tim, tim2;
      tim.tv_nsec = 0;
      tim.tv_nsec = 100000000;


      ssize_t bytesRead = read(serialPortFileDescriptor, readBuffer.begin(), 1);

      if (bytesRead < 0)
      {
          std::cerr << "Error reading from serial port" << std::endl;
          return 2;
      }

      if (bytesRead > 0)
      {
        int x = readBuffer[0];
        KeyType key = (KeyType )x;
        printf("\n\n%i\t%i\n\n", x, (int )key);
        generateKeyPressEvent(gUinputFileDescriptor, key);
        switch (x)
        {
          case (int )KeyType::RING:
            std::cout << "---- RING ---"; 
            break;
          /*case 0x80:
              xsleep = nanosleep(&tim, &tim2);
              read(serialPortFileDescriptor, readBuffer.begin(),1);
              std::printf("--->Rel:\t%.2X\n", readBuffer[0]);
              if(readBuffer[0] == 0x18){
                std::cout << "DOUBLE Ring 7 pressed" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::DBL_RING);
              }
              else{
                std::cout << "Button Ring 7 pressed" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::RING);
              }
              break;
*/
            case 0x81:
              xsleep = nanosleep(&tim, &tim2);
              read(serialPortFileDescriptor, readBuffer.begin(),1);
              std::printf("--->Rel:\t%.2X\n", readBuffer[0]);
              if(readBuffer[0] == 0x21){
                std::cout << "DOUBLE Side pressed" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::DBL_RING);
              }
              else{
                std::cout << "Side button pressed" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::SIDE);
              }
              break;

            case 0x82:
              xsleep = nanosleep(&tim, &tim2);
              read(serialPortFileDescriptor, readBuffer.begin(),1);
              std::printf("--->Rel:\t%.2X\n", readBuffer[0]);
              if(readBuffer[0] == 0x1f){
                std::cout << "DOUBLE Top pressed" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::DBL_RING);
              }
              else{
                std::cout << "Top Button pressed " << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::TOP);
              }
              break;

            case 0x83:
              xsleep = nanosleep(&tim, &tim2);
              read(serialPortFileDescriptor, readBuffer.begin(),1);
              std::printf("--->Rel:\t%.2X\n", readBuffer[0]);
              if(readBuffer[0] == 0x1c){
                std::cout << "DOUBLE Pinkie pressed" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::DBL_PINKIE);
              }
              else{
                std::cout << "Button Pinkie pressed" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::PINKIE);
              }
              break;

            case 0x84:
                std::cout << "Tall wheel moved counterclockwise" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::TALL_COUNTERCLOCKWISE);
                break;

            case 0x8F:
                std::cout << "iPod wheel moved clockwise" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::IPOD_CLOCKWISE);
                break;

            case 0x90:
                std::cout << "D-Pad up pressed" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::DPAD_UP);
                break;

            case 0x91:
                std::cout << "D-pad down pressed" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::DPAD_DOWN);
                break;

            case 0x92:
                std::cout << "D-pad left pressed" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::DPAD_LEFT);
                break;

            case 0x93:
                std::cout << "D-Pad right pressed" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::DPAD_RIGHT);
                break;

            case 0x0A:
                std::cout << "Scroll wheel clicked" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::SCROLL_PRESS);
                break;

            case 0x09:
                std::cout << "Scroll wheel down used" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::SCROLL_DOWN);
                break;

            case 0x2A:
                std::cout << "Button 11 pressed" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::MOON);
                break;

            case 0x22:
                std::cout << "Button A pressed" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::NINTENDO_B);
                break;

            case 0x23:
                std::cout << "Button B pressed" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::NINTENDO_A);
                break;

            case 0x37:
                std::cout << "Tall center wheel pressed" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::TALL_PRESS);
                break;

            case 0x38:
                std::cout << "iPod Wheel pressed" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::IPOD_PRESS);
                break;

            case 0x44:
                std::cout << "Tall center wheel moved clockwise" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::TALL_CLOCKWISE);
                break;

            case 0x49:
                std::cout << "Scroll wheel up used" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::SCROLL_UP);
                break;

            case 0x4F:
                std::cout << "iPod wheel moved counterclockwise" << std::endl;
                generateKeyPressEvent(gUinputFileDescriptor, KeyType::IPOD_COUNTERCLOCKWISE);
                break;
            default:
                std::printf("Rel:\t%.2X\t%.2X\n", readBuffer[0], readBuffer[1]);
                // Print for testing or debugging. Might want release events in the future.
          }
          std::printf("Key:\t%.2X\t%.2X\n", readBuffer[0], readBuffer[1]);
          if(xsleep < 0)
            fprintf(stderr, "%s: %i", "Nanosleep Failed! Error code:", xsleep);
        }
    }

    // Clean up (although currently you can't get here)
    destroyUinput(gUinputFileDescriptor);

    return 0;
}
