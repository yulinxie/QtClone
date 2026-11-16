@echo off
REM ===========================================================================
REM  QtClone build script
REM
REM  Builds the framework, the tools, the designer, the examples and runs the
REM  smoke tests.  Visual Studio 2010 (v100) is the primary target; newer
REM  toolchains are used as a fallback when v100 is not installed.
REM
REM  usage:  build.bat [lib|all|clean]
REM ===========================================================================
setlocal enabledelayedexpansion
set "ROOT=%~dp0"
if "%ROOT:~-1%"=="\" set "ROOT=%ROOT:~0,-1%"
cd /d "%ROOT%"

set "OUT=%ROOT%\out"
set "OBJ=%OUT%\obj"

if /i "%~1"=="clean" (
    if exist "%OUT%" rmdir /s /q "%OUT%"
    echo cleaned.
    exit /b 0
)

set "VCVARS="
for %%e in (
    "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
    "C:\Program Files\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
    "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars32.bat"
    "C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Auxiliary\Build\vcvars32.bat"
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"
) do (
    if exist "%%~e" if not defined VCVARS set "VCVARS=%%~e"
)
if not defined VCVARS (
    where cl.exe >nul 2>nul
    if errorlevel 1 (
        echo ERROR: no Visual C++ toolchain found.
        echo        Install Visual Studio 2010 ^(v100^) or run this from a
        echo        "Developer Command Prompt".
        exit /b 1
    )
) else (
    echo [setup] using "!VCVARS!"
    call "!VCVARS!" >nul
)

if not exist "%OUT%" mkdir "%OUT%"
if not exist "%OBJ%" mkdir "%OBJ%"

set "CFLAGS=/nologo /EHsc /MD /O2 /W3 /DUNICODE /D_UNICODE /D_CRT_SECURE_NO_WARNINGS /I"%ROOT%\include" /I"%ROOT%\src\gui" /I"%ROOT%\tools\uicgen""
set "LIBS=user32.lib gdi32.lib gdiplus.lib comctl32.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib shlwapi.lib uxtheme.lib imm32.lib advapi32.lib"
set "LOG=%OUT%\build.log"
echo. > "%LOG%"

REM --------------------------------------------------------------------------
echo [1/7] compiling framework ...
set "OBJS="
for %%f in ("%ROOT%\src\core\*.cpp" "%ROOT%\src\gui\*.cpp" "%ROOT%\src\widgets\*.cpp") do (
    cl %CFLAGS% /c "%%f" /Fo"%OBJ%\%%~nf.obj" >> "%LOG%" 2>&1
    if errorlevel 1 (echo FAILED: %%f & type "%LOG%" & exit /b 1)
    set "OBJS=!OBJS! "%OBJ%\%%~nf.obj""
)
lib /nologo /out:"%OUT%\wf.lib" !OBJS! >> "%LOG%" 2>&1 || exit /b 1
echo       %OUT%\wf.lib
if /i "%~1"=="lib" goto :done

REM --------------------------------------------------------------------------
echo [2/7] building qtcuic ...
cl %CFLAGS% /c "%ROOT%\tools\qtcuic\qtcuic.cpp" /Fo"%OBJ%\qtcuic.obj" >> "%LOG%" 2>&1 || (echo qtcuic compile FAILED & exit /b 1)
cl %CFLAGS% /c "%ROOT%\tools\uicgen\uicgen.cpp" /Fo"%OBJ%\uicgen.obj" >> "%LOG%" 2>&1 || (echo uicgen compile FAILED & exit /b 1)
link /nologo /out:"%OUT%\qtcuic.exe" "%OBJ%\qtcuic.obj" "%OBJ%\uicgen.obj" "%OUT%\wf.lib" %LIBS% >> "%LOG%" 2>&1 || (echo qtcuic link FAILED & exit /b 1)
echo       %OUT%\qtcuic.exe

echo [3/7] building designer ...
cl %CFLAGS% /c "%ROOT%\designer\designer.cpp" /Fo"%OBJ%\designer.obj" >> "%LOG%" 2>&1 || (echo designer compile FAILED & exit /b 1)
link /nologo /out:"%OUT%\wfdesigner.exe" "%OBJ%\designer.obj" "%OBJ%\uicgen.obj" "%OUT%\wf.lib" %LIBS% >> "%LOG%" 2>&1 || (echo designer link FAILED & exit /b 1)
echo       %OUT%\wfdesigner.exe

