# Script to test CI workflows locally on Windows
$ErrorActionPreference = "Continue"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
Set-Location $ProjectRoot

Write-Host "================================================" -ForegroundColor Cyan
Write-Host "Testing CI Workflows Locally" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

function Print-Status {
    param([bool]$Success, [string]$Message)
    if ($Success) {
        Write-Host "✓ " -NoNewline -ForegroundColor Green
    } else {
        Write-Host "✗ " -NoNewline -ForegroundColor Red
    }
    Write-Host $Message
}

function Print-Warning {
    param([string]$Message)
    Write-Host "⚠ $Message" -ForegroundColor Yellow
}

function Print-Info {
    param([string]$Message)
    Write-Host "ℹ $Message" -ForegroundColor Cyan
}

# Check prerequisites
Write-Host "Checking prerequisites..." -ForegroundColor Cyan
Write-Host ""

function Check-Tool {
    param([string]$Tool)
    try {
        $null = Get-Command $Tool -ErrorAction Stop
        Print-Status $true "$Tool is installed"
        return $true
    } catch {
        Print-Status $false "$Tool is not installed"
        return $false
    }
}

Check-Tool "cmake"
Check-Tool "g++"
Check-Tool "clang-format"
Check-Tool "cppcheck"

Write-Host ""
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "Step 1: Validate YAML Syntax" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

if (Get-Command python -ErrorAction SilentlyContinue) {
    $yamlFiles = @(
        ".github/workflows/ci.yml",
        ".github/workflows/pr-check.yml",
        ".github/workflows/release.yml",
        ".pre-commit-config.yaml"
    )
    
    foreach ($file in $yamlFiles) {
        try {
            $null = python -c "import yaml; yaml.safe_load(open('$file'))" 2>&1
            if ($LASTEXITCODE -eq 0) {
                Print-Status $true $file
            } else {
                Print-Status $false $file
            }
        } catch {
            Print-Status $false $file
        }
    }
} else {
    Print-Warning "Python not found, skipping YAML validation"
}

Write-Host ""
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "Step 2: Code Formatting Check" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

if (Get-Command clang-format -ErrorAction SilentlyContinue) {
    Write-Host "Checking C++ code formatting..."
    
    $files = Get-ChildItem -Path src,include,tests -Recurse -Include *.cpp,*.h,*.hpp -ErrorAction SilentlyContinue
    
    if ($files) {
        $hasIssues = $false
        foreach ($file in $files) {
            $result = clang-format --dry-run --Werror $file.FullName 2>&1
            if ($LASTEXITCODE -ne 0) {
                $hasIssues = $true
            }
        }
        
        if (-not $hasIssues) {
            Print-Status $true "All files are properly formatted"
        } else {
            Print-Status $false "Some files need formatting"
            Print-Info "Run: Get-ChildItem -Recurse -Include *.cpp,*.h,*.hpp | ForEach-Object { clang-format -i `$_.FullName }"
        }
    }
} else {
    Print-Warning "clang-format not found, skipping format check"
}

Write-Host ""
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "Step 3: Static Analysis (cppcheck)" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

if (Get-Command cppcheck -ErrorAction SilentlyContinue) {
    Write-Host "Running cppcheck..."
    $cppcheckArgs = @(
        "--enable=warning,style,performance,portability",
        "--suppress=missingIncludeSystem",
        "--suppress=unusedFunction",
        "--inline-suppr",
        "--quiet",
        "-I", "include",
        "src/"
    )
    
    $output = & cppcheck $cppcheckArgs 2>&1
    if ($output) {
        Print-Warning "Found some warnings:"
        Write-Host $output
    } else {
        Print-Status $true "No issues found"
    }
} else {
    Print-Warning "cppcheck not found, skipping static analysis"
}

Write-Host ""
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "Step 4: Build Test (Debug)" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

