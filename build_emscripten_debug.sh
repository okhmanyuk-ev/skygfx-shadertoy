mkdir build

emcmake cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
