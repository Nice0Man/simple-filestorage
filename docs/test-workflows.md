# Testing GitHub Workflows

## Quick Test

### Local Testing (Recommended)

**Linux/macOS:**
```bash
bash scripts/test-ci-locally.sh
```

**Windows:**
```powershell
powershell scripts/test-ci-locally.ps1
```

### GitHub Actions

1. Commit and push your changes:
```bash
git add .
git commit -m "feat: add CI/CD configuration"
git push origin main
```

2. Go to GitHub → **Actions** tab
3. Watch the workflows run

## Install Required Tools

### Ubuntu/Debian

```bash
# Install all tools at once
sudo apt-get update
sudo apt-get install -y \
    cmake \
    ninja-build \
    g++-12 \
    clang-14 \
    clang-format-14 \
    cppcheck \
    libssl-dev \
    postgresql-server-dev-all \
    pkg-config \
    python3 \
    python3-pip \
    python3-yaml

# Install pre-commit
pip3 install --user pre-commit

# Link clang-format
sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-14 100
```

### Windows (with Chocolatey)

```powershell
# Install Chocolatey first: https://chocolatey.org/install

# Install tools
choco install -y cmake ninja llvm python3 git

# Install pre-commit
pip install pre-commit
```

### Windows (with MSYS2/MinGW)

```bash
# Install MSYS2 from https://www.msys2.org/

# In MSYS2 terminal:
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-clang
pacman -S mingw-w64-x86_64-clang-tools-extra
```

## Workflow Testing Methods

### Method 1: Local Script (Fastest)

```bash
# Run full test suite
bash scripts/test-ci-locally.sh

# What it tests:
# ✓ YAML syntax
# ✓ Code formatting
# ✓ Static analysis
# ✓ Debug build
# ✓ Unit tests
# ✓ Release build
# ✓ Docker build (optional)
```

### Method 2: Act - Run GitHub Actions Locally

