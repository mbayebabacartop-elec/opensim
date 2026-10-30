@echo off
setlocal
set "QT_ROOT=C:\Qt\6.10.3\msvc2022_64"
set "APP=%~dp0build\windows-msvc-release\Release\openlab_app.exe"
if not exist "%APP%" (
  echo Build Release introuvable. Lancez cmake --build --preset windows-msvc-release.
  exit /b 1
)
if not exist "%QT_ROOT%\bin\Qt6Widgets.dll" (
  echo Qt Widgets 6.10.3 est introuvable dans "%QT_ROOT%".
  exit /b 1
)
set "PATH=%QT_ROOT%\bin;%PATH%"
set "QT_PLUGIN_PATH=%QT_ROOT%\plugins"
start "OpenLab Simulator" "%APP%"
endlocal
