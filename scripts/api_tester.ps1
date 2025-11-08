# Comprehensive API Tester for FileServer
param(
    [switch]$Quick,
    [switch]$Full,
    [switch]$Performance,
    [switch]$Security,
    [string]$BaseUrl = "http://localhost:8080",
    [string]$Username = "admin",
    [string]$Password = "admin123"
)

$Global:TestResults = @()
$Global:AuthToken = $null

function Write-Header($message) {
    Write-Host "`n>> $message" -ForegroundColor Green
    Write-Host ("=" * 60) -ForegroundColor Gray
}

function Write-Info($message) {
    Write-Host "[INFO] $message" -ForegroundColor Cyan
}

function Write-Success($message) {
    Write-Host "[PASS] $message" -ForegroundColor Green
}

function Write-Error($message) {
    Write-Host "[FAIL] $message" -ForegroundColor Red
}

function Write-Warning($message) {
    Write-Host "[WARN] $message" -ForegroundColor Yellow
}

function Add-TestResult {
    param(
        [string]$TestName,
        [string]$Endpoint,
        [string]$Method,
        [string]$Status,
        [int]$ResponseTime = 0,
        [int]$StatusCode = 0,
        [string]$Error = ""
    )
    
    $Global:TestResults += [PSCustomObject]@{
        TestName = $TestName
        Endpoint = $Endpoint
        Method = $Method
        Status = $Status
        ResponseTime = $ResponseTime
        StatusCode = $StatusCode
        Error = $Error
        Timestamp = Get-Date
    }
}

function Test-Endpoint {
    param(
        [string]$TestName,
        [string]$Endpoint,
        [string]$Method = "GET",
        [hashtable]$Headers = @{},
        [string]$Body = "",
        [string]$ContentType = "application/json",
        [int]$ExpectedStatus = 200,
        [switch]$AllowRedirect
    )
    
    $fullUrl = "$BaseUrl$Endpoint"
    $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    
    try {
        $params = @{
            Uri = $fullUrl
            Method = $Method
            UseBasicParsing = $true
            TimeoutSec = 30
        }
        
        if ($Headers.Count -gt 0) {
            $params.Headers = $Headers
        }
        
        if ($Body -and $Method -in @("POST", "PUT", "PATCH")) {
            $params.Body = $Body
            $params.ContentType = $ContentType
        }
        
        if (-not $AllowRedirect) {
            $params.MaximumRedirection = 0
        }
        
        $response = Invoke-WebRequest @params
        $stopwatch.Stop()
        
        if ($response.StatusCode -eq $ExpectedStatus) {
            Write-Success "$TestName - HTTP $($response.StatusCode) ($($stopwatch.ElapsedMilliseconds)ms)"
            Add-TestResult -TestName $TestName -Endpoint $Endpoint -Method $Method -Status "PASS" -ResponseTime $stopwatch.ElapsedMilliseconds -StatusCode $response.StatusCode
            return $response
        } else {
            Write-Error "$TestName - Expected $ExpectedStatus, got $($response.StatusCode)"
            Add-TestResult -TestName $TestName -Endpoint $Endpoint -Method $Method -Status "FAIL" -ResponseTime $stopwatch.ElapsedMilliseconds -StatusCode $response.StatusCode -Error "Unexpected status code"
            return $null
        }
    } catch {
        $stopwatch.Stop()
        Write-Error "$TestName - $($_.Exception.Message)"
        Add-TestResult -TestName $TestName -Endpoint $Endpoint -Method $Method -Status "FAIL" -ResponseTime $stopwatch.ElapsedMilliseconds -Error $_.Exception.Message
        return $null
    }
}

function Get-AuthToken {
    Write-Info "Authenticating with server..."
    
    $loginBody = @{
        username = $Username
        password = $Password
    } | ConvertTo-Json
    
    $response = Test-Endpoint -TestName "Authentication" -Endpoint "/api/v1/auth/login" -Method "POST" -Body $loginBody -ExpectedStatus 200
    
    if ($response) {
        $authData = $response.Content | ConvertFrom-Json
        if ($authData.token) {
            $Global:AuthToken = $authData.token
            Write-Success "Authentication successful - Token obtained"
            return $true
        }
    }
    
    Write-Error "Authentication failed - Cannot proceed with authenticated tests"
    return $false
}

function Test-HealthEndpoints {
    Write-Header "Testing Health & Info Endpoints"
    
    Test-Endpoint -TestName "Health Check" -Endpoint "/health"
    Test-Endpoint -TestName "API Documentation" -Endpoint "/docs"
    Test-Endpoint -TestName "Swagger Spec" -Endpoint "/swagger.yaml"
}

