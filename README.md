# FAT32_emulator
This project implements a FAT32 file system emulator in C.  
It provides a command-line interface (CLI) for interacting with a FAT32 file system stored within a regular file.  
The emulator can manage directories and files, allowing for operations such as formatting the file system, listing directory contents, changing directories, and creating files and directories.  
The emulator works with a file that simulates a FAT32 file system, and it can create a new file if one does not already exist.


### Features
- Create FAT32 File: If the FAT32 file does not exist, a 20 MB file will be created.
- Command Line Interface (CLI): Interact with the file system through commands.
  - `format`: Create a new FAT32 file system inside the file (erases all data).
  - `ls [path]`: List files and directories inside the specified directory (or the current directory if not specified).
  - `cd <path>`: Change to the specified directory.
  - `touch <name>`: Create an empty file with the specified name.
  - `mkdir <name>`: Create a new directory with the specified name.
- Error Handling: Displays error messages when commands are used incorrectly or the file system is not valid.


## Build

### Prerequisites
Make sure you have the following tools and libraries installed:
- [GCC](https://gcc.gnu.org) (GNU Compiler Collection) or any C compiler
- [Make](https://www.gnu.org/software/make/) (build automation tool)


### Instructions
1. **Clone the repository:**
   ```base
   git clone https://github.com/formodx/FAT32_emulator.git
   cd FAT32_emulator
   ```


2. **Build the project:**
   ```bash
   make
   ```


### Clean the build (optional)
If you want to clean up the compiled files, you can use:
```bash
make clean
```


Example
-------
```bash
$ ./fat32_emulator disk.img
```
