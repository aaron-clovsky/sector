# CD-ROM image sector library

## CREDIT

Based on CDRDAO.
 
## BUILD INSTRUCTIONS

- On Linux
  - Install a C compiler if one is not present
   (Run ```sudo apt install build-essential``` on Debian distros)
  - ```cd``` to this repo's directory
  - Run ```make```
- On Windows
  - Install VC++
  - Open the ```Native Tools Command Prompt```
  - ```cd``` to this repo's directory
  - Run ```make.bat```

## DESCRIPTION

TBD

## NOTES

- ```make style``` is implemented to keep code within style guidelines,
it requires clang-format-21, run the following to install (on Linux):
```
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 21
sudo apt install clang-format-21
```

- ```make lint``` is implemented (on Linux) to help check for warnings it
requires: gcc, g++, clang, clang++ and cppcheck

## LICENSE
This software is licensed under the [GPLv3](https://www.gnu.org/licenses/gpl-3.0.html).
