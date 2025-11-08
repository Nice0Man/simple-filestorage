# Admin Manager Script for FileServer
param(
    [switch]$CreateAdmin,
    [switch]$ListUsers,
    [switch]$TestEndpoints,
    [switch]$ResetPassword,
    [string]$Username = "",
    [string]$Password = "",
    [string]$Email = "",
    [string]$Role = "admin"
)

$DatabaseContainer = "fileserver-postgres"
$DatabaseName = "fileserver"
$DatabaseUser = "fileserver"
$ApiBaseUrl = "http://localhost:8080"

function Write-Header($message) {
    Write-Host "`n>> $message" -ForegroundColor Green
    Write-Host ("=" * 50) -ForegroundColor Gray
}

function Write-Info($message) {
    Write-Host "[INFO] $message" -ForegroundColor Cyan
}

function Write-Success($message) {
    Write-Host "[OK] $message" -ForegroundColor Green
}

function Write-Error($message) {
    Write-Host "[ERROR] $message" -ForegroundColor Red
}

function Test-DatabaseConnection {
    Write-Info "Testing database connection..."
    $result = docker exec $DatabaseContainer pg_isready -U $DatabaseUser -d $DatabaseName 2>$null
    if ($LASTEXITCODE -eq 0) {
        Write-Success "Database connection: OK"
        return $true
    } else {
        Write-Error "Database connection failed"
        return $false
    }
}

function Test-ServerConnection {
    Write-Info "Testing server connection..."
    try {
        $response = Invoke-WebRequest -Uri "$ApiBaseUrl/health" -UseBasicParsing -TimeoutSec 5
        if ($response.StatusCode -eq 200) {
            Write-Success "Server connection: OK"
            return $true
        }
    } catch {
        Write-Error "Server connection failed: $($_.Exception.Message)"
        return $false
    }
    return $false
}

function Create-AdminUser {
    param(
        [string]$Username,
        [string]$Password,
        [string]$Email,
        [string]$Role
    )
    
    Write-Header "Creating Admin User"
    
    if (-not (Test-DatabaseConnection)) {
        return $false
    }
    
    # Generate salt and hash password
    $Salt = -join ((1..16) | ForEach {'{0:x}' -f (Get-Random -Max 16)})
    $PasswordWithSalt = $Password + $Salt
    $PasswordBytes = [System.Text.Encoding]::UTF8.GetBytes($PasswordWithSalt)
    $HashBytes = [System.Security.Cryptography.SHA256]::Create().ComputeHash($PasswordBytes)
    $PasswordHash = -join ($HashBytes | ForEach {'{0:x2}' -f $_})
    
    Write-Info "Generated password hash for user: $Username"
    Write-Info "Salt: $Salt"
    
    # Create SQL command
    $SqlCommand = @"
INSERT INTO users (username, email, password_hash, salt, role, is_active)
VALUES ('$Username', '$Email', '$PasswordHash', '$Salt', '$Role', true)
ON CONFLICT (username) DO UPDATE SET
    email = EXCLUDED.email,
    password_hash = EXCLUDED.password_hash,
    salt = EXCLUDED.salt,
    role = EXCLUDED.role,
    updated_at = CURRENT_TIMESTAMP;
"@
    
    Write-Info "Executing SQL command..."
    $result = $SqlCommand | docker exec -i $DatabaseContainer psql -U $DatabaseUser -d $DatabaseName
    
    if ($LASTEXITCODE -eq 0) {
        Write-Success "User '$Username' created/updated successfully"
        
        # Log the action
        $LogSql = @"
SELECT log_user_action(
    (SELECT id FROM users WHERE username = '$Username'),
    'USER_CREATED',
    'USER',
    (SELECT id FROM users WHERE username = '$Username'),
    '{"created_by": "admin_script", "role": "$Role"}'::jsonb
);
"@
        $LogSql | docker exec -i $DatabaseContainer psql -U $DatabaseUser -d $DatabaseName > $null
        
        return $true
    } else {
        Write-Error "Failed to create user '$Username'"
        return $false
    }
}

function Get-UsersList {
    Write-Header "Users List"
    
    if (-not (Test-DatabaseConnection)) {
        return
    }
    
    $SqlCommand = @"
SELECT 
    username,
    email,
    role,
    is_active,
    created_at,
    last_login,
    login_attempts
FROM users 
ORDER BY created_at DESC;
"@
    
    Write-Info "Fetching users from database..."
    $result = $SqlCommand | docker exec -i $DatabaseContainer psql -U $DatabaseUser -d $DatabaseName
    
    if ($LASTEXITCODE -eq 0) {
        Write-Success "Users retrieved successfully"
    } else {
        Write-Error "Failed to retrieve users"
    }
}

