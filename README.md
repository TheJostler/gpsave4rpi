# gpsave4rpi
My custom Auto Backup Binary activated by gpio input.

This script starts listening for GPIO input using SYSFS library.
When it is triggered the program will start to recursively copy all files found in the "src" directory into the "dest"
directory, 

I hope to add a GPIO indicator LED sson, to indicate when copying starts and finishes

All options can be configured in the /etc/gpsave/gpsave.config file which is parsed when the program begins.

##Install

Currently this utility only runs on Raspberry pi with Sysfs installed
Installing is easy simply clone the repo
```git clone https://github.com/TheJostler/gpsave4rpi.git```

```cd gpsave4rpi```

```sudo make install```

##Configuration

Edit the config file in the 'gpsave4rpi' directory and run:

```sudo make config```

This will automatically update your copy in the etc folder

##Update

To update the binary in $PATH run

```sudo make update```

##Remove

To uninstall gpsave4rpi and remove all files relating to it run:

```sudo make remove```
