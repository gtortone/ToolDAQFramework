# ToolDAQFramework

## Build

```
cmake -B build
make -j -C build
```

CMakeLists.txt contains function to autodetect ZMQ and BOOST libaries if available on the host.

## Build options for CMake

```-DBOOST_ROOT=/path/boost```: set BOOST library path

```-DZMQ_ROOT=/path/zmq```: set ZMQ library path

## Cross build

```
cmake -B build-arm -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-linux-gnueabihf.cmake 

make -j -C build-arm
```

## Output products

- dynamic libraries (*.so) in `<build_dir>/lib`
- static libraries (*.a) in `<build_dir>/lib`
- header files (*.h) in `<build_dir>/include`
- test executable (main) in `<build_dir>/bin`
- config files `<build_dir>/bin/configfiles`
