# Vulkan Loader Attack Vectors

A sample Vulkan layer and code for installing the layer on Windows systems.

This layer performs simple keylogging and checks for administrator privileges upon application startup.
The layer writes logs to files named `sample_layer_output.log` and `sample_layer_key_output.log` in the working directory of any Vulkan application it runs with. `sample_layer_output.log` contains the status of whether or not the layer has administrator privileges, and `sample_layer_key_output.log` contains key-press logs over the lifetime of the Vulkan application.

The Vulkan layer is a modified version of [Baldur Karlsson's sample layer](https://github.com/baldurk/sample_layer), distributed under the BSD-2 license. See [Third-Party Licenses](#third-party-licenses)

## Build Requirements
- Windows 10 (64-bit)
- Vulkan SDK 1.3.204
- Visual Studio 17 2022
- CMake 3.10.2 or higher
- NSIS 3.08

## Build Instructions

This software is can be detected and automatically removed by anti-virus software.
Ensure your anti-virus software is turned off, or whitelist this repository in your anti-virus before building.

To build the layer, run these commands:
```
mkdir build
cd build
cmake -A x64 ..
cmake --build . -- config Release
```

The directory `build/src/Release` should now contain these files:
```
sample_layer.dll
sample_layer.exp
sample_layer.lib
sample_layer_windows.json
sample_layer_silent_vkconfig_override_replacer.nsi
sample_layer_silent_user_installer.nsi
```

The `.dll` and `.json` are the layer and the layer manifest file respectively.

The `.nsi` files are scripts used by NSIS to generate installers for the layer.
Use either the NSIS GUI or the `makensis` command line tool to generate their `.exe.` files.
```
makensis.exe sample_layer_silent_vkconfig_override_replacer.nsi
makensis.exe sample_layer_silent_user_installer.nsi
```

`sample_layer_silent_user_installer.exe` will silently install the layer for the current user at the location `%LocalAppData%/sample_layer`. An `uninstaller.exe` is generated for ease of uninstallation.

`sample_layer_silent_vkconfig_override_replacer.exe` will silently overwrite VkConfig's VkLayer_override layer with this layer. For this to function, VkConfig must either be running or the "Continue Overriding Layers on Exit" option should be enabled in VkConfig.

## Reproducing Attack Scenarios

### 1. VkConfig Privilege Escalation

Reproduction steps:
1. Launch VkConfig with administrator privileges.
1. Run `sample_layer_silent_vkconfig_override_replacer.exe` *without* administrator privileges.
1. Run any Vulkan application with administrator privileges.
1. In the working directory of the running escalated Vulkan application, `sample_layer_output.log` should be produced. The file should contain the following string:
```
Is process elevated? 1
```

If `sample_layer_output.log` is not produced, then the layer did not run. A change in the VkConfig settings potentially reset the VkLayer_override layer. Simply start over from step 2.

The VkConfig tool is included with the Vulkan SDK. The default location is `C:\VulkanSDK\1.3.204.1\Tools\maintenancetool.exe`.

For step 3, any Vulkan will work. The VulkanSDK includes a sample Vulkan application `C:\VulkanSDK\1.3.204.1\Bin\vkcube.exe` that renders a cube.

### 2. Silent User-Level Layer Installation

This sample layer can be installed silently on a system with only user-level permissions and be automatically ran whenever a Vulkan application starts.
A simple key-logger is built into the layer as a proof-of-concept malware that can be embedded into layers.
Simply run `sample_layer_silent_user_installer.exe` to install the layer. No message or popups should appear.

To test the layer, run any Vulkan application (such as `C:\VulkanSDK\1.3.204.1\Bin\vkcube.exe`, included with the VulkanSDK) without administrator privileges. The files `sample_layer_output.log` and `sample_layer_key_output.log` should be created in the working directory of the Vulkan application, with the latter file recording all key-presses.

When finished, navigate to `%LocalAppData%/sample_layer` and run `uninstaller.exe` to uninstall the layer.

## Vulkan Loader Static Analyses

[Version 1.3.204.1 of the Vulkan Loader](https://github.com/KhronosGroup/Vulkan-Loader/tree/sdk-1.3.204.1) was analyzed.

Versions of static analyses software used:
- [Cppcheck](https://cppcheck.sourceforge.io/) 2.7 (Open Source)
- [Flawfinder](https://dwheeler.com/flawfinder/) 2.0.19

The results of the analyses are in the `VulkanLoaderAnalyses/` directory.

For both analysis tools the default settings were used.

### Reproducing Cppcheck analysis results

The Cppcheck GUI configuration file used to produce the Cppcheck analysis results is `vulkan-loader.cppcheck` located in the `VulkanLoaderAnalyses/` folder within this repository. 
The GUI is preferred, as it offers features that are not available through the command line.

The Cppcheck configuration depends on the Visual Studio solution created after CMake project configuration.
Use these commands to generate the Visual Studio solution file `build/Vulkan-Loader.sln` in the Vulkan-Loader repository:
```
cd Vulkan-Loader
mkdir build
cd build
cmake -A x64 -DUPDATE_DEPS=ON -G "Visual Studio 17 2022" ..
```

Copy the `.cppcheck` file to the root of the Vulkan-Loader repository and use the Cppcheck GUI to open it.
Cppcheck will automatically produce the analysis results and allow you to view each error or warning in the GUI.
When prompted to create a `cppcheck-loader-cppcheck-build-dir` folder, do so to store the analysis results and speed up the analysis speed through caching.

### Reproducing Flawfinder analysis results

Flawfinder requires Python 2.7 or Python 3.
Simply install flawfinder using pip:
```
pip install flawfinder
```

Then run flawfinder on Vulkan-Loader's source code:
```
cd Vulkan-Loader
python -m flawfinder loader/
```
Pipe the output using your method of choice to `flawfinder.log`.

## Why no Docker container or Virtual Machine image?

To the best of my knowledge, there are no Windows Docker images supporting Vulkan;
nor are there virtual machines that support the Vulkan graphics API on the guest machines.

## Third-Party Licenses

Files `src/sample_layer.cpp` and `src/sample_layer_windows.json` are modification's of files from [Baldur Karlsson's sample layer](https://github.com/baldurk/sample_layer).
```
BSD 2-Clause License

Copyright (c) 2016, Baldur Karlsson
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```