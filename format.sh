#!/bin/bash

clang-format -i -style='{BasedOnStyle: Google, IndentWidth: 4}' include/*.h
clang-format -i -style='{BasedOnStyle: Google, IndentWidth: 4}' src/*.cpp

