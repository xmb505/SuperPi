# SuperPi Makefile (Gauss-Legendre version)
# Copyright (c) 2025 新毛宝贝 (xmb505)

CC = gcc
CFLAGS = -Wall -Wextra -O3 -std=c99 -march=native
LDFLAGS = -lm -lgmp -lfftw3 -lm
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share

# Get git hash for version info
GIT_VERSION := $(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown")

# Source files
SOURCES = src/superpi.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = superpi

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -DGIT_VERSION=\"$(GIT_VERSION)\" -c $< -o $@

# Install targets
install: $(TARGET)
	install -D -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)

# Uninstall target
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)

# Clean targets
clean:
	rm -f $(OBJECTS) $(TARGET)

# Test target
test: $(TARGET)
	./$(TARGET) 100
	./$(TARGET) 1000
	@echo "Basic tests completed successfully!"

# Development targets
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# Ubuntu/Debian specific targets
ubuntu-deps:
	sudo apt-get update
	sudo apt-get install -y build-essential libgmp-dev libfftw3-dev

# CPU stress test target
cpu-test: $(TARGET)
	./$(TARGET) 1000000  # 1 million digits stress test

# Package target
package: clean
	tar -czf superpi-5.0.0.tar.gz --exclude='.git' --exclude='*.tar.gz' .

.PHONY: all clean install uninstall test debug package ubuntu-deps cpu-test