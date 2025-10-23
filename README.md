# Computer graphic project for drawing 3D shapes to aid learning 3D geometry.

Some images here hopefully.

# Steps to build:

Clone the project:
```
git clone --recurse-submodules https://github.com/HTuanPhong/Computer-Graphic-HCMUS.git
```
Go in the repo directory, create build folder and Cmake build the project:
```
cd Computer-Graphic-HCMUS
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```
At this point you can just use your build system to build but we can continue with cmake:
```
cmake --build . --config Release
```
The binary should now be some where in build folder depend on your build system.


# Steps to dev:

The same as steps to build but on the last step use:
```
cmake --build . --target run-cg
```
This will build and run the binary saving dev times.