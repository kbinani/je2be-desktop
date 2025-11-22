# je2be-desktop
- Map converter for Minecraft
- Available on Microsoft Store https://www.microsoft.com/store/apps/9PC9MFX9QCXS
- Supported conversion modes:
  - Java ▶️ Bedrock
  - Bedrock ▶️ Java
  - Xbox360 ▶️ Bedrock
  - Xbox360 ▶️ Java
  - PS3 ▶️ Bedrock
  - PS3 ▶️ Java

# Screenshot

![app screen shot: selecting java input](/img/ss1.png)
![app screen shot: progress window while converting](/img/ss2.png)

# How to build
```sh
git clone https://github.com/kbinani/je2be-desktop.git
cd je2be-desktop
git submodule update --init --recursive
cd Builds
cmake ..
```

then build with cmake to get the exe `Builds/je2be-desktop_artifacts/Release/je2be.exe` by

```
cmake --build . --target je2be-desktop --config Release --parallel
```

or just open `Builds/je2be-desktop.slnx` with Visual Studio

# License
GPL-3.0

# Author
kbinani [@kbinani](https://twitter.com/kbinani)

# SAST Tools

[PVS-Studio](https://pvs-studio.com/en/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) - static analyzer for C, C++, C#, and Java code.
