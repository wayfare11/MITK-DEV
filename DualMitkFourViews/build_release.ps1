$ErrorActionPreference = "Stop"

$projectDir = $PSScriptRoot
$buildDir = Join-Path $projectDir "build"
$mitkDir = if ($env:MITK_DIR) { $env:MITK_DIR } else { "D:\mitk_202312_build\MITK-build" }
$qt5Dir = if ($env:Qt5_DIR) { $env:Qt5_DIR } else { "D:\ProgramData\Qt\5.15.2\msvc2019_64\lib\cmake\Qt5" }

cmake -S $projectDir -B $buildDir -G "Visual Studio 17 2022" -A x64 `
  -DMITK_DIR="$mitkDir" `
  -DQt5_DIR="$qt5Dir"

cmake --build $buildDir --config Release
