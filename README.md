## Simple CMake template for [rayfork][rayfork-link] using [sokol-app][sokol-app-link]

##### This template works best when used with an IDE that supports cmake projects such as:
- [Visual Studio][visual-studio-link]
- [CLion][clion-link]
- [QtCreator][qtcreator-link]
- [Visual Studio Code][vscode-link] with [CMake plugins][vscode-cmake-plugins]

In any of these IDEs you should be able to just run the `your-project-name-goes-here` target and it will work.

##### Notes:
- Bear in mind that rayfork is still under development and not officially released yet, you can use this template to experiment with rayfork if you are curious until it gets officially released. Once rayfork will be stable the `libs/rayfork` folder will be always updated with the latest release of rayfork, until then updates might be sparse.

- The CMake file will also setup an `ASSETS_PATH` macro that will be an absolute path to the assets folder on your computer, if you wanna share the executable with other people then check lines 17 and 18 in the cmake file.

- The gitignore file of this template works by first ignoring all files using `*` and then allowing specific files/folders like `!file` `!folder` `!folder/**`. If you don't like this setup feel free to change the gitignore, but if you keep it be aware in case you add new folders, or files in the root of the project. 

- Feel free to delete this readme.

##### This template uses: 
- [rayfork][rayfork-link] (the amalgamated version).
- [sokol-app][sokol-app-link] for the platform layer.
- [sokol-time][sokol-time-link] for high resolution timing.
- [glad][glad-link] for OpenGL loading.

<!-- Links -->
[rayfork-link]:    https://github.com/SasLuca/rayfork/tree/rayfork-0.9
[sokol-app-link]:  https://github.com/floooh/sokol/blob/master/sokol_app.h
[sokol-time-link]: https://github.com/floooh/sokol/blob/master/sokol_time.h
[glad-link]:       https://glad.dav1d.de/

[visual-studio-link]:   https://visualstudio.microsoft.com/vs/
[vscode-link]:          https://code.visualstudio.com/
[qtcreator-link]:       https://www.qt.io/product/development-tools
[clion-link]:           https://www.jetbrains.com/clion/
[vscode-cmake-plugins]: https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools
