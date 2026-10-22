
# Vole Machine Simulator — Initial Design

## Overview
This document describes the initial object-oriented design of the Vole Machine Simulator.

## Classes and Responsibilities

- **VoleMachine:** Coordinates program loading, execution, stepping, resetting, and halting.
- **RegisterFile:** Stores and provides access to the 16 general-purpose registers.
- **Memory:** Stores program instructions and data, and provides memory read/write operations.
- **ControlUnit:** Fetches, validates, decodes, and executes instructions while updating the program counter.
- **ALU:** Performs integer addition, floating-point addition, bitwise operations, and rotation.
- **Instruction:** Represents a decoded instruction and its operands.
- **InputOutput:** Handles input and output operations.

## Relationships

- VoleMachine owns RegisterFile, Memory, ControlUnit, and InputOutput.
- ControlUnit fetches instructions from Memory.
- ControlUnit reads from and writes to RegisterFile.
- ControlUnit requests operations from ALU.
- ControlUnit decodes and validates Instruction objects.
- ControlUnit communicates with InputOutput when needed.

## UML Diagram

See `VoleMachine.png` for the diagram and `VoleMachine.puml` for its source code.

## Design Status

This is the initial design and may be refined after comparing the designs produced by three AI tools.

