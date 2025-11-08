#!/usr/bin/env powershell

param(
    [switch]$Build,
    [switch]$Start,
    [switch]$Stop,
    [switch]$Test,
    [switch]$Logs,
    [switch]$Clean,
    [switch]$Help
)

$ContainerName = "fileserver-container"
$ImageName = "fileserver:latest"
$ServerUrl = "http://localhost:8080"

function Show-Help {
    Write-Host "FileServer Management Script" -ForegroundColor Green
    Write-Host "Usage: .\run_fileserver.ps1 [OPTIONS]" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Options:" -ForegroundColor Cyan
    Write-Host "  -Build    Build Docker image" -ForegroundColor White
    Write-Host "  -Start    Start the server" -ForegroundColor White
    Write-Host "  -Stop     Stop the server" -ForegroundColor White
    Write-Host "  -Test     Run API tests" -ForegroundColor White
    Write-Host "  -Logs     Show server logs" -ForegroundColor White
    Write-Host "  -Clean    Clean up containers and images" -ForegroundColor White
    Write-Host "  -Help     Show this help" -ForegroundColor White
    Write-Host ""
    Write-Host "Examples:" -ForegroundColor Cyan
    Write-Host "  .\run_fileserver.ps1 -Build -Start" -ForegroundColor Gray
    Write-Host "  .\run_fileserver.ps1 -Test" -ForegroundColor Gray
    Write-Host "  .\run_fileserver.ps1 -Stop -Clean" -ForegroundColor Gray
}

function Build-Image {
    Write-Host "Building FileServer Docker image..." -ForegroundColor Yellow
    
    # Create necessary directories
    New-Item -ItemType Directory -Force -Path "files", "logs", "config" | Out-Null
    Copy-Item "config.json" "config/" -Force -ErrorAction SilentlyContinue
    Copy-Item "users.json" "config/" -Force -ErrorAction SilentlyContinue
    
    docker build -t $ImageName .
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✅ Docker image built successfully!" -ForegroundColor Green
        return $true
    } else {
        Write-Host "❌ Failed to build Docker image" -ForegroundColor Red
        return $false
    }
}

function Start-Server {
    Write-Host "Starting FileServer..." -ForegroundColor Yellow
    
    # Stop existing container if running
    docker stop $ContainerName 2>$null | Out-Null
    docker rm $ContainerName 2>$null | Out-Null
    
    # Start new container
    $result = docker run -d `
        --name $ContainerName `
        -p 8080:8080 `
        -v "${PWD}/files:/app/files" `
        -v "${PWD}/logs:/app/logs" `
        -v "${PWD}/config:/app/config" `
        -e CONFIG_PATH=/app/config/config.json `
        -e LOG_LEVEL=INFO `
        $ImageName
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✅ FileServer started successfully!" -ForegroundColor Green
        Write-Host "🌐 Server URL: $ServerUrl" -ForegroundColor Cyan
        Write-Host "📋 Health check: $ServerUrl/health" -ForegroundColor Cyan
        Write-Host "📚 API docs: $ServerUrl/docs" -ForegroundColor Cyan
        
        # Wait for server to start
        Write-Host "Waiting for server to start..." -ForegroundColor Yellow
        Start-Sleep -Seconds 5
        
        # Test health endpoint
        try {
            $health = Invoke-RestMethod -Uri "$ServerUrl/health" -TimeoutSec 10
            Write-Host "✅ Health check passed!" -ForegroundColor Green
        } catch {
            Write-Host "⚠️  Health check failed, server may still be starting..." -ForegroundColor Yellow
        }
        
        return $true
    } else {
        Write-Host "❌ Failed to start FileServer" -ForegroundColor Red
        return $false
    }
}

function Stop-Server {
    Write-Host "Stopping FileServer..." -ForegroundColor Yellow
    
    docker stop $ContainerName 2>$null
    docker rm $ContainerName 2>$null
    
    Write-Host "✅ FileServer stopped" -ForegroundColor Green
}

function Test-API {
    Write-Host "Testing FileServer API..." -ForegroundColor Yellow
    
    # Check if server is running
    try {
        $health = Invoke-RestMethod -Uri "$ServerUrl/health" -TimeoutSec 5
        Write-Host "✅ Server is running" -ForegroundColor Green
    } catch {
        Write-Host "❌ Server is not responding. Please start the server first." -ForegroundColor Red
        return
    }
    
    # Test authentication
    Write-Host "Testing authentication..." -ForegroundColor Cyan
    try {
        $loginBody = @{
            username = "admin"
            password = "admin123"
        } | ConvertTo-Json
        
        $loginResponse = Invoke-RestMethod -Uri "$ServerUrl/api/v1/auth/login" -Method Post -Body $loginBody -ContentType "application/json" -TimeoutSec 10
        
        if ($loginResponse.success -and $loginResponse.token) {
            Write-Host "✅ Authentication successful" -ForegroundColor Green
            $token = $loginResponse.token
        } else {
            Write-Host "❌ Authentication failed" -ForegroundColor Red
            return
        }
    } catch {
        Write-Host "❌ Authentication test failed: $($_.Exception.Message)" -ForegroundColor Red
        return
    }
    
    # Test file listing
    Write-Host "Testing file listing..." -ForegroundColor Cyan
    try {
        $headers = @{ "Authorization" = "Bearer $token" }
        $filesResponse = Invoke-RestMethod -Uri "$ServerUrl/api/v1/files" -Method Get -Headers $headers -TimeoutSec 10
        Write-Host "✅ File listing successful" -ForegroundColor Green
    } catch {
        Write-Host "❌ File listing failed: $($_.Exception.Message)" -ForegroundColor Red
    }
    
    Write-Host "🎉 API testing completed!" -ForegroundColor Green
}

function Show-Logs {
    Write-Host "Showing FileServer logs..." -ForegroundColor Yellow
    docker logs -f $ContainerName
}

function Clean-Up {
    Write-Host "Cleaning up FileServer resources..." -ForegroundColor Yellow
    
    # Stop and remove container
    docker stop $ContainerName 2>$null | Out-Null
    docker rm $ContainerName 2>$null | Out-Null
    
    # Remove image
    docker rmi $ImageName 2>$null | Out-Null
    
    # Clean up build artifacts
    if (Test-Path "build") {
        Remove-Item -Recurse -Force "build"
    }
    
    Write-Host "✅ Cleanup completed" -ForegroundColor Green
}

# Main execution
if ($Help -or (-not ($Build -or $Start -or $Stop -or $Test -or $Logs -or $Clean))) {
    Show-Help
    exit 0
}

if ($Build) {
    if (-not (Build-Image)) {
        exit 1
    }
}

if ($Start) {
    if (-not (Start-Server)) {
        exit 1
    }
}

if ($Test) {
    Test-API
}

if ($Logs) {
    Show-Logs
}

if ($Stop) {
    Stop-Server
}

if ($Clean) {
    Clean-Up
}
