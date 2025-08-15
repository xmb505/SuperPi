# SuperPi Makefile (Gauss-Legendre version)
# Copyright (c) 2025 新毛宝贝 (xmb505)

CC = gcc
CFLAGS = -Wall -Wextra -O3 -std=c99 -march=native
LDFLAGS = -lm -lgmp -lfftw3 -lm
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share
LOCALEDIR = $(DATADIR)/locale

# Get git hash for version info
GIT_VERSION := $(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown")

# Source files
SOURCES = src/superpi.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = superpi

# Internationalization files
POT_FILE = i18n/superpi.pot
PO_FILES = i18n/zh_CN.po
MO_FILES = $(PO_FILES:.po=.mo)

# Default target
all: $(TARGET)

# Build the main executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -DGIT_VERSION=\"$(GIT_VERSION)\" -c $< -o $@

# Internationalization targets
pot: $(SOURCES)
	mkdir -p i18n
	xgettext --keyword=_ --language=C --add-comments --sort-output \
		--package-name=superpi --package-version=1.0.0 \
		--msgid-bugs-address=xmb505@blog.xmb505.top \
		-o $(POT_FILE) $(SOURCES)

i18n/zh_CN.po: $(POT_FILE)
	msginit --input=$(POT_FILE) --output=i18n/zh_CN.po --locale=zh_CN.UTF-8

%.mo: %.po
	msgfmt $< -o $@

# Install targets
install: $(TARGET) $(MO_FILES)
	install -D -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)
	install -D -m 644 i18n/zh_CN.mo $(DESTDIR)$(LOCALEDIR)/zh_CN/LC_MESSAGES/superpi.mo

# Uninstall target
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(TARGET)
	rm -f $(DESTDIR)$(LOCALEDIR)/zh_CN/LC_MESSAGES/superpi.mo

# Clean targets
clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f i18n/*.mo

distclean: clean
	rm -f i18n/*.po i18n/*.pot

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
	sudo apt-get install -y build-essential gettext libgmp-dev libfftw3-dev

# CPU stress test target
cpu-test: $(TARGET)
	./$(TARGET) 1000000  # 1 million digits stress test

# Package target
package: clean
	tar -czf superpi-5.0.0.tar.gz --exclude='.git' --exclude='*.tar.gz' .

.PHONY: all clean install uninstall test debug package pot ubuntu-deps cpu-test