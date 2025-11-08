# Complete FileServer System with PostgreSQL
param(
    [switch]$Build,
    [switch]$Start,
    [switch]$Stop,
    [switch]$Restart,
    [switch]$Logs,
    [switch]$Status,
    [switch]$Clean,
    [switch]$Test,
    [switch]$Migrate,
    [string]$Service = "all"
)

$ComposeFile = "docker/docker-compose.optimized.yml"
$ProjectName = "fileserver"

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

function Build-Services {
    Write-Header "Building Complete FileServer System"
    
    Write-Info "Building Docker images with PostgreSQL integration..."
    docker-compose -f $ComposeFile -p $ProjectName build --no-cache
    
    if ($LASTEXITCODE -eq 0) {
        Write-Success "Build completed successfully!"
    } else {
        Write-Error "Build failed!"
        exit 1
    }
}

function Start-Services {
    Write-Header "Starting Complete FileServer System"
    
    Write-Info "Starting PostgreSQL first..."
    docker-compose -f $ComposeFile -p $ProjectName up -d postgres
    
    Write-Info "Waiting for PostgreSQL to be ready..."
    $retries = 0
    $maxRetries = 30
    
    do {
        Start-Sleep 2
        $pgReady = docker exec fileserver-postgres pg_isready -U fileserver -d fileserver 2>$null
        $retries++
        
        if ($LASTEXITCODE -eq 0) {
            Write-Success "PostgreSQL is ready!"
            break
        }
        
        if ($retries -ge $maxRetries) {
            Write-Error "PostgreSQL failed to start within timeout"
            return
        }
        
        Write-Host "." -NoNewline -ForegroundColor Yellow
    } while ($true)
    
    Write-Info "Starting all services..."
    docker-compose -f $ComposeFile -p $ProjectName up -d
    
    if ($LASTEXITCODE -eq 0) {
        Write-Success "All services started successfully!"
        
        Write-Info "Waiting for services to be ready..."
        Start-Sleep 15
        
        Show-Status
        Show-URLs
        Test-Integration
    } else {
        Write-Error "Failed to start services!"
        exit 1
    }
}

function Stop-Services {
    Write-Header "Stopping FileServer System"
    
    docker-compose -f $ComposeFile -p $ProjectName down
    
    if ($LASTEXITCODE -eq 0) {
        Write-Success "Services stopped successfully!"
    } else {
        Write-Error "Failed to stop services!"
    }
}

function Restart-Services {
    Write-Header "Restarting FileServer System"
    
    Stop-Services
    Start-Sleep 3
    Start-Services
}

function Show-Logs {
    Write-Header "FileServer System Logs"
    
    if ($Service -eq "all") {
        docker-compose -f $ComposeFile -p $ProjectName logs -f --tail=100
    } else {
        docker-compose -f $ComposeFile -p $ProjectName logs -f --tail=100 $Service
    }
}

function Show-Status {
    Write-Header "System Status"
    
    docker-compose -f $ComposeFile -p $ProjectName ps
    
    Write-Host "`nService Health:" -ForegroundColor Yellow
    
    # Check PostgreSQL
    $pgHealth = docker exec fileserver-postgres pg_isready -U fileserver -d fileserver 2>$null
    if ($LASTEXITCODE -eq 0) {
        Write-Success "PostgreSQL: Healthy"
        
        # Check database tables
        $tableCount = docker exec fileserver-postgres psql -U fileserver -d fileserver -t -c "SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = 'public';" 2>$null
        if ($LASTEXITCODE -eq 0) {
            Write-Info "Database Tables: $($tableCount.Trim()) tables found"
        }
        
        # Check users
        $userCount = docker exec fileserver-postgres psql -U fileserver -d fileserver -t -c "SELECT COUNT(*) FROM users;" 2>$null
        if ($LASTEXITCODE -eq 0) {
            Write-Info "Database Users: $($userCount.Trim()) users configured"
        }
    } else {
        Write-Error "PostgreSQL: Unhealthy"
    }
    
    # Check FileServer
    try {
        $response = Invoke-WebRequest -Uri "http://localhost:8080/health" -UseBasicParsing -TimeoutSec 5
        if ($response.StatusCode -eq 200) {
            Write-Success "FileServer: Healthy"
            $healthData = $response.Content | ConvertFrom-Json
            Write-Info "Server Status: $($healthData.status)"
        } else {
            Write-Error "FileServer: Unhealthy (Status: $($response.StatusCode))"
        }
    } catch {
        Write-Error "FileServer: Unhealthy (Not responding)"
    }
    
    # Check Redis
    $redisHealth = docker exec fileserver-redis redis-cli ping 2>$null
    if ($LASTEXITCODE -eq 0) {
        Write-Success "Redis: Healthy"
    } else {
        Write-Error "Redis: Unhealthy"
    }
}

