$ErrorActionPreference = "Stop"

if ($env:MITK_2023_RELEASE) {
  $env:PATH = "$env:MITK_2023_RELEASE;$env:PATH"
}

$qtBin = "D:\ProgramData\Qt\5.15.2\msvc2019_64\bin"
if (Test-Path $qtBin) {
  $env:PATH = "$qtBin;$env:PATH"
}

$exe = Join-Path $PSScriptRoot "build\Release\DualMitkFourViews.exe"
if (!(Test-Path $exe)) {
  throw "Executable not found. Run build_release.ps1 first: $exe"
}

& $exe @args
