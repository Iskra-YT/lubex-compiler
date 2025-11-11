if (Test-Path build) {
    Remove-Item -Recurse -Force build
}

New-Item -ItemType Directory -Path build -ErrorAction SilentlyContinue | Out-Null
Set-Location build
cmake .. $args
cmake --build .
Set-Location ..
