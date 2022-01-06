#!/usr/bin/env bash

set -eux
rm -rfv build
mkdir build
cd build
cmake ..
make
sudo cp justoj-core-client   /usr/bin/justoj-core-client
sudo cp justoj-cpu-benchmark /usr/bin/justoj-cpu-benchmark