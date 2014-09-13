# ASSDraw3

ASSDraw3 is a tool for designing shapes to be used in ASS subtitle file. 

Support is available [on IRC](irc://irc.rizon.net/biki).

## Building ASSDraw3

### Windows

Prerequisites:

1. M$ Visual C++ 2010 or above (Express edition is good enough).
2. wxWidgets (3.0 or above). Build it using nmake. See [this](http://wiki.wxwidgets.org/Compiling_Using_MSVC_On_The_Commandline) for help or look at the documentation.
3. AGG (2.4.* or 2.5.* is good enough). Build using VSC++. A project file is supplied, see ./tools/agg25.vcxproj in this repo.

Building:

1. Open VC++ project file.
2. Add appropriate include and library directories.
3. ??? ~~You're a windows user, you can figure out this step yourself~~
4. Profit.

### Linux

Prerequisites:

1. AGG: `sudo apt-get install libagg-dev`
2. wxWidgets: `git clone https://github.com/wxWidgets/wxWidgets.git; ./configure; make; sudo make install`

Building:

The usual --> `./autogen.sh; ./configure; make`

### OSX

ASSDraw3  was not tested on OSX yet. It *should* work on OSX, give it a go and contact me if it works or not so that I can update this readme.

## License

All source files in this repository are licensed under a 3-clause BSD license.
