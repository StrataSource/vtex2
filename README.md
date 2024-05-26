# vtex2

vtex2 is a Valve Texture Format conversion and creation tool. It has a CLI and a GUI component for viewing, packing and otherwise converting the files.

## Usage

Command help documentation and usage examples can be shown on the command line using `vtex2 --help`.

For action-specific help, use `vtex2 <action> --help`.

### Creating VTFs

Creating a VTF can be done with the `vtex2 convert` action.

For example, the following command will create a VTF called `some-file.vtf` with the format `BGRA8888`:
```
vtex2 convert -f bgra8888 some-file.jpg
```

If you pass a directory to `vtex2 convert`, it will convert all files in that directory. The `-r` or `--recursive` parameter
will cause the program to descend and process subdirectories too.

Full list of options:
```
USAGE: vtex2 convert [OPTIONS] file...

  Convert a generic image file to VTF

Options:
  --bumpscale          Bumpscale
  --clamps             Clamp on S axis
  --clampt             Clamp on T axis
  --clampu             Clamp on U axis
  --gamma-correct      Apply gamma correction
  --pointsample        Set point sampling method
  --srgb               Process this image in sRGB color space
  --start-frame        Animation frame to start on
  --thumbnail          Generate thumbnail for the image
  --trilinear          Set trilinear sampling method
  --version            Set the VTF version to use
  -c,--compress [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]
                       DEFLATE compression level to use. 0=none, 9=max. This will force VTF version to 7.6
  -f,--format [rgba8888, abgr8888, rgb888, bgr888, rgb565, i8, ia88, p8, a8, rgb888_bluescreen, bgr888_bluescreen, argb8888, bgra8888, dxt1, dxt3, dxt5, bgrx8888, bgr565, bgrx5551, bgra4444, dxt1_onebitalpha, bgra5551, uv88, uvwq8888, rgba16161616f, rgba16161616, uvlx8888, r32f, rgb323232f, rgba32323232f, ati2n, ati1n]
                       Image format of the VTF
  -m,--mips            Number of mips to generate
  -n,--normal          Create a normal map
  -o,--output          Name of the output VTF
  -r,--recursive       Recursively process directories
  file                 Image file to convert
```

### Extracting image data from VTF

Extracting image data from a VTF can be done using `vtex2 extract`.

For example, the following command will extract the VTF to `some-file.jpg`:
```
vtex2 extract -f jpg some-file.vtf
```

If you pass a directory to `vtex2 extract`, it will convert all files in that directory. The `-r` or `--recursive` parameter
will cause the program to descend and process subdirectories too.

Full list of options:
```
USAGE: vtex2 extract [OPTIONS] file...

  Converts a VTF into png, tga, jpeg, bmp or hdr image file

Options:
  -f,--format [png, jpeg, jpg, tga, bmp, hdr]
                       Output format to use
  -m,--mip             Mipmap to extract from image
  -na,--no-alpha       Exclude alpha channel from converted image
  -o,--output          File to place the output in
  -r,--recursive       Recursively process directories
  file                 VTF file to convert
```

### Displaying VTF Info

The `vtex2 info` command can be used to display some info about VTF files.

Full list of options:
```
USAGE: vtex2 info [OPTIONS] file...

  Displays info about a VTF file

Options:
  -a,--all             Display all detailed info about a VTF
  -r,--resources       List all resource entries in the VTF
  file                 VTF file to process
```

## Building 

The first step is to clone the repository. Make sure to do a recursive clone!

```
git clone https://github.com/StrataSource/vtex2.git --recursive
```

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
