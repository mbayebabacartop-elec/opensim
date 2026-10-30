@echo off
setlocal
set "QT_ROOT=C:\Qt\6.10.3\msvc2022_64"
set "APP=%~dp0build\windows-msvc-release\Release\openlab_app.exe"
if not exist "%APP%" (
  echo Build Release introuvable. Lancez cmake --build --preset windows-msvc-release.
  exit /b 1
)
if not exist "%QT_ROOT%\bin\Qt6Quick.dll" (
  echo Qt Quick 6.10.3 est introuvable dans "%QT_ROOT%".
  exit /b 1
)
set "PATH=%QT_ROOT%\bin;%PATH%"
set "QT_PLUGIN_PATH=%QT_ROOT%\plugins"
set "QML_IMPORT_PATH=%QT_ROOT%\qml"
set "QML2_IMPORT_PATH=%QT_ROOT%\qml"
set "QT_QUICK_CONTROLS_STYLE=Basic"
start "OpenLab Simulator" "%APP%"
endlocal