function Test-AuthenticationEndpoints {
    Write-Header "Testing Authentication Endpoints"
    
    # Test valid login
    $loginBody = @{
        username = $Username
        password = $Password
    } | ConvertTo-Json
    
    Test-Endpoint -TestName "Valid Login" -Endpoint "/api/v1/auth/login" -Method "POST" -Body $loginBody
    
    # Test invalid login
    $invalidLoginBody = @{
        username = "invalid"
        password = "wrong"
    } | ConvertTo-Json
    
    Test-Endpoint -TestName "Invalid Login" -Endpoint "/api/v1/auth/login" -Method "POST" -Body $invalidLoginBody -ExpectedStatus 401
    
    # Test malformed request
    Test-Endpoint -TestName "Malformed Login" -Endpoint "/api/v1/auth/login" -Method "POST" -Body "invalid json" -ExpectedStatus 400
}

function Test-FileOperations {
    Write-Header "Testing File Operations"
    
    if (-not $Global:AuthToken) {
        Write-Warning "No auth token available - skipping file operations"
        return
    }
    
    $headers = @{Authorization = "Bearer $Global:AuthToken"}
    
    # Test file listing
    Test-Endpoint -TestName "List Files" -Endpoint "/api/v1/files" -Headers $headers
    
    # Test file upload
    $testContent = "Test file content created at $(Get-Date)"
    $testFileName = "api_test_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt"
    
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
    
    $uploadBody = $bodyLines -join $LF
    $uploadHeaders = $headers.Clone()
    
    Test-Endpoint -TestName "File Upload" -Endpoint "/api/v1/files/upload" -Method "POST" -Headers $uploadHeaders -Body $uploadBody -ContentType "multipart/form-data; boundary=$boundary" -ExpectedStatus 201
    
    # Test file download
    Test-Endpoint -TestName "File Download" -Endpoint "/api/v1/files/download/$testFileName" -Headers $headers
    
    # Test file delete
    Test-Endpoint -TestName "File Delete" -Endpoint "/api/v1/files/$testFileName" -Method "DELETE" -Headers $headers -ExpectedStatus 204
    
    # Test download non-existent file
    Test-Endpoint -TestName "Download Non-existent File" -Endpoint "/api/v1/files/download/nonexistent.txt" -Headers $headers -ExpectedStatus 404
}

function Test-SecurityFeatures {
    Write-Header "Testing Security Features"
    
    # Test unauthorized access
    Test-Endpoint -TestName "Unauthorized File Access" -Endpoint "/api/v1/files" -ExpectedStatus 401
    
    # Test invalid token
    $invalidHeaders = @{Authorization = "Bearer invalid_token_here"}
    Test-Endpoint -TestName "Invalid Token" -Endpoint "/api/v1/files" -Headers $invalidHeaders -ExpectedStatus 401
    
    # Test path traversal protection
    if ($Global:AuthToken) {
        $headers = @{Authorization = "Bearer $Global:AuthToken"}
        Test-Endpoint -TestName "Path Traversal Protection" -Endpoint "/api/v1/files/download/../../../etc/passwd" -Headers $headers -ExpectedStatus 400
    }
    
    # Test CORS headers
    $response = Test-Endpoint -TestName "CORS Headers Check" -Endpoint "/health"
    if ($response -and $response.Headers) {
        $corsHeader = $response.Headers["Access-Control-Allow-Origin"]
        if ($corsHeader) {
            Write-Success "CORS headers present: $corsHeader"
        } else {
            Write-Warning "CORS headers not found"
        }
    }
}

function Test-ErrorHandling {
    Write-Header "Testing Error Handling"
    
    # Test 404 endpoints
    Test-Endpoint -TestName "404 Not Found" -Endpoint "/nonexistent" -ExpectedStatus 404
    
    # Test method not allowed
    Test-Endpoint -TestName "Method Not Allowed" -Endpoint "/health" -Method "POST" -ExpectedStatus 405
    
    # Test large request (if server has limits)
    $largeBody = "x" * 10000
    Test-Endpoint -TestName "Large Request Body" -Endpoint "/api/v1/auth/login" -Method "POST" -Body $largeBody -ExpectedStatus 400
}