function Show-URLs {
    Write-Host "`nService URLs:" -ForegroundColor Yellow
    Write-Host "  FileServer API:     http://localhost:8080" -ForegroundColor Cyan
    Write-Host "  Health Check:       http://localhost:8080/health" -ForegroundColor Cyan
    Write-Host "  API Documentation:  http://localhost:8080/docs" -ForegroundColor Cyan
    Write-Host "  PostgreSQL:         localhost:5432" -ForegroundColor Cyan
    Write-Host "  Redis:              localhost:6379" -ForegroundColor Cyan
    Write-Host "  Grafana:            http://localhost:3000" -ForegroundColor Cyan
    Write-Host "  Prometheus:         http://localhost:9090" -ForegroundColor Cyan
    
    Write-Host "`nDefault Credentials:" -ForegroundColor Yellow
    Write-Host "  Admin: admin / admin123" -ForegroundColor Gray
    Write-Host "  User:  user / user123" -ForegroundColor Gray
    Write-Host "  DB:    fileserver / fileserver123" -ForegroundColor Gray
    Write-Host "  Grafana: admin / admin123" -ForegroundColor Gray
}

function Clean-All {
    Write-Header "Cleaning Complete System"
    
    Write-Info "Stopping and removing containers..."
    docker-compose -f $ComposeFile -p $ProjectName down -v --remove-orphans
    
    Write-Info "Removing images..."
    docker image rm fileserver:latest 2>$null
    
    Write-Info "Pruning unused resources..."
    docker system prune -f
    
    Write-Success "System cleaned!"
}

