# CEP-simulation

A class structure to read out and analyse a Pythia-simulation done with the MBR model. The class either reads from
a `Kinematic.root` file (created by Alice simulations) which stores events in a directory structure,
or from a standard pythia event TTree. The EventHandler then only deals with looping through the individual events
that the base class (ReadPythiaTree of ReadDirectoryTree) have provided. Here we can save event information or 
particle information via EventInitializer and ParticleInitializer member functions.

## Root-prepartion
To actually use the framework `root` has to be linked with `Pythia`, *i.e.* the shared library libEGPythia 
has to be compiled during the build process of root. A guide on how to do this can be found in this repository.

If `Pythia` is linked with root we can now try to implement this framework within root. We first have to compile
the librarie files (.so files). This can be done by first setting the paths to the src and include files but the
easiest method is just to put all `*.h` and `*.cxx` files into one folder and *e.g.* for the EventHander 
call within root:
```
.L EventHandler.cxx++
```
This creates a EventHandler_cxx.so file where all the important information is stored for root to use the
EventHandler class.
As we have to do this every time when we enter root we can automate this process by putting some lines in the
`~/rootlogon.C` file which is called every time root is called.
We first let root know where it can find these classes and the corresponding header file in the include directory.
The files are saved inside the folder `/path/to/folder' and `/path/to/include`
```
{
// this is copied inide the ~/rootlogon.C file
gInterpreter->AddIncludePath("/path/to/include");
evtClassDir      = TString("/path/to/src/");
```
The files are loaded automatically by the next lines:
```
load_string      = TString(".L ");
dirTree_string   = TString("ReadDirectoryTree.cxx++");
pytTree_string   = TString("ReadPythiaTree.cxx++");
evtH_string      = TString("EventHandler.cxx++");
histmaker_string = TString("THistMaker.cxx++");
gInterpreter->ProcessLine((load_string+evtClassDir+dirTree_string).Data());
gInterpreter->ProcessLine((load_string+evtClassDir+pytTree_string).Data());
gInterpreter->ProcessLine((load_string+evtClassDir+evtH_string).Data());
gInterpreter->ProcessLine((load_string+evtClassDir+histmaker_string).Data());
}
```

Now each time we enter root the libraries are complied. This takes a few seconds.
