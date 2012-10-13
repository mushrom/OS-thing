###obsidian os
- - - - -
This is a small OS project, at the moment consisting of barebones kernel. It's intended to be yet another posix clone, for educational purposes. 

To build it, you'll need a C compiler, nasm, and a unix-like environment (for image generation.)
to build:
    make cross-cc #if you don't already have a cross compiler set up
    make
    make test

