#!/bin/bash
# SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note
# Copyright (C) 2024-2026 [Tera Naam]
# gcc-plugin.sh - Check GCC plugin support for Linuxab OS

CC="$1"

if [ -z "$CC" ]; then
	echo "Usage: $0 <gcc-binary>" >&2
	exit 1
fi

# Check if GCC supports plugins
if ! $CC -print-file-name=plugin > /dev/null 2>&1; then
	echo "n"
	exit 0
fi

PLUGIN_DIR=$($CC -print-file-name=plugin 2>/dev/null)

if [ -z "$PLUGIN_DIR" ] || [ "$PLUGIN_DIR" = "plugin" ]; then
	echo "n"
	exit 0
fi

# Check for plugin headers
if [ ! -f "$PLUGIN_DIR/include/gcc-plugin.h" ]; then
	echo "n"
	exit 0
fi

# Check GCC version (need >= 4.8)
GCC_VER=$($CC -dumpversion)
GCC_MAJOR=$(echo $GCC_VER | cut -d. -f1)

if [ "$GCC_MAJOR" -lt 4 ]; then
	echo "n"
	exit 0
fi

if [ "$GCC_MAJOR" -eq 4 ]; then
	GCC_MINOR=$(echo $GCC_VER | cut -d. -f2)
	if [ "$GCC_MINOR" -lt 8 ]; then
		echo "n"
		exit 0
	fi
fi

echo "y"
exit 0