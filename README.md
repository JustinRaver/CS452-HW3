# Homework #1: Execution Integrity

* Author: Justin Raver
* Class: CS452 
* Semester: Fall 2021

## Overview

This project uses the prebuilt functionality of creating a lawn, mole_new, and mole_whack
to produce a small gui that produces N moles, consumes or "whacks" N moles, and terminates.

## Implementation

I expanded on the previous functionality by utilizing my pre-built queue implementation to store the
moles. I then multi threaded the produce and consume functionality so that each function call
utilizes and independent thread. In order to ensure thread safety I created a thread safe wrapper 
for my queue implementation and and locking and unlocking to ensure there were no race conditions 
or thread starvation. 

## Compiling and Using

From directory with all c files, Mastermakefile and GNUmakefile:

build and run / test: make run

valgrind report: make valgrind

## Valgrind Report

