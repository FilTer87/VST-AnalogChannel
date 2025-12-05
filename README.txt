================================================================================
                            AnalogChannel VST3
                          Channel Strip Plugin
                               Version 0.5.1
================================================================================

Author: Filippo Terenzi
Company: KuramaSound
Website: www.kuramasound.com
License: GPL v3

================================================================================
WHAT IS ANALOGCHANNEL?
================================================================================

AnalogChannel is a comprehensive VST3 channel strip plugin designed for fast
and characterful mixing. It combines saturation, filtering, compression, EQ,
and console emulation in a streamlined interface with carefully limited
controls for quick and musical results.

Key Features:
- Input saturation (Clean/Pure/Tape/Tube modes)
- HPF/LPF filters with Bump option
- Clean compressor for transparent peak control
- Low Dynamic processor (dual-mode expander/upward compressor)
- 4-band EQ with musical shelving and parametric bands
- Style compressor (Warm/Punch character modes)
- Console emulation (Essex/Oxford/USA)
- Output stage with saturation and clipping
- Channel Variation system (48 presets for analog modeling)
- Comprehensive metering (input/output/GR meters)
- Zero latency, full DAW automation support

AnalogChannel is your go-to channel strip for fast, colored, musical mixing
decisions. It is NOT intended for surgical precision!

================================================================================
SYSTEM REQUIREMENTS
================================================================================

Windows:
- Windows 10 or later (64-bit)
- VST3-compatible DAW
- Modern multi-core CPU recommended
- 4 GB RAM minimum

macOS:
- macOS 10.13 (High Sierra) or later
- macOS 11+ recommended for universal binary (Intel + Apple Silicon)
- VST3 or AU compatible DAW
- Modern multi-core CPU recommended
- 4 GB RAM minimum

Linux: Work In Progress

================================================================================
INSTALLATION
================================================================================

WINDOWS
-------
1. Extract the ZIP file
2. Copy "AnalogChannel.vst3" to one of these locations:
   - C:\Program Files\Common Files\VST3\  (system-wide, all users)
   - C:\Users\[YourUsername]\AppData\Local\Programs\Common\VST3\  (user only)
3. Rescan plugins in your DAW
4. Load AnalogChannel from your plugin browser (KuramaSound or FX category)

macOS
-----
OPTION 1: Automated Installation (Recommended)
1. Extract the ZIP file
2. Open Terminal and navigate to the macOS folder:
   cd /path/to/AnalogChannel_Release_vX.X.X/macOS
3. Run the installation script:
   ./install-macos.sh
4. The script will automatically:
   - Remove quarantine flags (avoids security warnings)
   - Install VST3 to ~/Library/Audio/Plug-Ins/VST3/
   - Install AU to ~/Library/Audio/Plug-Ins/Components/
   - Clear Audio Unit cache
5. Rescan plugins in your DAW
6. Load AnalogChannel from your plugin browser

OPTION 2: Manual Installation
VST3:
1. Extract the ZIP file
2. Navigate to macOS folder
3. Copy "AnalogChannel.vst3" to:
   ~/Library/Audio/Plug-Ins/VST3/  (user only, recommended)
   OR
   /Library/Audio/Plug-Ins/VST3/  (system-wide, requires sudo)
4. Remove quarantine flag:
   xattr -cr ~/Library/Audio/Plug-Ins/VST3/AnalogChannel.vst3

AU (Audio Units):
1. Copy "AnalogChannel.component" to:
   ~/Library/Audio/Plug-Ins/Components/  (user only, recommended)
   OR
   /Library/Audio/Plug-Ins/Components/  (system-wide, requires sudo)
2. Remove quarantine flag:
   xattr -cr ~/Library/Audio/Plug-Ins/Components/AnalogChannel.component
3. Clear AU cache:
   rm -rf ~/Library/Caches/AudioUnitCache
4. Rescan plugins in your DAW (Logic Pro: AU Manager → Reset & Rescan)

================================================================================
DOCUMENTATION
================================================================================

Full User Manual: AnalogChannel_UserManual.pdf (included in this package)

The manual covers:
- Complete interface overview
- Detailed section-by-section guide
- Metering system
- Signal flow and routing options
- Tips and best practices
- Technical specifications
- Credits and license information


================================================================================
TROUBLESHOOTING
================================================================================

Plugin Not Appearing in DAW:
- Verify you copied the plugin to the correct VST3/AU folder
- Rescan plugins in your DAW preferences
- Restart your DAW
- Check DAW's plugin scan log for errors

macOS "Damaged and Can't Be Opened" Error:
- Right-click the plugin → "Open" → "Open anyway"
- Or disable Gatekeeper for the plugin:
  xattr -cr /path/to/AnalogChannel.vst3
- If distributing publicly, code signing is recommended (Developer ID)

Linux Plugin Not Loading:
- Check library dependencies: ldd ~/.vst3/AnalogChannel.vst3/Contents/x86_64-linux/AnalogChannel.so
- Install missing dependencies (ALSA, JACK, X11, etc.)
- Check file permissions: chmod -R 755 ~/.vst3/AnalogChannel.vst3

Audio Issues:
- Check sample rate matches your DAW project (44.1kHz, 48kHz, etc.)
- Verify input/output meters show signal
- Try bypassing sections one by one to isolate issues

For further help, see the User Manual PDF or visit www.kuramasound.com

================================================================================
CREDITS
================================================================================

AnalogChannel is built using algorithms from:

AirWindows (MIT License):
- PurestDrive (saturation)
- ToTape8 (tape emulation)
- Tube2 (tube saturation)
- Baxandall2 (shelving EQ)
- PurestConsole3/Channel8 (console emulation)
- FinalClip/ClipSoftly (output clipping)
GitHub: https://github.com/airwindows/airwindows

JSFX Algorithms:
- DigitalVersatileCompV2 (clean compressor, GPL License, from Michael Gruhn [LOSER])
- CL1B Compressor (warm/optical compression, from JClones) - 

Framework:
- JUCE Framework (GPL v3)
  Website: https://juce.com/

Special thanks to Chris Johnson (AirWindows) for the exceptional open-source
DSP algorithms and to the JSFX community for their contributions.

================================================================================
LICENSE
================================================================================

AnalogChannel is licensed under the GNU General Public License v3 (GPL v3).

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program (see LICENSE.txt). If not, see <https://www.gnu.org/licenses/>.

Ported algorithms retain their original licenses (MIT for AirWindows, various
for JSFX). See User Manual for complete license information.

================================================================================
SOURCE CODE
================================================================================

Full source code is available on GitHub:
https://github.com/FilTer87/VST-AnalogChannel

Contributions, bug reports, and feature requests are welcome!

================================================================================
SUPPORT
================================================================================

Website: www.kuramasound.com
GitHub Issues: https://github.com/FilTer87/VST-AnalogChannel/issues

If you find AnalogChannel useful, consider supporting development!

	Buy Me a Coffee: https://buymeacoffee.com/oz3watvqah

================================================================================

Thank you for using AnalogChannel!

Filippo Terenzi / KuramaSound
Copyright © 2025

================================================================================
