# Install ROOT with PYTHIA
1. Download and unpack the ROOT's sources from the 
[download area](https://root.cern.ch/downloading-root).
We need the source version and not pre-builded binary version as we are going to link some libraries
during the build process.
Extract the downloaded file with `tar -zxf root_<version>.source.tar.gz`
2. Create a directory for containing the build. It is not supported to build ROOT on the 
source directory. `cd` to this directory 
    ```
    mkdir root_<version>_build
    cd root_<version>_build
    ```

3. Execute the cmake command on the shell replacing `path/to/source` with the path to the top of 
your ROOT source tree (the extracted `root_<version>.source.tar.gz`). Be sure before you invoke 
`cmake` that you have set the `PYTHIA8` and `PYTHIA8DATA` environment variables correctly
(in your ~/.bashrc) so that `cmake` can pick them up and we dont have to do this by ourselfs later
on.
   ```
   cmake path/to/source
   ```
   CMake will *hopefully* (=if you have set your environment variables correctly) detect your 
development environment, perform a series of test and generate the files required for building 
ROOT. CMake will use default values for all build parameters. 

4. We now open the `CMakeCache.txt` file and look if `cmake` picked up the following important 
enviroment variables.
   ```
   PYTHIA8_INCLUDE_DIR
   PYTHIA8_LIBRARY
   ```
   The include_dir  is looking for a path to an include directory (*duh*), it is where find 
`Pythia8/Pythia.h`. The library path should point to this file `libpythia8.so`. If any of those two
say something alone `PYTHIA8_LIBRARY-NOTFOUND` we have to go to `~/.bashrc` and fix the path variable
`$PYTHIA8` remove all files created in the build folder and run `cmake path/to/source` again.

5. If the paths are all set we can start the build from the build directory 
(in `root_<version>_build`)
   ```
   cmake --build . -- -jN
   ```
   This will take some time.

6. After the installation is complete we want to make sure we can execute root from wherever 
we are. This can be done by setting up the environment by putting the following lines in the `~/.bashrc` file
   ```
   source root_<version>_build/bin/thisroot.sh
   alias root='root -l'
   ```
