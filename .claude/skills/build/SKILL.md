---
name: build
description: Build FreeCAD debug or release. Usage /build [debug|release]
argument-hint: "[debug|release] (default: debug)"
allowed-tools: Bash, Read
user-invocable: true
---

# Build FreeCAD

Build FreeCAD using the pixi workflow.

## Arguments

`$ARGUMENTS` -- `debug` (default) or `release`

## Steps

1. Determine build type from `$ARGUMENTS` (default: debug)
2. Check pixi is available: `pixi --version`
3. Check if already configured: look for `build/{type}/build.ninja`
4. If not configured, run configure first
5. Build and report result

## Commands

**Debug (default):**
```bash
pixi run configure && pixi run build
```

**Release:**
```bash
pixi run configure-release && pixi run build-release
```

## Troubleshooting

If build fails:
- **Missing submodules**: `pixi run initialize`
- **Missing dependencies**: Check `pixi.toml`
- **Stale CMake cache**: Delete `build/{type}/` and reconfigure
- **Compile error**: Show the error and suggest fix

Report build duration and success/failure status.
