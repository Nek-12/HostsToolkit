
// ----------IMPORTANT------------
#define DOWNLOADED_HOSTS_PREFIX "hosts_"
#define DL_FOLDER               ".temp/"
#define URL_RETRIALS_CNT 5
#define SPLIT_CHAR       '>'
#define VERSION          "3.1.1"
// CONFIG has >CUSTOM, >FILES, >URLS in exactly this order
#define CONFIG_FNAME "settings.txt"

// --------------CROSS-PLATFORM--------------

#ifdef __linux__
#define HOSTS           "/etc/hosts"
#elif _WIN32
#define TIME_MULTIPLIER                                                        \
    400 // on Win, modifying hosts file slows down boot up process,
        // we'll reflect this in statistics
#define HOSTS "C:/Windows/System32/drivers/etc/hosts"
#endif

// -----------STRINGS-----------

#define FILEERRORMSG                                                           \
    "Couldn't read or write your file! Select another location or launch the app "   \
    "with admin privileges."

#define CONFIG_ERROR_MSG                                                       \
    "The program couldn't read " CONFIG_FNAME                                  \
    " Your custom settings file may be corrupted."
#define CREDITS                                                                \
    "# This file was generated with HostsTools: "                              \
    "https://github.com/Nek-12/HostsTools \n"

#define LICENSE                                                                \
    "This program is free software: you can redistribute it and/or modify it " \
    "under the terms of the GNU General Public License as published by the "   \
    "Free Software Foundation, either version 3 of the License, "              \
    "or (at your option) any later version. "                                   \
    "This program is distributed in the hope that it will be useful, "          \
    "but WITHOUT ANY WARRANTY;"                                                \
    "without even the implied warranty of MERCHANTABILITY or "                  \
    "FITNESS FOR A PARTICULAR PURPOSE. "                                        \
    "See the GNU General Public License for more details."
