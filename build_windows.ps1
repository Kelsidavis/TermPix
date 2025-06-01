Write-Host "Building TermPix for Windows..." -ForegroundColor Green

# Check for GCC
$gcc = Get-Command gcc -ErrorAction SilentlyContinue
if (-not $gcc) {
    Write-Error "GCC compiler not found. Please install TDM-GCC or MinGW and ensure gcc is in your PATH."
    exit 1
}

Write-Host "Found GCC: $($gcc.Source)" -ForegroundColor Yellow

# Build command with flags for braille support
$buildCommand = @(
    'gcc',
    '-std=c99',                    # C99 standard for better compatibility
    '-Wall',                       # All warnings
    '-Wextra',                     # Extra warnings
    '-O2',                         # Optimization level 2
    '-finput-charset=UTF-8',       # Input charset for UTF-8 source
    '-fexec-charset=UTF-8',        # Execution charset for UTF-8 output
    '-o', 'termpix.exe',          # Output executable
    'src\main.c',                  # Source files
    'src\image.c',
    'src\render.c', 
    'src\terminal.c',
    '-Ilib',                       # Include directory
    '-lm'                          # Math library
) -join ' '

Write-Host "Build command:" -ForegroundColor Cyan
Write-Host $buildCommand -ForegroundColor Gray

# Execute build
Invoke-Expression $buildCommand

# Check result
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed with exit code $LASTEXITCODE"
    Write-Host "`nTroubleshooting tips:" -ForegroundColor Yellow
    Write-Host "1. Ensure you have a recent GCC version (7.0+)" -ForegroundColor White
    Write-Host "2. Check that all source files exist in src\" -ForegroundColor White
    Write-Host "3. Verify stb_image.h is in lib\" -ForegroundColor White
    Write-Host "4. Try installing TDM-GCC or MSYS2 if using an older MinGW" -ForegroundColor White
    exit $LASTEXITCODE
}

Write-Host "Build succeeded! Output: termpix.exe" -ForegroundColor Green

# Test the executable
if (Test-Path "termpix.exe") {
    Write-Host "`nTesting executable..." -ForegroundColor Cyan
    .\termpix.exe --help
    
    Write-Host "`nUsage examples:" -ForegroundColor Yellow
    Write-Host "  .\termpix.exe image.jpg" -ForegroundColor White
    Write-Host "  .\termpix.exe --dither --width 80 image.png" -ForegroundColor White
    Write-Host "  .\termpix.exe --height 40 photo.bmp" -ForegroundColor White
} else {
    Write-Warning "termpix.exe was not created successfully"
}