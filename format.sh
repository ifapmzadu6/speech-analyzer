#!/bin/bash

clang-format -i -style='{BasedOnStyle: Google, IndentWidth: 4, ColumnLimit: 300}' include/*.h
clang-format -i -style='{BasedOnStyle: Google, IndentWidth: 4, ColumnLimit: 300}' src/*.cpp

