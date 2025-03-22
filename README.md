# csg-2025-assembly

<div align="center">
  Compétition d'assembleur pour les Computer Science Games 2025.
  <br />
  <br />
  <a href="./docs/fr/main.pdf">Documentation (fr)</a>
  <span>&nbsp;&nbsp;•&nbsp;&nbsp;</span>
  <a href="./docs/en/main.pdf">Documentation (en)</a>
</div>

## Prerequisites
Before building the project, ensure you have the following installed:
- [CMake](https://cmake.org/)
- [Ncurses](https://invisible-island.net/ncurses/)

```sh 
sudo apt-get install cmake libncurses5-dev
```

## Build instructions
To compile the project and generate the executable `./asm`, run the 
following commands : 

```sh
cmake -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=.. -B build 
cmake --build build
```
This will configure the build directory and compile the project.

