@echo off

set top=%~dp0

if exist %top%\ext\je2be\LICENSE (
	if exist %top%\ext\JUCE\LICENSE.md (
		goto submodule_ok
	) else (
		goto submodule_ng
	)
) else (
	goto submodule_ng
)

:submodule_ng
echo submodules have not been checked-out yet
exit /b 1

:submodule_ok


where msbuild
if errorlevel 1 (
	goto msbuild_ng
) else (
	goto msbuild_ok
)

:msbuild_ng
echo msbuild command not found
exit /b 1

:msbuild_ok


rem Build je2be dependencies
mkdir %top%\Builds\je2be
pushd %top%\Builds\je2be
call ..\..\ext\je2be\setup.bat
popd


rem Build Projucer
pushd %top%\ext\JUCE\extras\Projucer\Builds\VisualStudio2019
msbuild Projucer.sln /p:Configuration=Release
popd

start %top%\ext\JUCE\extras\Projucer\Builds\VisualStudio2019\x64\Release\App\Projucer.exe %top%\je2be.jucer
