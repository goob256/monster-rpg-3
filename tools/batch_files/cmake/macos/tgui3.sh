#!/bin/sh
DEP=$1
shift
cmake .. -DCMAKE_BUILD_TYPE=Release -DUSER_INCLUDE_PATH="/Users/trent/code/libs/mac/include/SDL2;/Users/trent/code/libs/mac/include" -DUSER_LIBRARY_PATH=/Users/trent/code/libs/mac/lib -DCMAKE_OSX_DEPLOYMENT_TARGET=$DEP -DCMAKE_MODULE_PATH=/Users/trent/code/Monster_RPG_3/tools/batch_files/cmake/macos -DSHARED=off $*
