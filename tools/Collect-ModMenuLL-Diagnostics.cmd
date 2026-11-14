@echo off
setlocal
title ModMenuLL read-only diagnostics
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Collect-ModMenuLL-Diagnostics.ps1" -GameExe "%~1"
if errorlevel 1 echo Collection failed. Send the error text to the developer.
pause