function Test-AllEndpoints {
    Write-Header "Testing All API Endpoints"
    
    if (-not (Test-ServerConnection)) {
        return
    }
    
    $TestResults = @()
    
    # Test 1: Health Check
    Write-Info "Testing health endpoint..."
    try {
        $response = Invoke-WebRequest -Uri "$ApiBaseUrl/health" -UseBasicParsing
        if ($response.StatusCode -eq 200) {
            Write-Success "Health endpoint: OK"
            $TestResults += @{Endpoint = "/health"; Status = "PASS"; Code = $response.StatusCode}
        }
    } catch {
        Write-Error "Health endpoint failed: $($_.Exception.Message)"
        $TestResults += @{Endpoint = "/health"; Status = "FAIL"; Error = $_.Exception.Message}
    }
    
    # Test 2: Authentication
    Write-Info "Testing authentication endpoint..."
    $AuthToken = $null
    try {
        $loginBody = @{
            username = "admin"
            password = "admin123"
        } | ConvertTo-Json
        
        $authResponse = Invoke-WebRequest -Uri "$ApiBaseUrl/api/v1/auth/login" -Method POST -Body $loginBody -ContentType "application/json" -UseBasicParsing
        
        if ($authResponse.StatusCode -eq 200) {
            $authData = $authResponse.Content | ConvertFrom-Json
            if ($authData.token) {
                $AuthToken = $authData.token
                Write-Success "Authentication: OK - Token received"
                $TestResults += @{Endpoint = "/api/v1/auth/login"; Status = "PASS"; Code = $authResponse.StatusCode}
            } else {
                Write-Error "Authentication: No token in response"
                $TestResults += @{Endpoint = "/api/v1/auth/login"; Status = "FAIL"; Error = "No token received"}
            }
        }
    } catch {
        Write-Error "Authentication failed: $($_.Exception.Message)"
        $TestResults += @{Endpoint = "/api/v1/auth/login"; Status = "FAIL"; Error = $_.Exception.Message}
    }
    
    if ($AuthToken) {
        $headers = @{Authorization = "Bearer $AuthToken"}
        
        # Test 3: List Files
        Write-Info "Testing file listing endpoint..."
        try {
            $filesResponse = Invoke-WebRequest -Uri "$ApiBaseUrl/api/v1/files" -Headers $headers -UseBasicParsing
            if ($filesResponse.StatusCode -eq 200) {
                Write-Success "File listing: OK"
                $TestResults += @{Endpoint = "/api/v1/files"; Status = "PASS"; Code = $filesResponse.StatusCode}
                
                $filesData = $filesResponse.Content | ConvertFrom-Json
                Write-Info "Files found: $($filesData.files.Count)"
            }
        } catch {
            Write-Error "File listing failed: $($_.Exception.Message)"
            $TestResults += @{Endpoint = "/api/v1/files"; Status = "FAIL"; Error = $_.Exception.Message}
        }
        
        # Test 4: File Upload
        Write-Info "Testing file upload endpoint..."
        try {
            # Create a test file
            $testContent = "This is a test file created by admin_manager.ps1 at $(Get-Date)"
            $testFileName = "test_upload_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"
            
            # Create multipart form data
            $boundary = [System.Guid]::NewGuid().ToString()
            $LF = "`r`n"
            
            $bodyLines = @(
                "--$boundary",
                "Content-Disposition: form-data; name=`"file`"; filename=`"$testFileName`"",
                "Content-Type: text/plain$LF",
                $testContent,
                "--$boundary",
                "Content-Disposition: form-data; name=`"path`"$LF",
                $testFileName,
                "--$boundary--$LF"
            )
            
            $body = $bodyLines -join $LF
            $contentType = "multipart/form-data; boundary=$boundary"
            
            $uploadResponse = Invoke-WebRequest -Uri "$ApiBaseUrl/api/v1/files/upload" -Method POST -Body $body -ContentType $contentType -Headers $headers -UseBasicParsing
            
            if ($uploadResponse.StatusCode -eq 200 -or $uploadResponse.StatusCode -eq 201) {
                Write-Success "File upload: OK"
                $TestResults += @{Endpoint = "/api/v1/files/upload"; Status = "PASS"; Code = $uploadResponse.StatusCode}
            }
        } catch {
            Write-Error "File upload failed: $($_.Exception.Message)"
            $TestResults += @{Endpoint = "/api/v1/files/upload"; Status = "FAIL"; Error = $_.Exception.Message}
        }
        
        # Test 5: File Download
        Write-Info "Testing file download endpoint..."
        try {
            $downloadResponse = Invoke-WebRequest -Uri "$ApiBaseUrl/api/v1/files/download/$testFileName" -Headers $headers -UseBasicParsing
            if ($downloadResponse.StatusCode -eq 200) {
                Write-Success "File download: OK"
                $TestResults += @{Endpoint = "/api/v1/files/download"; Status = "PASS"; Code = $downloadResponse.StatusCode}
                Write-Info "Downloaded content length: $($downloadResponse.Content.Length) bytes"
            }
        } catch {
            Write-Error "File download failed: $($_.Exception.Message)"
            $TestResults += @{Endpoint = "/api/v1/files/download"; Status = "FAIL"; Error = $_.Exception.Message}
        }
        
        # Test 6: File Delete
        Write-Info "Testing file delete endpoint..."
        try {
            $deleteResponse = Invoke-WebRequest -Uri "$ApiBaseUrl/api/v1/files/$testFileName" -Method DELETE -Headers $headers -UseBasicParsing
            if ($deleteResponse.StatusCode -eq 200 -or $deleteResponse.StatusCode -eq 204) {
                Write-Success "File delete: OK"
                $TestResults += @{Endpoint = "/api/v1/files/delete"; Status = "PASS"; Code = $deleteResponse.StatusCode}
            }
        } catch {
            Write-Error "File delete failed: $($_.Exception.Message)"
            $TestResults += @{Endpoint = "/api/v1/files/delete"; Status = "FAIL"; Error = $_.Exception.Message}
        }
    } else {
        Write-Error "Skipping authenticated endpoints - no auth token available"
    }
    
    # Test 7: API Documentation
    Write-Info "Testing API documentation endpoints..."
    try {
        $docsResponse = Invoke-WebRequest -Uri "$ApiBaseUrl/docs" -UseBasicParsing
        if ($docsResponse.StatusCode -eq 200) {
            Write-Success "API docs: OK"
            $TestResults += @{Endpoint = "/docs"; Status = "PASS"; Code = $docsResponse.StatusCode}
        }
    } catch {
        Write-Error "API docs failed: $($_.Exception.Message)"
        $TestResults += @{Endpoint = "/docs"; Status = "FAIL"; Error = $_.Exception.Message}
    }
    
    try {
        $swaggerResponse = Invoke-WebRequest -Uri "$ApiBaseUrl/swagger.yaml" -UseBasicParsing
        if ($swaggerResponse.StatusCode -eq 200) {
            Write-Success "Swagger spec: OK"
            $TestResults += @{Endpoint = "/swagger.yaml"; Status = "PASS"; Code = $swaggerResponse.StatusCode}
        }
    } catch {
        Write-Error "Swagger spec failed: $($_.Exception.Message)"
        $TestResults += @{Endpoint = "/swagger.yaml"; Status = "FAIL"; Error = $_.Exception.Message}
    }
    
    # Test Summary
    Write-Header "Test Results Summary"
    $PassCount = ($TestResults | Where-Object {$_.Status -eq "PASS"}).Count
    $FailCount = ($TestResults | Where-Object {$_.Status -eq "FAIL"}).Count
    
    Write-Host "Total Tests: $($TestResults.Count)" -ForegroundColor White
    Write-Host "Passed: $PassCount" -ForegroundColor Green
    Write-Host "Failed: $FailCount" -ForegroundColor Red
    
    Write-Host "`nDetailed Results:" -ForegroundColor Yellow
    foreach ($result in $TestResults) {
        $status = if ($result.Status -eq "PASS") { "[PASS]" } else { "[FAIL]" }
        $color = if ($result.Status -eq "PASS") { "Green" } else { "Red" }
        
        if ($result.Code) {
            Write-Host "$status $($result.Endpoint) - HTTP $($result.Code)" -ForegroundColor $color
        } else {
            Write-Host "$status $($result.Endpoint) - $($result.Error)" -ForegroundColor $color
        }
    }
    
    if ($FailCount -eq 0) {
        Write-Success "All tests passed! FileServer is working correctly."
    } else {
        Write-Error "$FailCount test(s) failed. Please check the server configuration."
    }
}

function Reset-UserPassword {
    param(
        [string]$Username,
        [string]$NewPassword
    )
    
    Write-Header "Resetting Password for User: $Username"
    
    if (-not (Test-DatabaseConnection)) {
        return $false
    }
    
    # Generate new salt and hash
    $Salt = -join ((1..16) | ForEach {'{0:x}' -f (Get-Random -Max 16)})
    $PasswordWithSalt = $NewPassword + $Salt
    $PasswordBytes = [System.Text.Encoding]::UTF8.GetBytes($PasswordWithSalt)
    $HashBytes = [System.Security.Cryptography.SHA256]::Create().ComputeHash($PasswordBytes)
    $PasswordHash = -join ($HashBytes | ForEach {'{0:x2}' -f $_})
    
    $SqlCommand = @"
UPDATE users 
SET password_hash = '$PasswordHash', 
    salt = '$Salt', 
    login_attempts = 0,
    locked_until = NULL,
    updated_at = CURRENT_TIMESTAMP
WHERE username = '$Username';
"@
    
    Write-Info "Updating password for user: $Username"
    $result = $SqlCommand | docker exec -i $DatabaseContainer psql -U $DatabaseUser -d $DatabaseName
    
    if ($LASTEXITCODE -eq 0) {
        Write-Success "Password reset successfully for user: $Username"
        return $true
    } else {
        Write-Error "Failed to reset password for user: $Username"
        return $false
    }
}

# Main execution
Write-Header "FileServer Admin Manager"

if ($CreateAdmin) {
    if (-not $Username -or -not $Password) {
        Write-Error "Username and Password are required for creating admin user"
        Write-Host "Usage: .\admin_manager.ps1 -CreateAdmin -Username 'admin' -Password 'password123' -Email 'admin@example.com'"
        exit 1
    }
    
    if (-not $Email) {
        $Email = "$Username@fileserver.local"
    }
    
    Create-AdminUser -Username $Username -Password $Password -Email $Email -Role $Role
}

if ($ListUsers) {
    Get-UsersList
}

if ($TestEndpoints) {
    Test-AllEndpoints
}

if ($ResetPassword) {
    if (-not $Username -or -not $Password) {
        Write-Error "Username and Password are required for resetting password"
        Write-Host "Usage: .\admin_manager.ps1 -ResetPassword -Username 'admin' -Password 'newpassword123'"
        exit 1
    }
    
    Reset-UserPassword -Username $Username -NewPassword $Password
}

# Default action if no parameters
if (-not ($CreateAdmin -or $ListUsers -or $TestEndpoints -or $ResetPassword)) {
    Write-Host "FileServer Admin Manager" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Usage:" -ForegroundColor White
    Write-Host "  Create Admin User:" -ForegroundColor Cyan
    Write-Host "    .\admin_manager.ps1 -CreateAdmin -Username 'admin' -Password 'admin123' -Email 'admin@example.com'"
    Write-Host ""
    Write-Host "  List All Users:" -ForegroundColor Cyan
    Write-Host "    .\admin_manager.ps1 -ListUsers"
    Write-Host ""
    Write-Host "  Test All Endpoints:" -ForegroundColor Cyan
    Write-Host "    .\admin_manager.ps1 -TestEndpoints"
    Write-Host ""
    Write-Host "  Reset User Password:" -ForegroundColor Cyan
    Write-Host "    .\admin_manager.ps1 -ResetPassword -Username 'admin' -Password 'newpassword123'"
    Write-Host ""
    Write-Host "  Combined Operations:" -ForegroundColor Cyan
    Write-Host "    .\admin_manager.ps1 -CreateAdmin -Username 'superadmin' -Password 'secure123' -TestEndpoints"
    Write-Host ""
    Write-Host "Examples:" -ForegroundColor Yellow
    Write-Host "  # Create admin and test system"
    Write-Host "  .\admin_manager.ps1 -CreateAdmin -Username 'admin' -Password 'admin123' -TestEndpoints"
    Write-Host ""
    Write-Host "  # Quick system check"
    Write-Host "  .\admin_manager.ps1 -ListUsers -TestEndpoints"
}

Write-Host ""
