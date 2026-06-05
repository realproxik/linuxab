# 1. Save to repo root
cp .gitattributes /path/to/linuxab/

# 2. Verify attributes on files
git check-attr -a drivers/acpi/acpi.c
# Output: drivers/acpi/acpi.c: text: set
#         drivers/acpi/acpi.c: diff: cpp
#         drivers/acpi/acpi.c: eol: lf

# 3. Renormalize if CRLF exists
git add --renormalize .
git status  # Check changes
git commit -m "Normalize line endings to LF"

# 4. Configure diff drivers (optional but recommended)
git config diff.dts.command "cat"  # Or your dts diff tool
git config diff.kconfig.command "diff -u"