# It is in very early pre-alpha. I have barely gotten the simple functions like objects to work.

[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![C++ Version](https://img.shields.io/badge/C%2B%2B-C%2B%2B20-blue)](https://isocpp.org/)
[![OpenGL Version](https://img.shields.io/badge/OpenGL-3.3-orange)](https://www.opengl.org/)

---
## Getting Started

### Prerequisites

- C++20 compatible compiler
- CMake 3.20+
- OpenGL 3.3 capable GPU

### Installation

Clone and build:

`
git clone https://github.com/Desnio/Nionyx.git
cd Nionyx
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
`

## Licences

Every licence for the dependencies can be found in `third_party_licences/xxx/LICENCE`

## Folders

- `EngineAssets/` has all the required assets for the engine to function. The sphere is used to show where light objects are
- `cfg` has the config files. The modes are `Game`, `Engine` and `Debug`
  - `Game` is the final game that will run
  - `Engine` is the full editor gui
  - `Debug` is the same as `Game` but with more info about the player and objects
- `custominclude` has all the engines `.hpp` files
- `include` has external libraries' `.h/.hpp` files that vcpkg can't install
- `shaders` has all the shaders
- `src` source files
- `third_party_licences` all the external libraries' licences

## Running the game

To run it in `Game` mode you must do some stuff beforehand to make it work
- Compile NXPKPacker in `src/NXPK`
- Run `./NXPKPacker shaders Shaders.nxpk`
- Run `./NXPKPacker saves Saves.nxpk`
- Run `./NXPKPacker assets Assets.nxpk`
  - Note that you can specify the archive name for the asset folder in each object in the save.json