function Test-Performance {
    Write-Header "Testing Performance"
    
    if (-not $Global:AuthToken) {
        if (-not (Get-AuthToken)) {
            Write-Warning "Cannot perform performance tests without authentication"
            return
        }
    }
    
    $headers = @{Authorization = "Bearer $Global:AuthToken"}
    
    # Concurrent requests test
    Write-Info "Testing concurrent requests..."
    $jobs = @()
    
    for ($i = 1; $i -le 10; $i++) {
        $job = Start-Job -ScriptBlock {
            param($BaseUrl, $Headers)
            try {
                $response = Invoke-WebRequest -Uri "$BaseUrl/health" -Headers $Headers -UseBasicParsing -TimeoutSec 10
                return @{Success = $true; StatusCode = $response.StatusCode; Time = (Measure-Command {$response}).TotalMilliseconds}
            } catch {
                return @{Success = $false; Error = $_.Exception.Message}
            }
        } -ArgumentList $BaseUrl, $headers
        
        $jobs += $job
    }
    
    $results = $jobs | Wait-Job | Receive-Job
    $jobs | Remove-Job
    
    $successCount = ($results | Where-Object {$_.Success}).Count
    $avgTime = ($results | Where-Object {$_.Success} | Measure-Object -Property Time -Average).Average
    
    if ($successCount -eq 10) {
        Write-Success "Concurrent requests: $successCount/10 successful (avg: $([math]::Round($avgTime, 2))ms)"
    } else {
        Write-Warning "Concurrent requests: $successCount/10 successful"
    }
    
    Add-TestResult -TestName "Concurrent Requests" -Endpoint "/health" -Method "GET" -Status $(if($successCount -eq 10){"PASS"}else{"WARN"}) -ResponseTime $avgTime
}

function Show-TestSummary {
    Write-Header "Test Results Summary"
    
    $totalTests = $Global:TestResults.Count
    $passedTests = ($Global:TestResults | Where-Object {$_.Status -eq "PASS"}).Count
    $failedTests = ($Global:TestResults | Where-Object {$_.Status -eq "FAIL"}).Count
    $warnTests = ($Global:TestResults | Where-Object {$_.Status -eq "WARN"}).Count
    
    Write-Host "Total Tests: $totalTests" -ForegroundColor White
    Write-Host "Passed: $passedTests" -ForegroundColor Green
    Write-Host "Failed: $failedTests" -ForegroundColor Red
    Write-Host "Warnings: $warnTests" -ForegroundColor Yellow
    
    $successRate = [math]::Round(($passedTests / $totalTests) * 100, 2)
    Write-Host "Success Rate: $successRate%" -ForegroundColor $(if($successRate -ge 90){"Green"}elseif($successRate -ge 70){"Yellow"}else{"Red"})
    
    if ($failedTests -gt 0) {
        Write-Host "`nFailed Tests:" -ForegroundColor Red
        $Global:TestResults | Where-Object {$_.Status -eq "FAIL"} | ForEach-Object {
            Write-Host "  - $($_.TestName): $($_.Error)" -ForegroundColor Red
        }
    }
    
    # Performance summary
    $avgResponseTime = ($Global:TestResults | Where-Object {$_.ResponseTime -gt 0} | Measure-Object -Property ResponseTime -Average).Average
    if ($avgResponseTime) {
        Write-Host "`nAverage Response Time: $([math]::Round($avgResponseTime, 2))ms" -ForegroundColor Cyan
    }
    
    # Export results to JSON
    $resultsFile = "test_results_$(Get-Date -Format 'yyyyMMdd_HHmmss').json"
    $Global:TestResults | ConvertTo-Json -Depth 3 | Out-File $resultsFile
    Write-Info "Detailed results exported to: $resultsFile"
}

# Main execution
Write-Header "FileServer API Comprehensive Tester"
Write-Info "Target: $BaseUrl"
Write-Info "User: $Username"

if ($Quick) {
    Write-Info "Running quick test suite..."
    Test-HealthEndpoints
    if (Get-AuthToken) {
        Test-FileOperations
    }
} elseif ($Full) {
    Write-Info "Running full test suite..."
    Test-HealthEndpoints
    Test-AuthenticationEndpoints
    if (Get-AuthToken) {
        Test-FileOperations
        Test-SecurityFeatures
        Test-ErrorHandling
    }
} elseif ($Performance) {
    Write-Info "Running performance tests..."
    Test-HealthEndpoints
    Test-Performance
} elseif ($Security) {
    Write-Info "Running security tests..."
    Test-SecurityFeatures
    Test-AuthenticationEndpoints
} else {
    # Default: run basic tests
    Write-Info "Running basic test suite..."
    Test-HealthEndpoints
    Test-AuthenticationEndpoints
    if (Get-AuthToken) {
        Test-FileOperations
    }
}

Show-TestSummary

# Return appropriate exit code
$failedCount = ($Global:TestResults | Where-Object {$_.Status -eq "FAIL"}).Count
exit $failedCount

