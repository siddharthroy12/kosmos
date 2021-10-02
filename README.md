## Kosmos

A bullet-hell shooter game made in C99 for my college project.

![Gameplay](./preview/gameplay.gif)


## Building

### Linux

#### Install requied libraries

**Ubuntu**
```bash
sudo apt install libasound2-dev mesa-common-dev libx11-dev libxrandr-dev libxi-dev xorg-dev libgl1-mesa-dev libglu1-mesa-dev

```

#### Compiling
```bash
mkdir build

cd build

cmake ..

cmake --build .

./kosmos
```

#### Cross Compiling to Window
```bash
mkdir build

cd build

cmake -DCMAKE_TOOLCHAIN_FILE=../mingw.cmake ..

make

wine kosmos.exe
```

### Window

Install cmake and a C compiler if you haven't already.

#### With MinGW
```bash
mkdir build

cd build

cmake .. -G "MinGW"

cmake --build .

# kosmos.exe should appear in the build folder
```

### MacOS
```bash
mkdir build

cd build

cmake ..

cmake --build .

./kosmos
```