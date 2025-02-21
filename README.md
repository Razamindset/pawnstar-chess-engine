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

## Development Status

Currently implemented:

- [x] Negamax search with alpha-beta
- [x] Quiescence search
- [x] Basic evaluation function
- [x] MVV-LVA move ordering

Under development:

- [ ] Advanced evaluation features
- [ ] Improved move ordering
- [ ] Time management
- [ ] Opening book support
- [ ] Transposition tables

## Future Plans

- Beat stockfish

## License

This project is licensed under the MIT License with an attribution requirement - see the [LICENSE](LICENSE) file for details.

When using this code or substantial portions of it in your project, please include attribution:

```
Contains code from PawnStar Chess Engine (https://github.com/Razamindset/pawnstar-chess-engine)
```

## Attribution Notices

PawnStar Chess Engine uses:

- [chess-library](https://github.com/Disservin/chess-library) for board representation
- Techniques and algorithms from [Chess Programming Wiki](https://www.chessprogramming.org)

Stay tuned for updates!
