# Single file format
clang-format -i drivers/acpi/acpi.c

# Recursive format all C/H files
find . -name '*.c' -o -name '*.h' | xargs clang-format -i

# Check diff without writing
clang-format --dry-run -Werror arch/x86/kernel/setup.c