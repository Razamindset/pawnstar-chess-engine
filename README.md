# PawnStar Chess Engine

PawnStar is a chess engine written in C++ that originally aimed to implement board representation, move generation, and validation from scratch. However, to avoid reinventing the wheel, we now use the [chess-library](https://github.com/Disservin/chess-library) to handle board operations and move validation.

## Setup

```sh
git clone https://github.com/Razamindset/pawnstar-chess-engine
cd pawnstar-chess-engine/
```

To use this project, clone the `chess-library` into an `external/` directory within your project:

```sh
mkdir -p external
cd external
git clone https://github.com/Disservin/chess-library.git
cd ..
```

## Compilation

To compile PawnStar using `g++`, on linux run the following command:

```sh
g++ -I external/chess-library/include src/main.cpp -o pawnstar.out
```

Replace `src/main.cpp` with the actual source file containing your `main` function.

## Running the Engine

After compilation, you can run the engine with:

```sh
./pawnstar.out
```

## Future Plans

- Beat stockfish

Stay tuned for updates!
