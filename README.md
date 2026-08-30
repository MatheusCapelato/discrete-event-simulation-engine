# Discrete Event Simulation Engine

This repository contains a high-performance Discrete Event Simulation (DES) engine written in C. It models a complex, resource-constrained geographical ecosystem over a continuous timeline of 525,600 minutes (one full year), managing entities, queues, and spatial routing.

The core of the engine is driven by a **Future Event List (LEF - Lista de Eventos Futuros)**, implemented as a strict Priority Queue.

## 🧠 Engineering Highlights

*   **Deterministic Event Loop:** The simulation avoids thread-sleeping or real-time polling. Instead, time jumps deterministically from event to event by popping the highest-priority struct from the LEF, ensuring maximum CPU utilization.
*   **Dynamic Memory & Pointer Arithmetic:** Heavy use of dynamic allocation (`malloc`/`free`) for spawning simulation entities (Heroes, Bases, Missions) and orchestrating lifecycle events. All memory operations are strictly audited to prevent leaks or Undefined Behavior (UB).
*   **Spatial & Resource Logic:** Implements Cartesian distance algorithms and dynamic queue-management. Entities possess basic AI to calculate waiting probabilities versus travel costs, deciding in real-time whether to queue for resources or reroute to different Cartesian coordinates.

## 🏗️ Architectural Note: The Monolithic Core

You will notice that the entirety of the simulation logic (state management, entity behavior, and the main event loop) is contained within a single file (`theboys.c`), rather than being split into multiple modular `.c` and `.h` headers for each entity type. 

**This is a deliberate architectural choice known as a Single Compilation Unit (or Amalgamation) for the core loop.** 

In high-frequency event simulations processing millions of cycles, cross-module function calls can introduce microscopic but compounding overhead. By keeping the core engine in a single translation unit, we allow the compiler to perform **aggressive function inlining** and structural optimizations across the entire event loop. The external dependencies are strictly limited to the abstract data types (Priority Queue, FIFO, and Sets).

## 🛠️ Tech Stack & Concepts

*   **Language:** C (C99 Standard)
*   **Data Structures:** Priority Queues, FIFO Queues, Dynamic Sets (Bitwise operations)
*   **Paradigms:** Discrete Event Simulation (DES), State Machines

## 🚀 How to Build and Run

This project includes a fully configured `makefile` to orchestrate the build process, applying strict compiler flags (`-Wall -Wextra -Werror -g`).

**To compile the simulation engine:**
```bash
make
```

**To run the simulation:**
```bash
make run
```

**To audit memory usage at runtime (Memory Leak Detection):**
```bash
make valgrind
```

**To clean build artifacts:**
```bash
make clean
