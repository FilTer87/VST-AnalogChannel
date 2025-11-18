# Version Update Workflow

This document describes how to update the plugin version across all files.

## Automated Method (Recommended)

### Step 1: Update version in Projucer

1. Open `AnalogChannel.jucer` in Projucer
2. Change the **version** field (e.g., from `0.4` to `0.4.1`)
3. **Save** the project (this regenerates build files)

### Step 2: Run the version update script

```powershell
.\update-version.ps1
```

The script will:
- Read the version from `AnalogChannel.jucer`
- Update all documentation and source files automatically
- Display a summary of changes

### Step 3: Rebuild and commit

```bash
# Review changes
git diff

# Rebuild plugin
cd Builds\VisualStudio2022
msbuild AnalogChannel.sln /p:Configuration=Release /p:Platform=x64

# Commit changes
git add .
git commit -m "Bump version to 0.4.1"

# Tag the release
git tag v0.4.1

# Push to GitHub
git push && git push --tags
```

---

## Manual Method (If script fails)

If you need to update the version manually, edit these files:

1. **AnalogChannel.jucer** (line 7)
   ```xml
   version="0.4.1"
   ```

2. **Docs/UserManual.md** (3 occurrences):
   - Line 4: `**Version 0.4.1**`
   - Line 1096: `- **Version**: 0.4.1`
   - Line 1258: `*AnalogChannel User Manual v0.4.1 | Copyright © 2025 KuramaSound | Filippo Terenzi*`

3. **InternalDocs/README_Release.txt** (line 4):
   ```
   Version 0.4.1
   ```

4. **Source/PluginEditor.cpp** (line 293):
   ```cpp
   infoLabel.setText ("AnalogChannel v0.4.1\nVST3 Channel Strip Plugin by Filippo Terenzi",
   ```

---

## Files Updated by Script

| File | Location | Pattern |
|------|----------|---------|
| `Docs/UserManual.md` | Line 4 | `**Version X.X.X**` |
| `Docs/UserManual.md` | Line 1096 | `- **Version**: X.X.X` |
| `Docs/UserManual.md` | Line 1258 | `*AnalogChannel User Manual vX.X.X \|` |
| `InternalDocs/README_Release.txt` | Line 4 | `Version X.X.X` |
| `Source/PluginEditor.cpp` | Line 293 | `AnalogChannel vX.X.X\n` |

---

## Semantic Versioning Guide

Use semantic versioning: `MAJOR.MINOR.PATCH`

- **MAJOR** (1.0.0 → 2.0.0): Breaking changes (incompatible with previous versions)
  - DSP algorithm changes that alter sound
  - Parameter removal/renaming
  - Preset format changes

- **MINOR** (1.0.0 → 1.1.0): New features (backward compatible)
  - New processing modes
  - Additional parameters
  - New GUI features

- **PATCH** (1.0.0 → 1.0.1): Bug fixes (backward compatible)
  - Bug fixes
  - Documentation updates
  - Minor GUI improvements

---

## Preset Compatibility

**Important**: Preset and DAW session compatibility depends on:
- Plugin Code (4 characters)
- Plugin Manufacturer Code (4 characters)
- Plugin Name

As long as these remain unchanged in the `.jucer` file, **all versions remain compatible** regardless of version number changes.

---

## Troubleshooting

### Script reports "Pattern not found"

This means the version format in the file has changed. You can:
1. Check the file manually and verify the pattern
2. Update the script's regex pattern in `update-version.ps1`
3. Update the file manually

### Script fails with encoding errors

Ensure all files are saved as UTF-8. If needed, convert them:
```powershell
Get-Content file.txt | Set-Content file.txt -Encoding UTF8
```

### Want to set a custom version without changing .jucer

```powershell
.\update-version.ps1 -Version "1.0.0-beta"
```

This updates all files to the specified version without reading from `.jucer`.
