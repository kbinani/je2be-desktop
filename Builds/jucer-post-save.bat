@echo off
setlocal enabledelayedexpansion

set out=%~dp0\VisualStudio2019\hoge.sln
del %out%

for /f "delims=" %%a in (%~dp0\VisualStudio2019\je2be.sln) do (
	set line=%%a
	if "%%a" == "EndProject" (
		echo EndProject>> %out%
		set l=Project^(^"{C7167F0D-BC9F-4E6E-AFE1-012C56B48DB5}^"^) = ^"Package^", ^"..\Package\Package.wapproj^", ^"{713653CB-7989-4C22-8171-0F1951271B8D}^"
		echo !l!>> %out%
		echo EndProject>> %out%
	) else (
		echo %%a>> %out%
	)
)

move %out% %~dp0\VisualStudio2019\je2be.sln
endlocal
