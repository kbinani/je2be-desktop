# je2be-desktop
- Map converter for Minecraft
- Available on Microsoft Store https://www.microsoft.com/store/apps/9PC9MFX9QCXS
- Supported conversion modes:
  - Java ▶️ Bedrock
  - Bedrock ▶️ Java
  - Xbox360 ▶️ Bedrock
  - Xbox360 ▶️ Java

# Screenshot

![app screen shot: selecting java input](/img/ss1.png)
![app screen shot: progress window while converting](/img/ss2.png)

# How to build
```sh
git clone https://github.com/kbinani/je2be-desktop.git
cd je2be-desktop
git submodule update --init --recursive
cd Builds
cmake .. -DJE2BE_DESKTOP_INTEL_COMPILER=OFF
```

then build with cmake to get the exe `Builds/je2be-desktop_artifacts/Release/je2be.exe` by

```
cmake --build . --target je2be-desktop --config Release --parallel
```

or just open `Builds/je2be-desktop.sln` with Visual Studio

# License
GPL-3.0

# Author
kbinani [@kbinani](https://twitter.com/kbinani)
