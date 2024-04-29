#!/usr/bin/env bash

gcc -o lsystem lsystem.c `pkg-config --cflags --libs cairo` -lm
