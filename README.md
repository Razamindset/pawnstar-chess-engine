# PawnStar Chess Engine

PawnStar is a chess engine written in C++ that uses the [chess-library](https://github.com/Disservin/chess-library) for board operations and move validation.

## Building the Project

### Prerequisites

- CMake (version 3.15 or higher)
- A C++ compiler that supports C++17
- Git (optional, only needed if you want to clone the repository)

### Build Instructions

1. Clone the repository (if you haven't already):

```sh
git clone https://github.com/Razamindset/pawnstar-chess-engine
cd pawnstar-chess-engine
```

2. Create a build directory and navigate to it:

```sh
mkdir build
cd build
```

3. Configure and build the project:

```sh
cmake ..
cmake --build .
```

The executable will be created in the `build` directory.

## Running the Engine

After building, you can run the engine from the build directory:

```sh
./pawnstar
```

## Future Plans

- Beat stockfish

Stay tuned for updates!
