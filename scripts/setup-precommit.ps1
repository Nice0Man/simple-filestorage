# Script to setup pre-commit hooks for the project on Windows
# Requires Python 3 and pip to be installed

$ErrorActionPreference = "Stop"

Write-Host "Setting up pre-commit hooks..." -ForegroundColor Cyan

# Check if Python is installed
try {
    $pythonVersion = python --version 2>&1
    Write-Host "Found: $pythonVersion" -ForegroundColor Green
} catch {
    Write-Host "Error: Python 3 is required but not installed." -ForegroundColor Red
    Write-Host "Please install Python 3 from https://www.python.org/ and try again." -ForegroundColor Yellow
    exit 1
}

# Check if pip is installed
try {
    $pipVersion = pip --version 2>&1
    Write-Host "Found pip: $pipVersion" -ForegroundColor Green
} catch {
    Write-Host "Error: pip is required but not installed." -ForegroundColor Red
    Write-Host "Please install pip and try again." -ForegroundColor Yellow
    exit 1
}

# Install pre-commit
Write-Host "`nInstalling pre-commit..." -ForegroundColor Cyan
try {
    pip install --user pre-commit
    Write-Host "pre-commit installed successfully" -ForegroundColor Green
} catch {
    Write-Host "Warning: Failed to install pre-commit, it might already be installed" -ForegroundColor Yellow
}

# Navigate to project root
$projectRoot = Split-Path -Parent $PSScriptRoot
Set-Location $projectRoot

# Install pre-commit hooks
Write-Host "`nInstalling pre-commit hooks..." -ForegroundColor Cyan
pre-commit install
pre-commit install --hook-type commit-msg

# Generate secrets baseline if it doesn't exist
if (-not (Test-Path ".secrets.baseline")) {
    Write-Host "`nGenerating secrets baseline..." -ForegroundColor Cyan
    try {
        detect-secrets scan | Out-File -FilePath ".secrets.baseline" -Encoding UTF8
    } catch {
        Write-Host "Warning: Could not generate secrets baseline" -ForegroundColor Yellow
        '{}' | Out-File -FilePath ".secrets.baseline" -Encoding UTF8
    }
}

# Run pre-commit on all files to check setup
Write-Host "`nRunning pre-commit on all files to verify setup..." -ForegroundColor Cyan
try {
    pre-commit run --all-files
} catch {
    Write-Host "Some checks failed. This is normal for first setup." -ForegroundColor Yellow
}

Write-Host "`n" + "="*60 -ForegroundColor Green
Write-Host "✓ Pre-commit hooks installed successfully!" -ForegroundColor Green
Write-Host "="*60 -ForegroundColor Green

Write-Host "`nUsage:" -ForegroundColor Cyan
Write-Host "  - Hooks will run automatically on 'git commit'" -ForegroundColor White
Write-Host "  - To run manually: pre-commit run --all-files" -ForegroundColor White
Write-Host "  - To update hooks: pre-commit autoupdate" -ForegroundColor White
Write-Host "  - To skip hooks (not recommended): git commit --no-verify" -ForegroundColor White
Write-Host ""

