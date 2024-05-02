#!/usr/bin/env bash

gcc -g -o lsystem lsystem.c `pkg-config --cflags --libs cairo` -lm

gdb ./lsystem
