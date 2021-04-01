#!/bin/bash

(mkdir -p build; cd build; meson .. && ninja) && ./build/gears
