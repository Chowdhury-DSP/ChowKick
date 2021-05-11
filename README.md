# ChowKick

![CI](https://github.com/Chowdhury-DSP/ChowKick/workflows/CI/badge.svg)
[![License](https://img.shields.io/badge/License-BSD-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

TODO:
- better output viz
- makeup gain for filter NLs
- Other NL types... (hard clip?)
- 4 voices
- presets
- CI

This repository contains template code for building a Chowdhury DSP
audio plugin.

## Building

To build from scratch, you must have CMake installed.

```bash
# Clone the repository
$ git clone https://github.com/Chowdhury-DSP/ChowKick.git
$ cd ChowKick

# initialize and set up submodules
$ git submodule update --init --recursive

# build with CMake
$ cmake -Bbuild
$ cmake --build build --config Release
```

## License

ChowKick is open source, and is licensed under the BSD 3-clause license.
Enjoy!
