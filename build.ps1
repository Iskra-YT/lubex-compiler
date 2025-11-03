if (Test-Path build) {
    Remove-Item -Recurse -Force build
}

New-Item -ItemType Directory -Path build -ErrorAction SilentlyContinue | Out-Null
Set-Location build
cmake ..
cmake --build .
Set-Location ..
