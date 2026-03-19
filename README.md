# This is a student game engine that I'm developing in my free time

## Functions for each folder

The EngineAssets folder contains all assets required by the engine to function, right now its only the sphere used for lighting placement.

cfg.json has one function for now and that is what mode to run the game in. There are 3 modes.
- "Engine" is the full editor interface.
- "Debug" is the same as the normal game but gives a UI for debugging info.
- "Game" is the final game

custominclude/ has all the engines .hpp files\
include/ has all the external libraries' .h/.hpp files that vcpkg cant install

shaders/ has all the shaders in the engine

src/ has the source files

It uses some files from LearnOpenGL learnopengl.com

## Installation

I uses cmake and vcpkg to manage installation\
vcpkg install most of the required dependencies but imgui needs to be compiled with stdlib added on and glad also needs to be added