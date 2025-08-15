# IFLOW.md

This file provides guidance to iFlow Cli when working with code in this repository.

## Build Commands

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install build-essential gettext libgmp-dev

# Build the project
make clean && make

# Run basic tests
make test

# CPU stress test with 100M digits
make cpu-test

# Install system-wide
sudo make install

# Install dependencies only
make ubuntu-deps
```

## Architecture Overview

### Core Components
- **superpi.c**: Main application implementing unlimited precision π calculation using the GMP library
- **Makefile**: Build system with GMP integration and internationalization support
- **po/**: Internationalization support with Chinese and English translations via gettext

### Key Features
- **Unlimited Precision**: Uses GMP library for arbitrary precision arithmetic
- **Accurate Calculation**: Implements Machin's formula for π computation
- **Internationalization**: GNU gettext support for multiple languages
- **CPU Stress Testing**: Designed for CPU stability testing with configurable precision

### Performance Characteristics
- **Precision-based performance**: Computation time scales with requested digit count
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
- GNU gettext for internationalization
- Sufficient RAM for large digit counts (scales with input size)