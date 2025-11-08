# Run FileServer with PostgreSQL
param(
    [switch]$Build,
    [switch]$Start,
    [switch]$Stop,
    [switch]$Restart,
    [switch]$Logs,
    [switch]$Status,
    [switch]$Clean,
    [switch]$Test,
    [string]$Service = "all"
)

$ComposeFile = "docker/docker-compose.optimized.yml"
$ProjectName = "fileserver"

function Write-Header($message) {
    Write-Host "`n🚀 $message" -ForegroundColor Green
    Write-Host ("=" * 50) -ForegroundColor Gray
}

function Write-Info($message) {
    Write-Host "ℹ️  $message" -ForegroundColor Cyan
}

function Write-Success($message) {
    Write-Host "✅ $message" -ForegroundColor Green
}

function Write-Error($message) {
    Write-Host "❌ $message" -ForegroundColor Red
}

function Build-Services {
    Write-Header "Building FileServer with PostgreSQL"
    
    Write-Info "Building Docker images..."
    docker-compose -f $ComposeFile -p $ProjectName build --no-cache
    
    if ($LASTEXITCODE -eq 0) {
        Write-Success "Build completed successfully!"
    } else {
        Write-Error "Build failed!"
        exit 1
    }
}

function Start-Services {
    Write-Header "Starting FileServer with PostgreSQL"
    
    Write-Info "Starting services..."
    docker-compose -f $ComposeFile -p $ProjectName up -d
    
    if ($LASTEXITCODE -eq 0) {
        Write-Success "Services started successfully!"
        
        Write-Info "Waiting for services to be ready..."
        Start-Sleep 10
        
        Show-Status
        Show-URLs
    } else {
        Write-Error "Failed to start services!"
        exit 1
    }
}

function Stop-Services {
    Write-Header "Stopping FileServer Services"
    
    docker-compose -f $ComposeFile -p $ProjectName down
    
    if ($LASTEXITCODE -eq 0) {
        Write-Success "Services stopped successfully!"
    } else {
        Write-Error "Failed to stop services!"
    }
}

function Restart-Services {
    Write-Header "Restarting FileServer Services"
    
    Stop-Services
    Start-Sleep 2
    Start-Services
}

function Show-Logs {
    Write-Header "FileServer Logs"
    
    if ($Service -eq "all") {
        docker-compose -f $ComposeFile -p $ProjectName logs -f
    } else {
        docker-compose -f $ComposeFile -p $ProjectName logs -f $Service
    }
}

