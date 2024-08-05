# Butos - a boot capable bare metal program 
Butos is a **bare metal program** that aspires to be a **framework** for other programs that want to run without an operating system directly in kernel mode. So the ultimate goal is to be able to make it possible to **run other bare metal software**.

![image](https://user-images.githubusercontent.com/71092108/207573430-31ec90b7-d47c-4c58-ac28-358f6acdce79.png)

## Project structure

```
Built with https://tree.nathanfriend.io/

butos/
├── init/
│   ├── include/
│   ├── libs/
│   └── mbr/
└── src/
    ├── boot/
    │   └── boot
    ├── cpu/
    ├── drivers/
    ├── fs/
    ├── include/
    ├── libs/
    └── butos
```

- `init`: contains the real mode **bootloader** for the *second stage butos bootloader*
    - `include`: contains **macros** for the gnu-as assembler with the goal of making assembly code more readable
    - `libs`: contains **assembly libraries** with useful *functions* for bootloader operation
    - `mbr`: contains **Master Boot Record data**
- `src`: contains the C code of the 32-bit **butos kernel**
    - `include`: contains some **macros** which can be useful throughout the project
    - `boot`: contains the **second stage bootloader** which start the kernel
    - `fs`: contains the **filesystem libraries**
    - `butos`: contains the **binary** executable of butos
    - `cpu`: contains **drivers** to handle interrupts and other **cpu features**
    - `drivers`: contains **drivers** for **hardware** components accessible via the system bus
    - `libs`: contains the implementation of globally useful **libraries** in the project

Note that the headers are placed in the *same folder* as the source files.

## Documentation
The `doc/` folder should present the same structure shown before and should contain a `filename.doc` file for each `file.s`, `file.c` and `file.h` in order to make more usable this codebase. At the moment no documentation file has been implemented.

## Scripts
Writing a bare metal program can be very repetitive. The `script` folder contains useful scripts for automating some jobs. Some are no longer needed, others are still useful at compile time.
