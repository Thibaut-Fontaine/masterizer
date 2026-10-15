# Configure, build and test on Windows with the MSVC preset.
# Usage:  ./scripts/build.ps1
$ErrorActionPreference = "Stop"

cmake --preset windows-msvc
cmake --build --preset windows-msvc --config Release
ctest --preset windows-msvc
Write-Host "Done. CLI: .\build\windows-msvc\app\Release\masterizer_cli.exe <ref.wav> <src.wav> <out.wav> [intensity]"
