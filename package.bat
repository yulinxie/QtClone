@echo off
REM Creates a redistributable SDK layout in dist\
setlocal
set "ROOT=%~dp0"
if "%ROOT:~-1%"=="\" set "ROOT=%ROOT:~0,-1%"
cd /d "%ROOT%"

if not exist "%ROOT%\out\wf.lib" (
    echo ERROR: out\wf.lib not found. Run build.bat first.
    exit /b 1
)

set "DIST=%ROOT%\dist"
if exist "%DIST%" rmdir /s /q "%DIST%"
mkdir "%DIST%\include\qtc"
mkdir "%DIST%\lib"
mkdir "%DIST%\bin"

xcopy "%ROOT%\include\qtc\*.h" "%DIST%\include\qtc\" /Y >nul
copy /y "%ROOT%\out\wf.lib" "%DIST%\lib\" >nul
copy /y "%ROOT%\out\qtcuic.exe" "%DIST%\bin\" >nul
copy /y "%ROOT%\out\wfdesigner.exe" "%DIST%\bin\" >nul
copy /y "%ROOT%\out\gallery.exe" "%DIST%\bin\" >nul
copy /y "%ROOT%\README.md" "%DIST%\" >nul
copy /y "%ROOT%\LICENSE" "%DIST%\" >nul
xcopy "%ROOT%\docs" "%DIST%\docs\" /E /I /Y >nul
xcopy "%ROOT%\examples" "%DIST%\examples\" /E /I /Y >nul
if exist "%ROOT%\out\shots" xcopy "%ROOT%\out\shots" "%DIST%\screenshots\" /E /I /Y >nul

echo.
echo dist\ created:
echo   include\qtc\   headers
echo   lib\wf.lib     import-free static library
echo   bin\           qtcuic.exe, wfdesigner.exe, gallery.exe
echo   docs\          architecture, reflection, styling, designer, vs2010
echo   examples\      .ui + generated-code sample
exit /b 0
