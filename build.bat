@echo off

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "%VSWHERE%" (
    echo vswhere.exe not found
    exit /b 1
)

for /f "usebackq delims=" %%i in (`
    "%VSWHERE%" -latest -products * ^
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 ^
    -property installationPath
`) do (
    set "VSINSTALL=%%i"
)

if not defined VSINSTALL (
    echo No Visual Studio C++ toolchain installation found
    exit /b 1
)

echo Using Visual Studio:
echo %VSINSTALL%

call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat"

if errorlevel 1 (
    echo Failed to initialize MSVC environment
    exit /b 1
)

echo.
cmake -S . -B build -G Ninja
