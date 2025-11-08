# Optimized Docker Build Script for FileServer
param(
    [switch]$NoBuildCache,
    [switch]$Verbose,
    [string]$Platform = "linux/amd64",
    [string]$Tag = "fileserver:latest"
)

Write-Host "🚀 Starting optimized FileServer build..." -ForegroundColor Green

# Enable BuildKit
$env:DOCKER_BUILDKIT = "1"

# Build arguments
$buildDate = Get-Date -Format 'yyyy-MM-ddTHH:mm:ssZ'
$vcsRef = "unknown"
try {
    $vcsRef = git rev-parse --short HEAD 2>$null
} catch {
    $vcsRef = "unknown"
}

$buildArgs = @(
    "--build-arg", "BUILD_DATE=$buildDate",
    "--build-arg", "VCS_REF=$vcsRef",
    "--build-arg", "VERSION=1.0.0"
)

Write-Host "📦 Building with optimizations:" -ForegroundColor Cyan
Write-Host "  - Multi-stage caching enabled" -ForegroundColor Gray
Write-Host "  - BuildKit parallel processing" -ForegroundColor Gray
Write-Host "  - Platform: $Platform" -ForegroundColor Gray

try {
    $startTime = Get-Date
    
    if ($Verbose) {
        docker build --platform $Platform --target production --progress plain -t $Tag $buildArgs ..
    } else {
        docker build --platform $Platform --target production -t $Tag $buildArgs ..
    }
    
    $endTime = Get-Date
    
    if ($LASTEXITCODE -eq 0) {
        $buildTime = ($endTime - $startTime).TotalSeconds
        Write-Host "✅ Build completed successfully in $([math]::Round($buildTime, 2)) seconds!" -ForegroundColor Green
        
        Write-Host "`n📊 Image Information:" -ForegroundColor Cyan
        docker images $Tag --format "table {{.Repository}}\t{{.Tag}}\t{{.Size}}\t{{.CreatedAt}}"
    } else {
        Write-Host "❌ Build failed with exit code $LASTEXITCODE" -ForegroundColor Red
        exit $LASTEXITCODE
    }
    
} catch {
    Write-Host "❌ Build error: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}

Write-Host "`n🎉 Optimized build process completed!" -ForegroundColor Green