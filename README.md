# Exploration of Compiler Design

This repository contains a custom compiler project aimed at exploring the basics of compiler design. It is meant as an organized space that would help me structure my work. The development environment is set up using **Docker Compose**, the project is written in C++20 and built using **CMake**.

This project is by no means a serious attempt at creating a powerful optimized compiler, and the made-up language it is targeted at isn't Turing complete. It is more of a joke-like motive to get myself to practice Docker, CMake, C++ new standards, assembler and basics of compiler design.

Steps of compiler development follow https://www.youtube.com/watch?v=vcSijrRsrY0&list=PLUDlas_Zy_qC7c5tCgTMYq2idyyT241qs

## Prerequisites

All development environment is organised in a Linux container that is set up with **Docker Compose**. So **Docker** is all prerequisites needed.

---

## Setup

This project uses Docker Compose to set up a Linux development environment with all the necessary dependencies. To start the environment:

```bash
docker-compose run --build --rm dev-environment
```

or (Windows):
```
.\scripts\docker-compose-run
```

This will:
- Build the Docker image as defined in the `Dockerfile`.
- Start the `dev-environment` service defined in `docker-compose.yml`.
- Bind mount the project dir to the container environment.
- Open a bash shell inside the development container, where you can start building and testing the compiler.

---

## Build and run

- **Simple run**:

    ```bash
    ./scripts/cmake-build.sh
    ./build/compile ./input/test.code
    ```

- **Debug**:

    ```bash
    ./scripts/cmake-debug.sh
    ./scripts/gdb-debug.sh
    ```

`input` directory has examples of code to compile.