if (Test-Path "build") {
    Print-Warning "Build directory exists, cleaning..."
    Remove-Item -Recurse -Force build
}

Write-Host "Configuring CMake (Debug)..."
cmake -B build -DCMAKE_BUILD_TYPE=Debug 2>&1 | Out-File -FilePath "$env:TEMP\cmake-config.log"

if ($LASTEXITCODE -eq 0) {
    Print-Status $true "CMake configuration successful"
    
    Write-Host "Building project..."
    cmake --build build 2>&1 | Out-File -FilePath "$env:TEMP\cmake-build.log"
    
    if ($LASTEXITCODE -eq 0) {
        Print-Status $true "Build successful"
    } else {
        Print-Status $false "Build failed"
        Write-Host ""
        Write-Host "Last 20 lines of build log:"
        Get-Content "$env:TEMP\cmake-build.log" | Select-Object -Last 20
    }
} else {
    Print-Status $false "CMake configuration failed"
    Write-Host ""
    Write-Host "Configuration log:"
    Get-Content "$env:TEMP\cmake-config.log"
}

Write-Host ""
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "Step 5: Run Tests" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

if ((Test-Path "build") -and (Test-Path "build/bin/fileserver_tests.exe")) {
    Write-Host "Running unit tests..."
    Set-Location build
    ctest --output-on-failure --verbose
    
    if ($LASTEXITCODE -eq 0) {
        Print-Status $true "All tests passed"
    } else {
        Print-Status $false "Some tests failed"
    }
    Set-Location ..
} else {
    Print-Warning "Test binary not found, skipping tests"
}

Write-Host ""
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "Step 6: Build Test (Release)" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue

Write-Host "Configuring CMake (Release)..."
cmake -B build -DCMAKE_BUILD_TYPE=Release 2>&1 | Out-File -FilePath "$env:TEMP\cmake-config-release.log"

if ($LASTEXITCODE -eq 0) {
    Print-Status $true "CMake configuration successful"
    
    Write-Host "Building project..."
    cmake --build build 2>&1 | Out-File -FilePath "$env:TEMP\cmake-build-release.log"
    
    if ($LASTEXITCODE -eq 0) {
        Print-Status $true "Release build successful"
    } else {
        Print-Status $false "Release build failed"
    }
} else {
    Print-Status $false "CMake configuration failed"
}

Write-Host ""
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "Step 7: Docker Build Test (optional)" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

if (Get-Command docker -ErrorAction SilentlyContinue) {
    if (Test-Path "Dockerfile") {
        Write-Host "Building Docker image..."
        docker build -t fileserver:test . 2>&1 | Out-File -FilePath "$env:TEMP\docker-build.log"
        
        if ($LASTEXITCODE -eq 0) {
            Print-Status $true "Docker build successful"
            
            Write-Host "Testing Docker image..."
            $dockerTest = docker run --rm fileserver:test 2>&1
            Print-Status $true "Docker image runs"
        } else {
            Print-Status $false "Docker build failed"
            Write-Host ""
            Write-Host "Last 20 lines of Docker build log:"
            Get-Content "$env:TEMP\docker-build.log" | Select-Object -Last 20
        }
    } else {
        Print-Warning "Dockerfile not found, skipping Docker build"
    }
} else {
    Print-Warning "Docker not found, skipping Docker build test"
}

Write-Host ""
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "Summary" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""
Print-Info "Local CI checks completed!"
Write-Host ""
Write-Host "Next steps:"
Write-Host "  1. Fix any issues found above"
Write-Host "  2. Commit your changes: git add . && git commit"
Write-Host "  3. Push to GitHub: git push"
Write-Host "  4. Check GitHub Actions tab for CI results"
Write-Host ""
Print-Info "To run workflows locally with act:"
Write-Host "  Install: https://github.com/nektos/act"
Write-Host "  Run: act -l  # list workflows"
Write-Host "  Run: act push  # simulate push event"
Write-Host ""

