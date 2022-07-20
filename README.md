# vtex2

vtex2 is a Valve Texture Format conversion and creation tool. It has a CLI and a GUI component for viewing, packing and otherwise converting the files.

## Building 

### Linux

Required packages:
* qtbase5-dev (Optional, only for GUI)
* cmake
* make or ninja

```sh
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Windows

#### Packages
You must install Qt5 (Or 6, the viewer may work with that too). 
Make note of the install directory.

CMake and Visual Studio 2022 or 2019 are required.

#### Building

(From powershell)
```
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -DQT_BASEDIR=C:\path\to\my\Qt5
```

You can then open `build\vtex2.sln` in Visual Studio and compile from there.
