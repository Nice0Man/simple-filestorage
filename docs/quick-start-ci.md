# Quick Start: CI/CD Setup

## 🚀 Quick Setup (5 minutes)

### 1. Install Pre-commit Hooks

**Linux/macOS:**
```bash
bash scripts/setup-precommit.sh
```

**Windows:**
```powershell
powershell scripts/setup-precommit.ps1
```

### 2. Make Your First Commit

```bash
# Format all code
pre-commit run --all-files

# Add changes
git add .

# Commit with conventional format
git commit -m "feat: add CI/CD configuration"
```

### 3. Push and Verify

```bash
# Push to trigger CI
git push origin main

# Go to GitHub → Actions tab to see workflows running
```

## 📋 What Was Added

### GitHub Workflows

1. **`.github/workflows/ci.yml`** - Main CI pipeline
   - Builds on Ubuntu 20.04/22.04
   - Tests with GCC 11/12 and Clang 14/15
   - Runs code quality checks
   - Tests Docker build

2. **`.github/workflows/pr-check.yml`** - PR validation
   - Checks PR title format
   - Monitors PR size
   - Detects merge conflicts
   - Auto-labels PRs

3. **`.github/workflows/release.yml`** - Release automation
   - Creates GitHub releases
   - Builds artifacts
   - Publishes Docker images

### Pre-commit Configuration

- **`.pre-commit-config.yaml`** - Hooks configuration
- **`.clang-format`** - C++ code style
- **`.secrets.baseline`** - Secret detection baseline
- **`.github/labeler.yml`** - PR auto-labeling rules

### Documentation

- **`docs/development.md`** - Complete development guide
- **`docs/ci-cd.md`** - CI/CD documentation
- **`.github/COMMIT_CONVENTION.md`** - Commit message guide
- **`.github/PULL_REQUEST_TEMPLATE.md`** - PR template

### Scripts

- **`scripts/setup-precommit.sh`** - Linux/macOS setup
- **`scripts/setup-precommit.ps1`** - Windows setup

## 🔍 Pre-commit Hooks

### What Runs Automatically

✅ **On Every Commit:**
- Trailing whitespace removal
- End-of-file fixer
- YAML/JSON validation
- Large file detection
- clang-format (code formatting)
- cppcheck (static analysis)
- Include guard validation
- Debug print detection
- Secret detection

### Manual Commands

```bash
# Run all hooks
pre-commit run --all-files

# Run specific hook
pre-commit run clang-format

# Update hooks
pre-commit autoupdate

# Skip hooks (not recommended)
git commit --no-verify
```

## 🏗️ CI Pipeline

### Trigger Events

- **Push** to `main` or `develop`
- **Pull Request** to `main` or `develop`
- **Tag** push (for releases)

### Build Matrix

| OS | Compilers | Build Types |
|---|---|---|
| Ubuntu 20.04 | GCC 11, Clang 14 | Debug, Release |
| Ubuntu 22.04 | GCC 11/12, Clang 14/15 | Debug, Release |

### What CI Checks

1. ✅ Compilation on multiple platforms
2. ✅ All unit tests pass
3. ✅ Code formatting (clang-format)
4. ✅ Static analysis (cppcheck)
5. ✅ Docker build

## 📦 Creating a Release

### Step 1: Prepare

```bash
# Update version in CMakeLists.txt if needed
# Commit changes
git add .
git commit -m "chore: prepare release v1.0.0"
git push origin main
```

### Step 2: Tag and Push

```bash
# Create tag
git tag -a v1.0.0 -m "Release version 1.0.0"

# Push tag (triggers release workflow)
git push origin v1.0.0
```

### Step 3: Wait

- GitHub Actions builds artifacts
- Creates GitHub Release
- Publishes Docker image to GHCR

## 🔧 Troubleshooting

### Pre-commit Hook Fails

```bash
# See what failed
pre-commit run --all-files

# Auto-fix issues
pre-commit run --all-files

# Add fixes and commit
git add .
git commit
```

### CI Build Fails

1. Check Actions tab on GitHub
2. Review error logs
3. Test locally with same compiler:
   ```bash
   # Example with GCC 12
   CC=gcc-12 CXX=g++-12 cmake -B build
   cmake --build build
   ```

### Formatting Issues

```bash
# Format all files
find src include tests -name '*.cpp' -o -name '*.h' -o -name '*.hpp' | \
  xargs clang-format -i

# Check result
pre-commit run clang-format --all-files
```

## 📖 Commit Message Format

### Basic Format

```
type(scope): subject

body

footer
```

### Examples

```bash
# Simple feature
git commit -m "feat(auth): add JWT token validation"

# Bug fix
git commit -m "fix(file-manager): resolve memory leak in upload handler"

# Documentation
git commit -m "docs(readme): update installation instructions"

# Breaking change
git commit -m "feat(api)!: change authentication response format

BREAKING CHANGE: Login endpoint now returns access_token
and refresh_token separately."
```

### Valid Types

- `feat` - New feature
- `fix` - Bug fix
- `docs` - Documentation
- `style` - Formatting
- `refactor` - Code restructuring
- `perf` - Performance
- `test` - Tests
- `build` - Build system
- `ci` - CI/CD
- `chore` - Maintenance

## 🎯 Best Practices

### Before Commit

1. ✅ Run tests: `cd build && ctest`
2. ✅ Format code: `pre-commit run --all-files`
3. ✅ Check lints: No new warnings
4. ✅ Write good commit message

### Before PR

1. ✅ Rebase on latest main
2. ✅ Squash related commits
3. ✅ Update documentation
4. ✅ All CI checks pass

### PR Guidelines

- Keep PRs small (< 50 files, < 1000 lines)
- One feature/fix per PR
- Write descriptive PR title (conventional format)
- Fill out PR template completely

## 🔗 Next Steps

1. **Read Full Documentation:**
   - [Development Guide](development.md)
   - [CI/CD Documentation](ci-cd.md)
   - [Commit Convention](.github/COMMIT_CONVENTION.md)

2. **Configure Branch Protection:**
   - Go to GitHub → Settings → Branches
   - Protect `main` branch
   - Require CI checks before merge

3. **Set Up Notifications:**
   - GitHub → Settings → Notifications
   - Enable workflow notifications

## 📞 Need Help?

- 📖 Check documentation in `docs/`
- 🐛 Open an issue on GitHub
- 💬 Ask in project discussions

---

**Happy coding! 🎉**

