#!/bin/sh

mpicc -o crack main.c utils.c rank0.c md5.c ranki.c -lm
