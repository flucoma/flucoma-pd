# Fluid Corpus Manipulation: PD Objects Library

This repository hosts code for generating the Pure Data objects and documentation resources for the Fluid Corpus Manipulation Project. Much of the actual code that does the exciting stuff lives in this repository's principal dependency,  the [Fluid Corpus Manipulation Library](). What lives here are: 

* A wrapper from our code to the PD API, that allows us to generate PD objects from a generic class. 
* Stubs for producing PD objects for each 'client' in the Fluid Corpus Manipulation Library. 
* CMake code for managing dependencies, building and packaging. 

## I'm in a Hurry...

...and you already have a development environment set up, understand CMake, and have the PD API (`m_pd.h`) available? And Python 3 + DocUtils + Jinja if you want docs? 

Cool: 

```
cmake -DPD_PATH=<location of your PD tree(see Dependencies below)> -DDOCS=<ON/OFF> ..
make 
```

Running `make install` will assemble a package in `release-packaging`. Changing the CMake variable `CMAKE_INSTALL_PREFIX` when you run CMake can alter this default location, if you'd rather install straight into your PD extensions folder. 

## Pre-requisites

* [CMake](http://cmake.org) >= 3.11
* A C++ 14 compliant compiler for Mac or Windows (via XCode tools on Mac;  Visual Studio 17 >= 15.9 on Windows; GCC or clang on Linux)

## Dependencies 

### PD API 
this is the only dependency we don't (optionally) manage for you, so there must be a version available to point to when you run CMake, using the CMake Variable `PD_PATH`

To build on all platforms, we need `m_pd.h` (the Pure Data API). Additionally, on Windows, we need `pd.lib` to link against. The variable `PD_PATH` should point to the top-level of either 
* a PD source-tree (Linux and Mac)
* a PD installation (Windows – because we need the compiled `.lib` for linking)
* The `Contents/Resources` folder of a Mac installation (i.e. `pd-<version>.app`), as this contains `m_pd.h` in the expected sub-folder. 

### Others 
These will be downloaded and configured automatically, unless you pass CMake a source code location on disk for each (see below): 
* [Fluid Corpus Manipulation Library]()
* [Eigen](https://gitlab.com/libeigen/eigen) (3.3.5)
* [HISSTools Library](https://github.com/AlexHarker/HISSTools_Library)

Unless you are simultaneously committing changes to these dependencies (or are *seriously* worried about disk space), there is nothing to be gained by pointing to external copies, and the easiest thing to is let CMake handle it all. 

## Building 

Simplest possible build: 
1. Locate `m_pd.h` if you haven't already
2. Clone this repo
```
git clone <whereis this>
```
3. Change to the directory for this repo, make a build directory, and run CMake, passing in the location for PD
```
mkdir -p build && cd build 
cmake -DMAX_SDK_PATH=<location of your PD install – see Dependencies above> ..
```
At this point, CMake will set up your tool chain (i.e. look for compilers), and download all the dependencies. 

An alternative to setting up / running CMake directly on the command line is to install the CMake GUI, or use to use the curses gui `ccmake`.

With CMake you have a choice of which build system you use. 
* The default on Mac is `Unix Makefiles`, but you can use Xcode by passing `-GXcode` to CMake when you first run it. 
* The default on Windows is the latest version of Visual Studio installed. However, Visual Studio can open CMake files directly as projects, which has some upsides. When used this way, CMake variables have to be set via a JSON file that MSVC will use to configure CMake. 
* The default on Linux is also `Unix Makefiles`

Another option is to install the build system `Ninja`, which can result in quicker builds. 

When using `make`, then
```
make
```
will compile all objects ('targets', in CMake-ish). Alternatively, in Xcode or Visual Studio, running 'build' will (by default) build all objects. Note that these IDEs allow you to build both for debugging and for release, whereas with Makefiles, you need to re-run CMake and pass a `CMAKE_CONFIG` variable for different build types.


```
make install 
```
Will assemble a PD package in `release-packaging`. 

## Generating Documentation 

Pre-requisites: 
* Python 3 
* Docutils python package (ReST parsing)
* Jinja python package (template engine)

To generate html documentation for the objects requires a further dependency, [flucoma_paramdump](), which we use to combine generated and human-written docs. We then pass `DOCS=ON` to CMake
```
cmake -DDOCS=ON ..
```
Unless we pass the location on disk of the `flucoma_paramdump`, CMake will again take care of downloading this dependency.

This process 
* has only ever been tested on Mac, so may well not work at all on Windows 
* can sometimes produce spurious warnings in Xcode, but *should* work (we use an anaconda environment here, and CMake gets the location of Python 3 via FindProgram; so long as there isn't anything too whacky in your `PATH`, all should be well)

## Development: Manual Dependencies 

If you're making changes to the Fluid Corpus Manipulation Library and testing against PD (and, hopefully, our other deployment environments), then it makes sense to have the source for this cloned somewhere else on your disk, so that you can commit and push changes, and ensure that they work in all environments. This would be the case, e.g., if you're developing a new client. 

To bypass the automatic cloning of the Fluid Corpus Manipulation Library, we pass in the cache variable `FLUID_PATH`

```
cmake -DPD_PATH=<location of your PD install> -DFLUID_PATH=<location of Fluid Corpus Manipulation Library> ..
```
Note 
1. You don't need to run CMake on the Fluid Corpus Manipulation Library first (well, you can, but it doesn't make any difference!). CMake's FetchContent facility will grab the targets from there, but won't look in its CMakeCache, so there should never be a conflict between the state of a build tree at `FLUID_PATH` and your build-tree here. 
2. It is **up to you** to make sure the commits you have checked out in each repository make sense together. We use tags against release versions on the `master` branch, so – at the very least – these should line up (unless you're tracking down some regression bug or similar). In general, there is no guarantee, or likelihood, that mismatched tags will build or run, as architectural changes can, do, will happen...

To include a manually checked out location of `flucoma_paramdump` (e.g. for debugging documentation generation), the same steps and caveats apply, but the variable is now `FLUID_PARAMDUMP_PATH`: 
```
cmake -DPD_PATH=<location of your PD install> -DFLUID_PARAMDUMP_PATH=<location of flucoma_paramdump utilities> ..
```
### Dependencies of dependencies! 
The same steps and considerations apply to manually managing the dependencies of the Fluid Corpus Manipulation Library itself. If these aren't explicitly passed whilst running CMake against this build tree, then CMake will download them itself against the tags / commits we develop against. Nevertheless, if you are in the process of making changes to these libraries and running against this (which is much less likely than above), then the CMake variables of interest are: 
* `EIGEN_PATH` pointing to the location of Eigen on disk 
* `HISSTOOLS_PATH` pointing to the location of the HISSTools Library 

To find out which branches / tags / commits of these we use, look in the top level `CMakeLists.txt` of the  Fluid Corpus Manipulation Library for the `FetchContent_Declare` statements for each dependency. 
