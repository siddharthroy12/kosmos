## Kosmos

> **NOTE:** Still is development

## Building
```
mkdir build
cd build
cmake .. # or cmake .. -G "MinGW Makefiles" for windows with mingw
cmake --build .
```

## Cross Compilation From Linux to Windows
```
mkdir build
cd build
cmake -DBUILD_SHARED_LIBS=OFF  -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc -DCMAKE_RC_COMPILER="$(which x86_64-w64-mingw32-windres)"  -DCMAKE_FIND_ROOT_PATH=/usr/x86_64-w64-mingw32 -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY -DCMAKE_INSTALL_PREFIX=../install-win ..
make
```

##### This game uses: 
- [rayfork][rayfork-link] (the amalgamated version).
- [sokol-app][sokol-app-link] for the platform layer.
- [sokol-time][sokol-time-link] for high resolution timing.
- [sokol-audio][sokol-audio-link] for audio.
- [glad][glad-link] for OpenGL loading.

<!-- Links -->
[rayfork-link]:    		https://github.com/SasLuca/rayfork/tree/rayfork-0.9
[sokol-app-link]:  		https://github.com/floooh/sokol/blob/master/sokol_app.h
[sokol-time-link]: 		https://github.com/floooh/sokol/blob/master/sokol_time.h
[glad-link]:       		https://glad.dav1d.de/
[sokol-audio-link]:		https://github.com/floooh/sokol/blob/master/sokol_audio.h