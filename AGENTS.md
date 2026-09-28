# Domino

Jogo de dominó em duplas em C (raylib). Regras em `domino_core` (sem raylib), tela em `domino_ui`.

## Build

Toolchain MSYS2 UCRT64 (`C:\msys64\ucrt64\bin` no PATH):

```bash
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

## tlc-spec-lean

profile: light
budget: 150k
