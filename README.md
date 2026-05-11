# project klein

![logo](assets-src/logo.png)

2D side-scroller platformer game using 2D raytraced portals to create confusing,
non-eucledian worlds

This project has been inspired significantly by
[AAAAXY](https://divverent.github.io/aaaaxy/) (by [divVerent](https://github.com/divVerent))

## Cloning and building

Important: If you obtained this source repository by cloning it from `git`,
make sure to initialize the git submodules first:

```bash
git submodule update --init --recursive
```

### Linux build instructions

On Linux, this project is built and developed *primarily* using Nix and CMake:

```bash
nix develop # or, use direnv: direnv allow
cmake -B build -G Ninja
ninja -C build
```

(Default configuration is `RelWithDebInfo`; to build in `Debug` mode pass
`-DCMAKE_BUILD_TYPE=Debug` to the `cmake` command)

### Windows build instructions

On Windows, the *only* officially supported path is Visual Studio's
built-in CMake support.\
Use CMakeLists.txt/CMakeSettings.json provided with the project.

- Make sure you have `git` installed in your `PATH` (system-wide) on your host machine,\
  it is required to configure the project (and is not a core part of VS2026)\
  (to quickly install it run `winget install Git.Git`)

## License

All code under src/ is licensed under the terms of the PolyForm Non-Commercial License,\
Check LICENSE.md for more details.

Some assets might be licensed under different terms.

## Third-party Asset Licenses

- **KMR Editor Icon Set** by komorra\
  License: [CC-BY 3.0](https://creativecommons.org/licenses/by/3.0/)

- **Free Prototype 2D Platformer 32×32 Pixel Tileset**\
  Source: <https://craftpix.net/freebies/free-prototype-2d-platformer-32x32-pixel-tileset/>\
  License: <https://craftpix.net/file-licenses/>

## Note

No AI/LLM agents, generated code or documentation has been *directly* used for
this project (and no such contributions will be made or accepted in the future)

(I have used Copilot auto-complete and LLM(s) to assist in debugging
during development though.)