[Act](https://github.com/nektos/act) runs GitHub Actions on your local machine using Docker.

**Install:**
```bash
# Linux/macOS with Homebrew
brew install act

# Linux with curl
curl https://raw.githubusercontent.com/nektos/act/master/install.sh | sudo bash

# Windows with Chocolatey
choco install act-cli
```

**Usage:**
```bash
# List all workflows
act -l

# Run push workflow
act push

# Run PR workflow
act pull_request

# Run specific job
act -j build-and-test

# Dry run (don't actually execute)
act -n

# With secrets
act --secret-file .secrets
```

**Note:** First run will download Docker images (~500MB-2GB)

### Method 3: ActionLint - Validate Workflow Syntax

[ActionLint](https://github.com/rhysd/actionlint) validates GitHub Actions workflow files.

**Install:**
```bash
# Linux/macOS
bash <(curl https://raw.githubusercontent.com/rhysd/actionlint/main/scripts/download-actionlint.bash)

# Or with Homebrew
brew install actionlint

# Windows with Chocolatey
choco install actionlint
```

**Usage:**
```bash
# Check all workflows
actionlint

# Check specific file
actionlint .github/workflows/ci.yml

# With detailed output
actionlint -verbose

# Output in JSON
actionlint -format '{{json .}}'
```

### Method 4: GitHub Push to Test Branch

```bash
# Create test branch
git checkout -b test-ci

# Make changes
git add .
git commit -m "test: verify CI configuration"

# Push and watch Actions tab
git push origin test-ci
```

## Understanding Workflow Triggers

### CI Workflow (`.github/workflows/ci.yml`)

**Triggers on:**
- Push to `main` or `develop` branches
- Pull requests to `main` or `develop`

**Test locally:**
```bash
# Simulate push event
act push

# Simulate PR event
act pull_request
```

### PR Check Workflow (`.github/workflows/pr-check.yml`)

**Triggers on:**
- Pull request opened
- Pull request synchronized (new commits)
- Pull request reopened

**Test:**
```bash
# Create PR from feature branch
git checkout -b feature/test
git commit --allow-empty -m "feat: test feature"
git push origin feature/test
# Then create PR on GitHub
```

### Release Workflow (`.github/workflows/release.yml`)

**Triggers on:**
- Tag push matching `v*.*.*` pattern

**Test:**
```bash
# Create test tag (don't push to production!)
git tag v0.0.1-test
git push origin v0.0.1-test

# Watch Actions tab
# Delete test tag after:
git tag -d v0.0.1-test
git push origin :refs/tags/v0.0.1-test
```

## Debugging Failed Workflows

### Check Workflow Logs

1. Go to **Actions** tab on GitHub
2. Click on failed workflow run
3. Click on failed job
4. Expand failed step
5. Read error messages

### Common Issues

#### 1. Build Failures

**Symptom:** Compilation errors in CI but works locally

**Fix:**
```bash
# Check with same compiler as CI
CC=gcc-11 CXX=g++-11 cmake -B build
cmake --build build

# Or with Clang
CC=clang-14 CXX=clang++-14 cmake -B build
cmake --build build
```

#### 2. Test Failures

**Symptom:** Tests fail in CI

**Fix:**
```bash
# Run tests with verbose output
cd build
ctest --verbose --output-on-failure

# Run specific test
./bin/fileserver_tests --gtest_filter=TestSuite.TestName
```

#### 3. Formatting Issues

**Symptom:** clang-format check fails

**Fix:**
```bash
# Format all files
find src include tests -name '*.cpp' -o -name '*.h' -o -name '*.hpp' | \
    xargs clang-format -i

# Check result
clang-format --dry-run --Werror src/**/*.cpp

# Commit formatting changes
git add .
git commit -m "style: apply clang-format"
```

#### 4. cppcheck Warnings

**Symptom:** Static analysis finds issues

**Fix:**
```bash
# Run locally to see issues
cppcheck --enable=all \
    --suppress=missingIncludeSystem \
    --suppress=unusedFunction \
    -I include \
    src/

# Fix issues or add inline suppressions
// cppcheck-suppress warningName
problematic_code();
```

#### 5. Docker Build Fails

**Symptom:** Docker build works locally but fails in CI

**Fix:**
```bash
# Build without cache
docker build --no-cache -t fileserver:test .

# Check multi-platform compatibility
docker buildx build --platform linux/amd64,linux/arm64 -t fileserver:test .
```

## CI Performance Tips

### Speed Up Builds

1. **Use caching:**
   - CMake cache (included in workflows)
   - Docker layer cache (included)
   - ccache for compilation

2. **Parallel builds:**
   ```cmake
   cmake --build build -j$(nproc)
   ```

3. **Skip unnecessary jobs:**
   ```bash
   # Skip CI for docs-only changes
   git commit -m "docs: update readme [skip ci]"
   ```

### Reduce CI Costs

For private repositories (GitHub Actions minutes are limited):

1. **Reduce matrix size:**
   - Test fewer compiler versions
   - Test fewer OS versions

2. **Use self-hosted runners:**
   - Set up your own CI machine
   - Free for private repos

3. **Conditional workflows:**
   ```yaml
   if: contains(github.event.head_commit.message, '[ci skip]') == false
   ```

## Monitoring CI Health

### Key Metrics

- ⏱️ **Build Time:** Should be < 10 minutes
- ✅ **Success Rate:** Should be > 95%
- 🐛 **Flaky Tests:** Should be 0
- 📊 **Code Coverage:** Track over time

### GitHub Actions Insights

1. Go to repository **Insights** tab
2. Click **Actions** in sidebar
3. View:
   - Workflow runs over time
   - Success/failure rates
   - Average duration
   - Workflow usage

## Troubleshooting Guide

### Workflow Doesn't Trigger

**Check:**
1. ✅ Workflow file is in `.github/workflows/`
2. ✅ YAML syntax is valid
3. ✅ Branch matches trigger pattern
4. ✅ Actions are enabled in repo settings
5. ✅ Workflow file is in default branch

**Test:**
```bash
# Validate YAML
python3 -c "import yaml; yaml.safe_load(open('.github/workflows/ci.yml'))"

# Check with actionlint
actionlint .github/workflows/ci.yml
```

### Permission Errors

**Error:** "Resource not accessible by integration"

**Fix:** Go to Settings → Actions → General → Workflow permissions → Select "Read and write permissions"

### Rate Limiting

**Error:** "API rate limit exceeded"

**Fix:** Wait or use GitHub token with higher limits

### Secrets Not Available

**Error:** "Secret not found"

**Fix:**
1. Go to Settings → Secrets and variables → Actions
2. Add required secrets
3. Use in workflow: `${{ secrets.SECRET_NAME }}`

## Best Practices

### Before Committing

```bash
# Run pre-commit hooks
pre-commit run --all-files

# Run local CI test
bash scripts/test-ci-locally.sh

# Check build
cmake -B build && cmake --build build

# Run tests
cd build && ctest
```

### Pull Request Checklist

- [ ] All CI checks pass
- [ ] No new warnings
- [ ] Tests added for new features
- [ ] Documentation updated
- [ ] Commits follow convention
- [ ] PR title follows convention

### Release Checklist

- [ ] All tests pass on main
- [ ] Version updated in CMakeLists.txt
- [ ] CHANGELOG updated
- [ ] Tag follows semver
- [ ] Release notes prepared

## Additional Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Act - Local GitHub Actions](https://github.com/nektos/act)
- [ActionLint](https://github.com/rhysd/actionlint)
- [CMake Best Practices](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)
- [Pre-commit Framework](https://pre-commit.com/)

## Quick Reference

### Test Commands

```bash
# Local CI test
bash scripts/test-ci-locally.sh

# Pre-commit hooks
pre-commit run --all-files

# Format code
find src include tests -name '*.cpp' -o -name '*.h' -o -name '*.hpp' | xargs clang-format -i

# Static analysis
cppcheck --enable=all -I include src/

# Build
cmake -B build && cmake --build build -j$(nproc)

# Test
cd build && ctest --output-on-failure

# Docker
docker build -t fileserver:test .
```

### GitHub Actions Commands

```bash
# List workflows
act -l

# Run push workflow
act push

# Run with secrets
act --secret-file .secrets

# Dry run
act -n
```

