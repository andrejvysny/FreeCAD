---
name: check-format
description: Run formatting checks (clang-format for C++, Black for Python) on staged or specified files
argument-hint: "[file_path] (default: staged files)"
allowed-tools: Bash, Read, Grep, Glob
user-invocable: true
---

# Check Formatting

Run clang-format (C++) and Black (Python) formatting checks.

## Arguments

`$ARGUMENTS` -- Optional file path. Default: check staged files.

## Steps

1. Determine files to check:
   - If argument: check that specific file
   - If no argument: `git diff --cached --name-only` for staged files

2. **C++ files** (.cpp, .h):
```bash
clang-format --dry-run --Werror -style=file <file>
```

3. **Python files** (.py):
```bash
black --check --line-length 100 <file>
```

4. Report results:
   - List files with issues
   - Show diff of required changes
   - Suggest fix command

## Quick Fix Commands

```bash
# Fix C++ formatting
clang-format -i -style=file <files>

# Fix Python formatting
black --line-length 100 <files>

# Run all pre-commit hooks
pixi run -- pre-commit run --all-files
```

## FreeCAD Formatting Rules

### C++ (clang-format)
- 4-space indent, 100-char limit, no tabs
- Braces: new line for class/struct/function; same line for if/for/while
- Always braces on control statements
- Break before binary operators
- Left-aligned pointers: `int* ptr`
- Never auto-sort includes

### Python (Black)
- 100-char line limit
- PEP8 compliant
- Enforced by pre-commit hooks
