# Instructions for the pd version of the Fluid.buf* algorithms



## How to start:

1) move the objects from pd_objects to a relevant pd search path

2) Instantiate objects as normal

3) Object names are fluidobjectname~, where objectname is the same as the max name post fluid. and with all lowercase.

4) Object arguments match those in max.

5) Parameters can be set by message (as in max with the same names) OR by option in the object box which work similarly to attributes in max, but you use "-" as a prefix and not "@"

#### Enjoy!


## Known issues:
- there is no help - use the max help system for now for parameter names etc.
- buffers don't work.
- fluidgain~ doesn't work because it doesn't allow pointer aliasing, which we cannot explictly avoid in pd.
- pd is single threaded.

