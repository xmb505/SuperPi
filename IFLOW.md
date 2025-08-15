# IFLOW.md

This file provides guidance to iFlow Cli when working with code in this repository.

## Build Commands

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install build-essential libgmp-dev libfftw3-dev

# Build the project
make clean && make

# Run basic tests
make test

# CPU stress test with 1M digits
make cpu-test

# Install system-wide
sudo make install

# Install dependencies only
make ubuntu-deps
```

## Architecture Overview

### Core Components
- **src/superpi.c**: Main application implementing Gauss-Legendre π calculation using GMP and FFTW3 libraries
- **Makefile**: Build system with GMP and FFTW3 integration

### Key Features
- **High Precision**: Uses GMP library for arbitrary precision arithmetic
- **Fast Computation**: Implements Gauss-Legendre algorithm for π computation
- **CPU Stress Testing**: Designed for CPU stability testing with configurable precision
- **Multiple Modes**: Supports single calculation and continuous calculation modes

### Performance Characteristics
- **Algorithm-based performance**: Gauss-Legendre algorithm has quadratic convergence
- **Memory-based limits**: Uses system RAM as constraint for digit count
- **Performance metrics**: Reports digits/second processing rate

### Development Workflow
1. Install dependencies: `make ubuntu-deps`
2. Build: `make clean && make`
3. Test: `make test` or `make cpu-test`
4. Stress test: `./superpi <digit_count>` (e.g., 1000000 for 1 million digits)

### System Requirements
- Linux system with GCC compiler
- GMP library for arbitrary precision arithmetic
- FFTW3 library for optimized computations
- Sufficient RAM for large digit counts (scales with input size)