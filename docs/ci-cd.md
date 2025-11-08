# CI/CD Documentation

## Overview

This project uses GitHub Actions for continuous integration and deployment. The CI/CD pipeline includes:

- **Build and Test**: Automated builds across multiple platforms and compilers
- **Code Quality**: Static analysis and formatting checks
- **Pull Request Validation**: Automated PR checks
- **Release Management**: Automated release creation and artifact building

## Workflows

### 1. CI Workflow (`.github/workflows/ci.yml`)

**Triggers:**
- Push to `main` or `develop` branches
- Pull requests to `main` or `develop`

**Jobs:**

#### Build and Test Matrix
Tests the project across multiple configurations:

| OS | Compilers | Build Types |
|---|---|---|
| Ubuntu 20.04 | GCC 11, Clang 14 | Debug, Release |
| Ubuntu 22.04 | GCC 11, GCC 12, Clang 14, Clang 15 | Debug, Release |

**Steps:**
1. Checkout code with submodules
2. Install dependencies (OpenSSL, PostgreSQL, CMake)
3. Install specific compiler version
4. Configure CMake
5. Build project
6. Run tests with CTest
7. Upload test results on failure

#### Code Quality Checks
Runs static analysis and formatting validation:

- **clang-format**: Validates code formatting
- **cppcheck**: Performs static analysis

**Steps:**
1. Install tools (clang-format, cppcheck)
2. Check formatting with `clang-format --dry-run --Werror`
3. Run cppcheck with multiple checks enabled

#### Docker Build Test
Validates the Dockerfile:

**Steps:**
1. Set up Docker Buildx
2. Build Docker image
3. Test image execution
4. Use GitHub Actions cache for faster builds

### 2. Pull Request Check (`.github/workflows/pr-check.yml`)

**Triggers:**
- Pull request opened, synchronized, or reopened

**Jobs:**

#### Title Check
Validates PR title follows conventional commits format:
- Allowed types: `feat`, `fix`, `docs`, `style`, `refactor`, `perf`, `test`, `build`, `ci`, `chore`
- Example: `feat(auth): add JWT validation`

#### Size Check
Monitors PR size and warns if:
- More than 50 files changed
- More than 1000 lines added

#### Conflict Check
Ensures PR has no merge conflicts with base branch

#### Auto-label
Automatically labels PRs based on changed files:
- `C: Core` - Core functionality changes
- `C: API` - API changes
- `C: Security` - Security-related changes
- `C: Database` - Database changes
- `T: Tests` - Test changes
- `T: Documentation` - Documentation updates
- `T: Build` - Build system changes
- `T: CI/CD` - CI/CD changes

### 3. Release Workflow (`.github/workflows/release.yml`)

**Triggers:**
- Push of version tags (format: `v*.*.*`)
- Example: `v1.0.0`, `v2.1.3`

**Jobs:**

#### Create Release
Creates GitHub release with:
- Version number extracted from tag
- Changelog generated from commits since last tag
- Installation instructions
- Automatic prerelease detection (for `alpha`, `beta`, `rc` versions)

#### Build Artifacts
Builds release binaries for:
- Ubuntu 20.04 (linux-x86_64)
- Ubuntu 22.04 (linux-x86_64-ubuntu22)

Creates `.tar.gz` packages with:
- Compiled binary
- LICENSE file
- README.md

#### Build Docker Image
- Builds Docker image
- Pushes to GitHub Container Registry
- Tags with version number and `latest`
- Uses GitHub Actions cache

## Setting Up CI/CD for Your Fork

### 1. Enable GitHub Actions

1. Go to repository Settings → Actions → General
2. Select "Allow all actions and reusable workflows"
3. Save

### 2. Configure Secrets (Optional)

For private container registry or additional features:

1. Go to Settings → Secrets and variables → Actions
2. Add repository secrets as needed

### 3. Enable Container Registry (for releases)

1. Go to Settings → Actions → General → Workflow permissions
2. Select "Read and write permissions"
3. Enable "Allow GitHub Actions to create and approve pull requests"

### 4. Configure Branch Protection (Recommended)

Protect `main` branch:

1. Go to Settings → Branches → Add branch protection rule
2. Branch name pattern: `main`
3. Enable:
   - ✅ Require a pull request before merging
   - ✅ Require status checks to pass before merging
   - ✅ Require branches to be up to date before merging
   - ✅ Status checks: `Build and Test`, `Code Quality Checks`
4. Save

## Creating a Release

### 1. Prepare Release

```bash
# Update version in CMakeLists.txt
# Update CHANGELOG.md (if exists)

git add CMakeLists.txt
git commit -m "chore: prepare release v1.0.0"
git push origin main
```

