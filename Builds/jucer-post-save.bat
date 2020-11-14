@echo off
setlocal enabledelayedexpansion

set out=%~dp0\VisualStudio2019\hoge.sln
del %out%

for /f "delims=" %%a in (%~dp0\VisualStudio2019\je2be.sln) do (
	set line=%%a
	if "%%a" == "EndProject" (
		echo EndProject>> %out%
		set l=Project^(^"{C7167F0D-BC9F-4E6E-AFE1-012C56B48DB5}^"^) = ^"Package^", ^"..\Package\Package.wapproj^", ^"{713653CB-7989-4C22-8171-0F1951271B8D}^"
		rem "
		echo !l!>> %out%
		echo EndProject>> %out%
	) else (
		echo %%a>> %out%
	)
)

move %out% %~dp0\VisualStudio2019\je2be.sln
endlocal


set out=%~dp0\Package\hoge.appxmanifest
@del %out%

for /f "delims=" %%a in ('%~dp0..\ext\JUCE\extras\Projucer\Builds\VisualStudio2019\x64\Release\App\Projucer.exe --get-version %~dp0..\je2be.jucer') do set version=%%a

for /f "tokens=* delims=0123456789 eol=" %%a in ('findstr /n "^" %~dp0Package\Package.appxmanifest') do (
	set b=%%a
	setlocal enabledelayedexpansion
		set line=!b:~1!

		set maybeHeader=!line:~2,5!
		set maybeVersion=!line:~4,8!
		if "!maybeHeader!" == "<?xml" (
			echo.^<?xml version=^"1.0^" encoding=^"utf-8^"?^>>>%out%
		) else (
			if "!maybeVersion!" == "Version=" (
				echo.    Version="%version%.0" ^/^>>> %out%
			) else (
				echo.!line!>> %out%
			)
		)
	endlocal
)

copy /b %~dp0bom.bin+%out% %~dp0Package\Package.appxmanifest
