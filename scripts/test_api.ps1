#!/usr/bin/env powershell

# FileServer API Test Script

$ServerUrl = "http://localhost:8080"
Write-Host "Testing FileServer API at $ServerUrl" -ForegroundColor Green
Write-Host "==================================" -ForegroundColor Green

# Test 1: Health Check
Write-Host "1. Testing health endpoint..." -ForegroundColor Yellow
try {
    $healthResponse = Invoke-RestMethod -Uri "$ServerUrl/health" -Method Get -TimeoutSec 5
    Write-Host "Health check successful!" -ForegroundColor Green
    $healthResponse | ConvertTo-Json -Depth 3
} catch {
    Write-Host "Health check failed: $($_.Exception.Message)" -ForegroundColor Red
}
Write-Host ""

# Test 2: Login
Write-Host "2. Testing authentication..." -ForegroundColor Yellow
try {
    $loginBody = @{
        username = "admin"
        password = "admin123"
    } | ConvertTo-Json

    $loginResponse = Invoke-RestMethod -Uri "$ServerUrl/api/v1/auth/login" -Method Post -Body $loginBody -ContentType "application/json" -TimeoutSec 5
    
    if ($loginResponse.success -and $loginResponse.token) {
        $token = $loginResponse.token
        Write-Host "Login successful! Token: $($token.Substring(0, [Math]::Min(20, $token.Length)))..." -ForegroundColor Green
    } else {
        Write-Host "Login failed!" -ForegroundColor Red
        exit 1
    }
} catch {
    Write-Host "Login failed: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
Write-Host ""

# Test 3: List files
Write-Host "3. Testing file listing..." -ForegroundColor Yellow
try {
    $headers = @{
        "Authorization" = "Bearer $token"
    }
    
    $filesResponse = Invoke-RestMethod -Uri "$ServerUrl/api/v1/files" -Method Get -Headers $headers -TimeoutSec 5
    Write-Host "File listing successful!" -ForegroundColor Green
    $filesResponse | ConvertTo-Json -Depth 3
} catch {
    Write-Host "File listing failed: $($_.Exception.Message)" -ForegroundColor Red
}
Write-Host ""

# Test 4: Create test file
Write-Host "4. Creating test file..." -ForegroundColor Yellow
$testContent = "Hello, FileServer!"
$testContent | Out-File -FilePath "test_file.txt" -Encoding UTF8
Write-Host "Test file created." -ForegroundColor Green
Write-Host ""

# Test 5: Upload file (simplified - in real scenario would use multipart)
Write-Host "5. Testing file upload..." -ForegroundColor Yellow
try {
    # Note: PowerShell multipart upload is more complex
    # This is a simplified version for demonstration
    Write-Host "File upload test skipped (requires multipart implementation)" -ForegroundColor Yellow
} catch {
    Write-Host "File upload failed: $($_.Exception.Message)" -ForegroundColor Red
}
Write-Host ""

# Test 6: Download file (if it exists)
Write-Host "6. Testing file download..." -ForegroundColor Yellow
try {
    $downloadResponse = Invoke-WebRequest -Uri "$ServerUrl/api/v1/files/download/test_file.txt" -Headers $headers -TimeoutSec 5
    
    if ($downloadResponse.StatusCode -eq 200) {
        Write-Host "File download successful!" -ForegroundColor Green
        $downloadResponse.Content | Out-File -FilePath "downloaded_file.txt" -Encoding UTF8
        Write-Host "Content: $(Get-Content downloaded_file.txt)" -ForegroundColor Cyan
    }
} catch {
    Write-Host "File download failed: $($_.Exception.Message)" -ForegroundColor Red
}
Write-Host ""

# Test 7: Delete file
Write-Host "7. Testing file deletion..." -ForegroundColor Yellow
try {
    $deleteResponse = Invoke-RestMethod -Uri "$ServerUrl/api/v1/files/test_file.txt" -Method Delete -Headers $headers -TimeoutSec 5
    Write-Host "File deletion successful!" -ForegroundColor Green
    $deleteResponse | ConvertTo-Json -Depth 3
} catch {
    Write-Host "File deletion failed: $($_.Exception.Message)" -ForegroundColor Red
}
Write-Host ""

# Cleanup
Write-Host "Cleaning up test files..." -ForegroundColor Yellow
Remove-Item -Path "test_file.txt" -ErrorAction SilentlyContinue
Remove-Item -Path "downloaded_file.txt" -ErrorAction SilentlyContinue

Write-Host "API testing completed!" -ForegroundColor Green
