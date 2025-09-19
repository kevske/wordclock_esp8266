# Tests (Path A: Arduino-CLI project with host unit tests)

This folder contains host-run unit tests that exercise pure C++ logic from the project without requiring hardware.

We compile tests with a normal C++ compiler (g++) using light-weight mocks of Arduino/Adafruit headers found in `tests/unit/mocks/`.

## Layout

- `tests/unit/ledmatrix/` — unit tests for `ledmatrix.cpp/h` color helpers and future logic
- `tests/unit/mocks/` — minimal stubs for Arduino/Adafruit and `UDPLogger` to allow host compilation
- `scripts/ci/run_unit_tests.sh` — CI helper to discover and run all unit tests

Your existing Arduino CLI workflow still builds the sketch for ESP8266. The new workflow runs host tests in parallel on GitHub Actions.

## Local usage

### Linux / macOS

Requirements:
- g++ and make

Run:

```bash
cd tests/unit/ledmatrix
make clean && make && make run
```

Or run all unit tests via the CI helper script:

```bash
chmod +x ../../scripts/ci/run_unit_tests.sh
../../scripts/ci/run_unit_tests.sh
```

### Windows

Option A (recommended): WSL (Ubuntu)
- Install WSL and Ubuntu
- Inside Ubuntu shell, follow the Linux steps above

Option B: MSYS2 / Mingw64
- Install MSYS2, open the Mingw64 shell, and install build tools:
  - `pacman -S --needed base-devel mingw-w64-x86_64-toolchain`
- Then run the `make` commands in `tests/unit/ledmatrix/`

## CI

A new workflow `.github/workflows/unit_tests.yml` builds and runs the host tests using Ubuntu runners:
- Installs build-essential
- Executes `scripts/ci/run_unit_tests.sh`

This is independent of the existing Arduino CLI compile workflow `compile_esp8266.yml`.

## Adding more tests

- Create a new suite under `tests/unit/<area>/`
- Add a `Makefile` similar to `tests/unit/ledmatrix/Makefile`
- Include needed mocks via `-I../mocks` and project sources via `-I../../../`
- Add your `test_*.cpp` files and reference any project `.cpp` sources needed for linking

Keep test code independent from hardware by:
- Factoring pure logic into `.cpp/.h` files
- Wrapping hardware-specific calls behind small interfaces that can be mocked in `tests/unit/mocks/`
