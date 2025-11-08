#!/usr/bin/env powershell

Write-Host "=== Building FileServer ===" -ForegroundColor Green

# Set location to script directory
Set-Location $PSScriptRoot

# Create necessary directories
New-Item -ItemType Directory -Force -Path "files", "logs", "config", "build" | Out-Null

# Copy configuration files
Copy-Item "config.json" "config/" -Force
Copy-Item "users.json" "config/" -Force

Write-Host "Building Docker image..." -ForegroundColor Yellow
$buildResult = docker build -t fileserver:latest .

if ($LASTEXITCODE -ne 0) {
    Write-Host "Failed to build Docker image" -ForegroundColor Red
    exit 1
}

Write-Host "=== Starting FileServer ===" -ForegroundColor Green

# Stop existing container if running
docker stop fileserver-container 2>$null
docker rm fileserver-container 2>$null

# Run container
$runResult = docker run -d `
  --name fileserver-container `
  -p 8080:8080 `
  -v "${PWD}/files:/app/files" `
  -v "${PWD}/logs:/app/logs" `
  -v "${PWD}/config:/app/config" `
  -e CONFIG_PATH=/app/config/config.json `
  -e LOG_LEVEL=INFO `
  fileserver:latest

if ($LASTEXITCODE -ne 0) {
    Write-Host "Failed to start container" -ForegroundColor Red
    exit 1
}

Write-Host "=== FileServer started successfully! ===" -ForegroundColor Green
Write-Host ""
Write-Host "Server is running on: http://localhost:8080" -ForegroundColor Cyan
Write-Host "Health check: http://localhost:8080/health" -ForegroundColor Cyan
Write-Host "API Documentation: http://localhost:8080/docs" -ForegroundColor Cyan
Write-Host ""
Write-Host "Default credentials:" -ForegroundColor Yellow
Write-Host "  Username: admin" -ForegroundColor White
Write-Host "  Password: admin123" -ForegroundColor White
Write-Host ""
Write-Host "To view logs: docker logs -f fileserver-container" -ForegroundColor Gray
Write-Host "To stop server: docker stop fileserver-container" -ForegroundColor Gray
Write-Host ""

# Wait a moment for server to start
Start-Sleep -Seconds 3

# Test health endpoint
Write-Host "Testing health endpoint..." -ForegroundColor Yellow
try {
    $response = Invoke-RestMethod -Uri "http://localhost:8080/health" -Method Get -TimeoutSec 5
    Write-Host "Health check successful!" -ForegroundColor Green
    $response | ConvertTo-Json -Depth 3
} catch {
    Write-Host "Health check failed - server may still be starting" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=== Setup complete! ===" -ForegroundColor Green