echo [4/7] building gallery ...
cl %CFLAGS% /c "%ROOT%\examples\gallery\gallery.cpp" /Fo"%OBJ%\gallery.obj" >> "%LOG%" 2>&1 || (echo gallery compile FAILED & exit /b 1)
link /nologo /out:"%OUT%\gallery.exe" "%OBJ%\gallery.obj" "%OUT%\wf.lib" %LIBS% >> "%LOG%" 2>&1 || (echo gallery link FAILED & exit /b 1)
echo       %OUT%\gallery.exe

echo [5/7] building and running smoke tests ...
cl %CFLAGS% /c "%ROOT%\tests\smoke.cpp" /Fo"%OBJ%\smoke.obj" >> "%LOG%" 2>&1 || (echo smoke compile FAILED & exit /b 1)
link /nologo /out:"%OUT%\wf_smoke.exe" "%OBJ%\smoke.obj" "%OBJ%\uicgen.obj" "%OUT%\wf.lib" %LIBS% >> "%LOG%" 2>&1 || (echo smoke link FAILED & exit /b 1)
"%OUT%\wf_smoke.exe" > "%OUT%\smoke.log" 2>&1
REM check both: exit 1 (assertion failure) and negative codes (crash / stack overflow)
if errorlevel 1 goto smoke_failed
if not errorlevel 0 goto smoke_failed
goto smoke_passed
:smoke_failed
echo smoke tests FAILED - see %OUT%\smoke.log
type "%OUT%\smoke.log"
exit /b 1
:smoke_passed
echo       smoke tests passed

echo [6/7] running designer self test ...
"%OUT%\wfdesigner.exe" --selftest > "%OUT%\designer_selftest.log" 2>&1
if errorlevel 1 goto designer_failed
if not errorlevel 0 goto designer_failed
goto designer_passed
:designer_failed
echo designer self test FAILED - see %OUT%\designer_selftest.log
type "%OUT%\designer_selftest.log"
exit /b 1
:designer_passed
echo       designer self test passed

echo [7/7] building the .ui example and screenshots ...
if not exist "%OUT%\form" mkdir "%OUT%\form"
"%OUT%\qtcuic.exe" "%ROOT%\examples\form\Form1.ui" -o "%OUT%\form\ui_form1.h" -q || exit /b 1
cl %CFLAGS% /I"%OUT%\form" /c "%ROOT%\examples\form\main.cpp" /Fo"%OBJ%\form_main.obj" >> "%LOG%" 2>&1 || (echo form compile FAILED & exit /b 1)
link /nologo /out:"%OUT%\wf_form.exe" "%OBJ%\form_main.obj" "%OUT%\wf.lib" %LIBS% >> "%LOG%" 2>&1 || (echo form link FAILED & exit /b 1)
echo       %OUT%\wf_form.exe

if not exist "%OUT%\shots" mkdir "%OUT%\shots"
"%OUT%\gallery.exe" --screenshot "%OUT%\shots\gallery_light.png" >nul
"%OUT%\gallery.exe" --screenshot "%OUT%\shots\gallery_dark.png" --theme dark >nul
"%OUT%\wfdesigner.exe" --screenshot "%OUT%\shots\designer_light.png" >nul
"%OUT%\wfdesigner.exe" --screenshot "%OUT%\shots\designer_dark.png" --theme dark >nul
"%OUT%\wfdesigner.exe" --open "%ROOT%\examples\form\Form1.ui" --screenshot "%OUT%\shots\designer_form.png" >nul
"%OUT%\wfdesigner.exe" --open "%ROOT%\examples\form\Form1.ui" --theme dark --screenshot "%OUT%\shots\designer_form_dark.png" >nul
"%OUT%\wf_form.exe" --screenshot "%OUT%\shots\form_light.png" >nul
echo       screenshots in %OUT%\shots

:done
echo.
echo done.  binaries in "%OUT%"
exit /b 0