### 2. Create and Push Tag

```bash
# Create annotated tag
git tag -a v1.0.0 -m "Release version 1.0.0"

# Push tag to trigger release workflow
git push origin v1.0.0
```

### 3. Monitor Release

1. Go to Actions tab
2. Watch "Release" workflow
3. Once complete, check Releases page
4. Download artifacts or Docker image

### Release Versioning

Follow Semantic Versioning (SemVer):

- **MAJOR** version: incompatible API changes
- **MINOR** version: new functionality (backward-compatible)
- **PATCH** version: bug fixes (backward-compatible)

Examples:
- `v1.0.0` - First stable release
- `v1.1.0` - New features added
- `v1.1.1` - Bug fixes
- `v2.0.0` - Breaking changes
- `v1.0.0-alpha` - Alpha prerelease
- `v1.0.0-beta.1` - Beta prerelease
- `v1.0.0-rc.1` - Release candidate

## Using Docker Images

### Pull Latest Release

```bash
docker pull ghcr.io/<owner>/simple-filestorage:latest
```

### Pull Specific Version

```bash
docker pull ghcr.io/<owner>/simple-filestorage:1.0.0
```

### Run Container

```bash
docker run -p 8080:8080 \
  -v $(pwd)/data:/data \
  ghcr.io/<owner>/simple-filestorage:latest
```

## Monitoring CI/CD

### View Workflow Runs

1. Go to Actions tab
2. Select workflow from sidebar
3. View run details, logs, and artifacts

### Understanding Build Status

- ✅ Green checkmark: All checks passed
- ❌ Red X: Some checks failed
- 🟡 Yellow dot: Checks in progress
- ⚪ Gray circle: Checks pending

### Debugging Failed Builds

1. Click on failed workflow run
2. Expand failed job
3. Review error logs
4. Common issues:
   - **Compilation errors**: Check build logs
   - **Test failures**: Check test output
   - **Formatting issues**: Run `pre-commit run --all-files` locally
   - **cppcheck warnings**: Review static analysis output

### Re-running Failed Jobs

1. Open failed workflow run
2. Click "Re-run jobs" → "Re-run failed jobs"
3. Or "Re-run all jobs" to run everything again

## Best Practices

### For Contributors

1. **Run pre-commit hooks locally** before pushing
2. **Ensure tests pass locally** before creating PR
3. **Follow conventional commit format** for PR titles
4. **Keep PRs focused and small** (< 50 files, < 1000 lines)
5. **Update documentation** when changing features

### For Maintainers

1. **Review CI results** before merging PRs
2. **Test release artifacts** before announcing releases
3. **Keep dependencies updated** regularly
4. **Monitor workflow execution times** and optimize if needed
5. **Update CI configuration** when adding new compilers or OSes

## Troubleshooting

### Workflow Not Triggering

**Problem**: Workflow doesn't run on push/PR

**Solutions**:
1. Check Actions are enabled in repository settings
2. Verify workflow file syntax: `yamllint .github/workflows/*.yml`
3. Ensure branch matches trigger configuration
4. Check workflow file is in `main` branch

### Build Fails on Specific Platform

**Problem**: Build works locally but fails in CI

**Solutions**:
1. Check compiler version matches
2. Verify all dependencies are installed
3. Review platform-specific code paths
4. Test with Docker using same OS image

### Docker Build Timeout

**Problem**: Docker build exceeds time limit

**Solutions**:
1. Optimize Dockerfile (use multi-stage builds)
2. Enable Docker layer caching
3. Reduce image size
4. Split into multiple jobs if needed

### High CI Cost (For Private Repos)

**Problem**: Using too many Action minutes

**Solutions**:
1. Reduce build matrix (fewer OS/compiler combinations)
2. Use caching effectively
3. Skip CI for documentation-only changes
4. Use self-hosted runners for frequent builds

## Maintenance

### Regular Updates

**Weekly:**
- Review failed workflow runs
- Check for workflow improvements

**Monthly:**
- Update pre-commit hooks: `pre-commit autoupdate`
- Review GitHub Actions versions
- Update compiler versions in matrix

**Quarterly:**
- Review and update dependencies
- Evaluate new CI/CD features
- Optimize workflow performance

### Metrics to Monitor

- ⏱️ Average build time
- ✅ Success rate
- 📊 Test coverage
- 🐛 Bug detection rate
- 🔄 Time to merge PRs

## Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Conventional Commits](https://www.conventionalcommits.org/)
- [Semantic Versioning](https://semver.org/)
- [Docker Best Practices](https://docs.docker.com/develop/dev-best-practices/)

