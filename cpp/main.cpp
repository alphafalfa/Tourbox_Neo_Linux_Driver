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

int main(int argc, char *argv[])
{
    cfg_t *cfg; /* 'confuse' library for config files */
    /* Localize messages & types according to environment, since v2.9 */
#ifdef LC_MESSAGES /* confuse requests these locale setting */
    setlocale(LC_MESSAGES, "");
    setlocale(LC_CTYPE, "");
#endif

    char *filename = (char *)"tourbox.conf";
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
    const int serialPortFileDescriptor = open(ss.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);

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

    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = 25000000L;
    KeyType key;
    ssize_t bytesRead = read(serialPortFileDescriptor, readBuffer.begin(),1);
    while(0 <= bytesRead)
    {
      bytesRead = read(serialPortFileDescriptor, readBuffer.begin(),1);
      if(0 < bytesRead)
      {
        key = (KeyType )readBuffer[0];
        if (KeyType::PINKIE == key 
         || KeyType::RING == key  
         || KeyType::SIDE == key
         || KeyType::TOP == key){
            nanosleep(&tim, &tim2);
            bytesRead = read(serialPortFileDescriptor, readBuffer.begin(),1);
            if(0 != bytesRead)
              key = (KeyType )readBuffer[0]; 
        }
        generateKeyPressEvent(gUinputFileDescriptor, key);
        nanosleep(&tim, &tim2);
      }
      else 
        nanosleep(&tim, &tim2);
    }
    // Clean up (although currently you can't get here)
    destroyUinput(gUinputFileDescriptor);

    return 0;
}
