## Kosmos

> **NOTE:** Still is development

## Building
```bash
mkdir build
cd build
cmake .. # or cmake .. -G "MinGW Makefiles" for windows with mingw
cmake --build .
```

## Cross Compilation From Linux to Windows
```bash
mkdir build
cd build
cmake -DBUILD_SHARED_LIBS=OFF  -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc -DCMAKE_RC_COMPILER="$(which x86_64-w64-mingw32-windres)"  -DCMAKE_FIND_ROOT_PATH=/usr/x86_64-w64-mingw32 -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY -DCMAKE_INSTALL_PREFIX=../install-win ..
make
```