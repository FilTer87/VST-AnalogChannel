# Credits and Attributions

## AnalogChannel Plugin

**Developer**: Filippo Terenzi - KuramaSound
**License**: GNU General Public License v3.0 (GPL-3.0)
**Framework**: JUCE 8.0 (GPL/Commercial dual-license)

---

## Third-Party Algorithms and Libraries

This plugin incorporates algorithms and code from the following open-source projects. We are deeply grateful to the original authors for their contributions to the audio development community.

### AirWindows Algorithms (MIT License)

**Author**: Chris Johnson (Airwindows)
**Website**: https://www.airwindows.com
**Source**: https://github.com/airwindows/airwindows
**License**: MIT License

The following algorithms are faithful ports from AirWindows:

- **PurestDrive** - Clean saturation algorithm
  Original source: `airwindows/plugins/LinuxVST/src/PurestDrive`
  Used in: Pre-Input section (Pure mode)

- **ToTape8** - Tape emulation with flutter, bias, and head bump
  Original source: `airwindows/plugins/LinuxVST/src/ToTape8`
  Used in: Pre-Input section (Tape mode)

- **Tube2** - Tube saturation with asymmetric clipping and hysteresis
  Original source: `airwindows/plugins/LinuxVST/src/Tube2`
  Used in: Pre-Input section (Tube mode)

- **Baxandall2** - Shelving EQ (bass and treble)
  Original source: `airwindows/plugins/LinuxVST/src/Baxandall2`
  Used in: EQ section (Low Shelf and High Shelf)

- **Channel8** - Console emulation (Neve/API/SSL)
  Original source: `airwindows/plugins/LinuxVST/src/Channel8`
  Used in: Console section (Essex/Oxford/USA modes)

- **FinalClip** - Hard clipper with slew limiting
  Original source: `airwindows/plugins/LinuxVST/src/FinalClip`
  Used in: OutStage section (Hard Clip mode)

- **ClipSoftly** - Soft clipper with adaptive smoothing
  Original source: `airwindows/plugins/LinuxVST/src/ClipSoftly`
  Used in: OutStage section (Soft Clip mode)

**MIT License Text:**
```
Copyright (c) 2016 Chris Johnson (Airwindows)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

### JClones JSFX Algorithms (MIT License)

**Author**: JClones
**Source**: https://github.com/JClones/JSFXClones
**License**: MIT License

- **CL1B Compressor** - Optical compressor emulation
  Original source: `JSFXClones/JClones_CL1B.jsfx`
  Used in: Style-Comp section (Warm mode)

**MIT License applies** (same terms as above)

---

### Michael Gruhn (LOSER) JSFX Algorithms (GPL License)

**Author**: Michael Gruhn (LOSER)
**Source**: JSFX plugins included with REAPER DAW
**License**: GNU General Public License (GPL)

- **Digital Versatile Compressor V2** - Clean, transparent compressor
  Original source: `Digital_Versatile_Compressor_V2.jsfx`
  Used in: Control-Comp section and Style-Comp section (Punch mode)

**GPL License**: This algorithm is licensed under the GNU General Public License. The complete GPL v3 text is available in the LICENSE file.

---

## JUCE Framework

**Developer**: ROLI Ltd. (now part of Focusrite Group)
**Website**: https://juce.com
**License**: Dual-licensed under GPL v3 and Commercial License

This plugin is built using the JUCE framework version 8.0, licensed under GPL v3.

---

## Porting and Integration

All third-party algorithms have been **faithfully ported** to C++/JUCE, preserving the original DSP mathematics and signal processing behavior. The ports are designed to maintain bit-accurate output within floating-point precision limits (< 1e-6 deviation).

**Port Author**: Filippo Terenzi - KuramaSound
**Port Date**: November 2025

---

## License Compatibility

This project combines code from multiple sources with compatible licenses:

- **GPL v3** (JUCE framework, Michael Gruhn algorithms, AnalogChannel original code)
- **MIT License** (AirWindows algorithms, JClones algorithms)

The MIT License is compatible with GPL v3, and all MIT-licensed code can be incorporated into GPL-licensed projects. The resulting combined work (AnalogChannel) is distributed under **GPL v3**.

---

## Acknowledgments

Special thanks to:

- **Chris Johnson (Airwindows)** for creating an incredible library of open-source audio algorithms and sharing them with the community under a permissive MIT license
- **JClones** for high-quality JSFX algorithm implementations
- **Michael Gruhn (LOSER)** for versatile compressor algorithms
- **JUCE team** for providing an excellent cross-platform audio plugin framework
- The entire open-source audio development community

---

**AnalogChannel** © 2025 KuramaSound
Licensed under GNU General Public License v3.0

For full license text, see LICENSE file.
For source code, see the Source/ directory.
