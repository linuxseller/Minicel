#!/bin/bash

set -xe

clang main.c
./a.out test.mcell