function Test-Integration {
    Write-Header "Testing System Integration"
    
    # Test health endpoint
    Write-Info "Testing health endpoint..."
    try {
        $health = Invoke-WebRequest -Uri "http://localhost:8080/health" -UseBasicParsing
        if ($health.StatusCode -eq 200) {
            Write-Success "Health check: OK"
            $healthData = $health.Content | ConvertFrom-Json
            Write-Host "Response: $($healthData.status) - $($healthData.message)" -ForegroundColor Gray
        }
    } catch {
        Write-Error "Health check failed: $($_.Exception.Message)"
        return
    }
    
    # Test database connection
    Write-Info "Testing database connection..."
    try {
        $dbTest = docker exec fileserver-postgres psql -U fileserver -d fileserver -c "SELECT COUNT(*) FROM users;" 2>$null
        if ($LASTEXITCODE -eq 0) {
            Write-Success "Database connection: OK"
            Write-Host "Users in database: $($dbTest.Split("`n")[2].Trim())" -ForegroundColor Gray
        }
    } catch {
        Write-Error "Database test failed"
    }
    
    # Test authentication with database
    Write-Info "Testing authentication with PostgreSQL..."
    try {
        $body = @{username="admin"; password="admin123"} | ConvertTo-Json
        $auth = Invoke-WebRequest -Uri "http://localhost:8080/api/v1/auth/login" -Method POST -Body $body -ContentType "application/json" -UseBasicParsing
        
        if ($auth.StatusCode -eq 200) {
            Write-Success "Authentication: OK"
            $authData = $auth.Content | ConvertFrom-Json
            if ($authData.token) {
                Write-Host "JWT Token received: $($authData.token.Substring(0, 20))..." -ForegroundColor Gray
            }
        }
    } catch {
        Write-Error "Authentication failed: $($_.Exception.Message)"
    }
    
    # Test file operations
    Write-Info "Testing file operations..."
    try {
        # First authenticate to get token
        $body = @{username="admin"; password="admin123"} | ConvertTo-Json
        $auth = Invoke-WebRequest -Uri "http://localhost:8080/api/v1/auth/login" -Method POST -Body $body -ContentType "application/json" -UseBasicParsing
        
        if ($auth.StatusCode -eq 200) {
            $authData = $auth.Content | ConvertFrom-Json
            $token = $authData.token
            
            # Test file listing
            $headers = @{Authorization = "Bearer $token"}
            $files = Invoke-WebRequest -Uri "http://localhost:8080/api/v1/files" -Headers $headers -UseBasicParsing
            
            if ($files.StatusCode -eq 200) {
                Write-Success "File listing: OK"
                $fileData = $files.Content | ConvertFrom-Json
                Write-Host "Files found: $($fileData.files.Count)" -ForegroundColor Gray
            }
        }
    } catch {
        Write-Error "File operations test failed: $($_.Exception.Message)"
    }
}

function Run-Migrations {
    Write-Header "Running Database Migrations"
    
    Write-Info "Checking if PostgreSQL is running..."
    $pgHealth = docker exec fileserver-postgres pg_isready -U fileserver -d fileserver 2>$null
    if ($LASTEXITCODE -ne 0) {
        Write-Error "PostgreSQL is not running. Please start it first with -Start"
        return
    }
    
    Write-Info "Running schema migration..."
    Get-Content sql/migrations/001_initial_schema.sql | docker exec -i fileserver-postgres psql -U fileserver -d fileserver
    
    Write-Info "Running seed data..."
    Get-Content sql/seeds/001_default_users.sql | docker exec -i fileserver-postgres psql -U fileserver -d fileserver
    
    Write-Success "Migrations completed!"
}

# Main execution
if (-not (Test-Path $ComposeFile)) {
    Write-Error "Docker Compose file not found: $ComposeFile"
    Write-Info "Please run this script from the project root directory"
    exit 1
}

if ($Build) {
    Build-Services
}

if ($Start) {
    Start-Services
}

if ($Stop) {
    Stop-Services
}

if ($Restart) {
    Restart-Services
}

if ($Logs) {
    Show-Logs
}

if ($Status) {
    Show-Status
}

if ($Clean) {
    Clean-All
}

if ($Test) {
    Test-Integration
}

if ($Migrate) {
    Run-Migrations
}

# Default action if no parameters
if (-not ($Build -or $Start -or $Stop -or $Restart -or $Logs -or $Status -or $Clean -or $Test -or $Migrate)) {
    Write-Header "Complete FileServer System Management"
    
    Write-Host "Usage:" -ForegroundColor Yellow
    Write-Host "  .\run_complete_system.ps1 -Build          # Build all services" -ForegroundColor Cyan
    Write-Host "  .\run_complete_system.ps1 -Start          # Start all services" -ForegroundColor Cyan
    Write-Host "  .\run_complete_system.ps1 -Stop           # Stop all services" -ForegroundColor Cyan
    Write-Host "  .\run_complete_system.ps1 -Restart        # Restart all services" -ForegroundColor Cyan
    Write-Host "  .\run_complete_system.ps1 -Status         # Show system status" -ForegroundColor Cyan
    Write-Host "  .\run_complete_system.ps1 -Logs           # Show logs" -ForegroundColor Cyan
    Write-Host "  .\run_complete_system.ps1 -Test           # Test system integration" -ForegroundColor Cyan
    Write-Host "  .\run_complete_system.ps1 -Migrate        # Run database migrations" -ForegroundColor Cyan
    Write-Host "  .\run_complete_system.ps1 -Clean          # Clean system" -ForegroundColor Cyan
    
    Write-Host "`nCombined operations:" -ForegroundColor Yellow
    Write-Host "  .\run_complete_system.ps1 -Build -Start   # Build and start" -ForegroundColor Cyan
    Write-Host "  .\run_complete_system.ps1 -Start -Test    # Start and test" -ForegroundColor Cyan
    
    Write-Host "`nService-specific logs:" -ForegroundColor Yellow
    Write-Host "  .\run_complete_system.ps1 -Logs -Service postgres" -ForegroundColor Cyan
    Write-Host "  .\run_complete_system.ps1 -Logs -Service fileserver" -ForegroundColor Cyan
    
    Write-Host "`nSystem Components:" -ForegroundColor Yellow
    Write-Host "  - C++ FileServer with REST API" -ForegroundColor White
    Write-Host "  - PostgreSQL Database with Connection Pooling" -ForegroundColor White
    Write-Host "  - JWT Authentication & Authorization" -ForegroundColor White
    Write-Host "  - Redis Caching Layer" -ForegroundColor White
    Write-Host "  - Prometheus Monitoring" -ForegroundColor White
    Write-Host "  - Grafana Dashboards" -ForegroundColor White
    Write-Host "  - Nginx Reverse Proxy" -ForegroundColor White
    Write-Host "  - Swagger API Documentation" -ForegroundColor White
}

Write-Host ""
