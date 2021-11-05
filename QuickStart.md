# Instructions for the Pure Data version of the Fluid Corpus Manipulation toolbox

## How to start:

> Step by step installation instructions can be found at https://learn.flucoma.org/installation/pd/

1. Put the FluidCorpusManipulation folder where you see fit

2. Add this folder to your path preferences, or use the `[declare]` object in your patch.

3. In your startup preferences add `fluid_libmanipulation` to 'Pd libraries to load on startip': this is for `fluid.dataset` and friends. 

4. The Pure Data document 'Fluid_Decomposition_Overview.pd' shows the toolbox at a glance. Help is available for every objects and there is the /examples folder for further demonstration of the various usages.

5. More detailed documentation is provided as HTML pages in /docs

6. Parameters can be set by message OR by option in the object box, which work similarly to the `[sigmund~]` options.

7. Most objects working on arrays/buffers are multichannel.
>  The toolbox uses the following convention: a named `array` is expected to have a name, followed by `-x` where `x` is the 'channel' number, 0-indexed. For instance, a stereo source buffer defined as 'mybuf' will expect an array named `mybuf-0` for the left channel, and an array named `mybuf-1` for the right channel. A utility `[multiarray.pd]` is used throughout the help files in conjonction with the `[clone]` to programmatically generate such 'multichannel' buffers.

#### Enjoy!


## Known issues:
- Pd is single threaded so doing buffer ops will do bad things to realtime audio if not using the -blocking 0 option - please see the documentation

> This project has received funding from the European Research Council (ERC) under the European Union’s Horizon 2020 research and innovation programme (grant agreement No 725899).
