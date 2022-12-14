![Made with C++](https://forthebadge.com/images/badges/made-with-c-plus-plus.svg)
[![Powered by Qt](https://forthebadge.com/images/badges/powered-by-qt.svg)](https://qt.io)
[![CodeFactor](https://www.codefactor.io/repository/github/tim-gromeyer/converter/badge/main)](https://www.codefactor.io/repository/github/tim-gromeyer/converter/overview/main)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![GitHub release](https://img.shields.io/github/release/tim-gromeyer/Converter.svg)](https://github.com/tim-gromeyer/Converter/releases/)


# [SConverter](https://tim-gromeyer.github.io/Converter/)

Simple, fast, and powerful: convert your strings with SConverter.

## Features

- Undo and redo actions
- Real-time preview with less than 1 ms delay
- Syntax highlighting editor
- Light on resources
- Quick access to recent files through the toolbar or file menu
- Open source
- Native look and feel

| From         	| To                                            	|
|--------------	|-----------------------------------------------	|
| **Plain**    		| C-string<br>Sorted<br>MD5<br>SHA256<br>SHA512 	|
| **Markdown** 	|                 HTML<br>Plain                 	|
| **HTML**     	|               Markdown<br>Plain               	|
| **C-string** 		|                     Plain                     	|

![Example](docs/images/Example.png)

## Preview

A preview is available [here](https://tim-gromeyer.github.io/Converter/converter.html).

Important:

- Performance might be bad

## Dependencies:

[Qt](https://qt.io/).

-------

## Build instructions

Run the [build script](scripts/build.sh) or follow the instructions below.

- Clone Converter: `git clone https://github.com/tim-gromeyer/Converter --depth=1 && cd Converter`
- Clone all repositories required by Converter by running the command `git submodule update --init --recursive -j 3 --depth=1`.
- Create the build folder: `mkdir build && cd build`.
- Now create a Makefile with CMake: `cmake ..`.
- Build it: `cmake --build . -j4`

In summary:
```bash
git clone https://github.com/tim-gromeyer/Converter --depth=1 && cd Converter
git submodule update --init --recursive -j 3 --depth=1
mkdir build && cd build
cmake ..
cmake --build . -j4
```

## Credits

- The conversion from Markdown to HTML is done with the help of the [md4c](https://github.com/mity/md4c) - library by *Martin Mitáš*.
