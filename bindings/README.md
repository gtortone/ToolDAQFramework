## Python bindings for DAQInterface

To add Python bindings a separate CMakeLists.txt is provided.

### Build for ARMhf

```
cmake -B build-armhf -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-arm-linux-gnueabihf.cmake \
  -DDAQ_INCLUDE_DIR=../build-armhf/include  -DDAQ_LIB_DIR=../build-armhf/lib -DCROSS_TARGET=arm-linux-gnueabihf

make -j -C build-armhf
```

### Build for ARM64
```
cmake -B build-arm64 -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-aarch64-linux-gnu.cmake \
  -DDAQ_INCLUDE_DIR=../build-arm64/include  -DDAQ_LIB_DIR=../build-arm64/lib -DCROSS_TARGET=aarch64-linux-gnu

make -j -C build-arm64
```

### Installation

Copy .so file from build directory in `/usr/lib/python<version>/dist-packages


### Usage example

```
python3 -c "import daqinterface; s = daqinterface.ToolFramework.Store(); s.SetInt('value', 42) ; s.Print()"
```
