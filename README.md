# Fluid Corpus Manipulation: PD Objects Library

This repository hosts code for generating the Pure Data objects and documentation resources for the Fluid Corpus Manipulation Project. Much of the actual code that does the exciting stuff lives in this repository's principal dependency,  the [Fluid Corpus Manipulation Library](https://github.com/flucoma/flucoma-core). 

# Minimal Quick Build

Minimal build steps below. For detailed guidance see https://github.com/flucoma/flucoma-pd/wiki/Compiling

## Prerequisites 

* C++14 compliant compiler (clang, GCC or MSVC)
* cmake 
* make (or Ninja or XCode or VisualStudio)
* git 
* an internet connection 
* [PureData Source](https://github.com/pure-data/pure-data) and possibly `pd.lib` (for windows)

CMake will automatically download the dependencies needed

```bash
mkdir -p build && cd build
cmake -DPD_PATH=</path/to/api> ..
make install
```

This will assemble a package in `release-packaging`.

## Credits 
#### FluCoMa core development team (in alphabetical order)
Owen Green, Gerard Roma, Pierre Alexandre Tremblay

#### Other contributors (in alphabetical order):
James Bradbury, Francesco Cameli, Alex Harker, Ted Moore

> This project has received funding from the European Research Council (ERC) under the European Union's Horizon 2020 research and innovation programme (grant agreement No 725899).
