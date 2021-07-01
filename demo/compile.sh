#!/bin/bash

g++ demo-sequential.cpp \
-std=c++17 \
-O3 -march=native -funroll-loops \
-Wall -Wextra -Wpedantic \
-o demo-sequential

g++ demo-parallel.cpp \
-std=c++17 -fopenmp -latomic \
-O3 -march=native -funroll-loops \
-Wall -Wextra -Wpedantic \
-o demo-parallel