function Show-Status {
    Write-Header "Service Status"
    
    docker-compose -f $ComposeFile -p $ProjectName ps
    
    Write-Host "`n📊 Container Health:" -ForegroundColor Yellow
    
    # Check PostgreSQL
    $pgHealth = docker exec fileserver-postgres pg_isready -U fileserver -d fileserver 2>$null
    if ($LASTEXITCODE -eq 0) {
        Write-Success "PostgreSQL: Healthy"
    } else {
        Write-Error "PostgreSQL: Unhealthy"
    }
    
    # Check FileServer
    try {
        $response = Invoke-WebRequest -Uri "http://localhost:8080/health" -UseBasicParsing -TimeoutSec 5
        if ($response.StatusCode -eq 200) {
            Write-Success "FileServer: Healthy"
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
    Write-Host "`n🌐 Service URLs:" -ForegroundColor Yellow
    Write-Host "  📡 FileServer API:    http://localhost:8080" -ForegroundColor Cyan
    Write-Host "  💚 Health Check:     http://localhost:8080/health" -ForegroundColor Cyan
    Write-Host "  📖 API Documentation: http://localhost:8080/docs" -ForegroundColor Cyan
    Write-Host "  🗄️  PostgreSQL:       localhost:5432" -ForegroundColor Cyan
    Write-Host "  🔴 Redis:            localhost:6379" -ForegroundColor Cyan
    Write-Host "  📊 Grafana:          http://localhost:3000" -ForegroundColor Cyan
    Write-Host "  📈 Prometheus:       http://localhost:9090" -ForegroundColor Cyan
    
    Write-Host "`n🔐 Default Credentials:" -ForegroundColor Yellow
    Write-Host "  Admin: admin / admin123" -ForegroundColor Gray
    Write-Host "  User:  user / user123" -ForegroundColor Gray
    Write-Host "  DB:    fileserver / fileserver123" -ForegroundColor Gray
}

function Clean-All {
    Write-Header "Cleaning FileServer Environment"
    
    Write-Info "Stopping and removing containers..."
    docker-compose -f $ComposeFile -p $ProjectName down -v --remove-orphans
    
    Write-Info "Removing images..."
    docker image rm fileserver:latest 2>$null
    
    Write-Info "Pruning unused resources..."
    docker system prune -f
    
    Write-Success "Environment cleaned!"
}

function Test-API {
    Write-Header "Testing FileServer API"
    
    # Test health endpoint
    Write-Info "Testing health endpoint..."
    try {
        $health = Invoke-WebRequest -Uri "http://localhost:8080/health" -UseBasicParsing
        if ($health.StatusCode -eq 200) {
            Write-Success "Health check: OK"
            Write-Host "Response: $($health.Content)" -ForegroundColor Gray
        }
    } catch {
        Write-Error "Health check failed: $($_.Exception.Message)"
        return
    }
    
    # Test authentication
    Write-Info "Testing authentication..."
    try {
        $body = @{username="admin"; password="admin123"} | ConvertTo-Json
        $auth = Invoke-WebRequest -Uri "http://localhost:8080/api/v1/auth/login" -Method POST -Body $body -ContentType "application/json" -UseBasicParsing
        
        if ($auth.StatusCode -eq 200) {
            Write-Success "Authentication: OK"
            $token = ($auth.Content | ConvertFrom-Json).token
            Write-Host "Token received: $($token.Substring(0, 20))..." -ForegroundColor Gray
        }
    } catch {
        Write-Error "Authentication failed: $($_.Exception.Message)"
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
    Test-API
}

# Default action if no parameters
if (-not ($Build -or $Start -or $Stop -or $Restart -or $Logs -or $Status -or $Clean -or $Test)) {
    Write-Header "FileServer with PostgreSQL Management"
    
    Write-Host "Usage:" -ForegroundColor Yellow
    Write-Host "  .\run_with_postgres.ps1 -Build          # Build services" -ForegroundColor Cyan
    Write-Host "  .\run_with_postgres.ps1 -Start          # Start services" -ForegroundColor Cyan
    Write-Host "  .\run_with_postgres.ps1 -Stop           # Stop services" -ForegroundColor Cyan
    Write-Host "  .\run_with_postgres.ps1 -Restart        # Restart services" -ForegroundColor Cyan
    Write-Host "  .\run_with_postgres.ps1 -Status         # Show status" -ForegroundColor Cyan
    Write-Host "  .\run_with_postgres.ps1 -Logs           # Show logs" -ForegroundColor Cyan
    Write-Host "  .\run_with_postgres.ps1 -Test           # Test API" -ForegroundColor Cyan
    Write-Host "  .\run_with_postgres.ps1 -Clean          # Clean environment" -ForegroundColor Cyan
    
    Write-Host "`nCombined operations:" -ForegroundColor Yellow
    Write-Host "  .\run_with_postgres.ps1 -Build -Start   # Build and start" -ForegroundColor Cyan
    Write-Host "  .\run_with_postgres.ps1 -Start -Test    # Start and test" -ForegroundColor Cyan
    
    Write-Host "`nService-specific logs:" -ForegroundColor Yellow
    Write-Host "  .\run_with_postgres.ps1 -Logs -Service postgres" -ForegroundColor Cyan
    Write-Host "  .\run_with_postgres.ps1 -Logs -Service fileserver" -ForegroundColor Cyan
}

Write-Host ""
