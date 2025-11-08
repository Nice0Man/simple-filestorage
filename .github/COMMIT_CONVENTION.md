# Commit Message Convention

This project follows the [Conventional Commits](https://www.conventionalcommits.org/) specification.

## Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Components

#### Type (Required)

The type must be one of the following:

- **feat**: A new feature
- **fix**: A bug fix
- **docs**: Documentation only changes
- **style**: Changes that don't affect code meaning (formatting, missing semicolons, etc.)
- **refactor**: Code change that neither fixes a bug nor adds a feature
- **perf**: Performance improvements
- **test**: Adding or updating tests
- **build**: Changes to build system or dependencies
- **ci**: Changes to CI/CD configuration
- **chore**: Other changes that don't modify src or test files

#### Scope (Optional)

The scope should be the name of the component affected:

- **core**: Core functionality
- **api**: API endpoints
- **auth**: Authentication/Authorization
- **database**: Database operations
- **config**: Configuration management
- **file-manager**: File operations
- **tests**: Test suite

#### Subject (Required)

The subject contains a succinct description of the change:

- Use imperative, present tense: "change" not "changed" nor "changes"
- Don't capitalize the first letter
- No period (.) at the end
- Maximum 50 characters

#### Body (Optional)

- Provide additional context about the change
- Wrap at 72 characters
- Explain **what** and **why**, not **how**
- Use bullet points for multiple items

#### Footer (Optional)

- Reference issues: `Closes #123`, `Fixes #456`
- Breaking changes: `BREAKING CHANGE: description`

## Examples

### Simple Commit

```
feat(auth): add JWT token validation
```

### Commit with Scope and Body

```
fix(file-manager): resolve memory leak in upload handler

The upload handler was not properly releasing file buffers after
processing large files. This commit adds proper resource cleanup
using RAII pattern.

Fixes #42
```

### Breaking Change

```
feat(api)!: change authentication endpoint response format

BREAKING CHANGE: The /api/auth/login endpoint now returns a JSON
object with separate access_token and refresh_token fields instead
of a single token string.

Before:
{
  "token": "..."
}

After:
{
  "access_token": "...",
  "refresh_token": "...",
  "expires_in": 3600
}
```

### Refactoring

```
refactor(database): extract connection pool logic

Moved connection pool implementation to separate class for better
testability and reusability.
```

### Documentation

```
docs(readme): update installation instructions

- Add prerequisites section
- Update build commands for CMake 3.20+
- Add troubleshooting section
```

### Multiple Changes

```
chore: update dependencies and improve build

- Update cpp-httplib to v0.14.3
- Update nlohmann/json to v3.11.3
- Improve CMake configuration for better caching
- Add support for Ninja generator

This improves build times by ~30% on most systems.
```

## Pre-commit Validation

The project uses pre-commit hooks to validate commit messages. Invalid formats will be rejected.

### Running Validation Manually

```bash
# Validate last commit message
git log -1 --pretty=%B | pre-commit run --hook-stage commit-msg
```

## Common Mistakes

### ❌ Bad Examples

```
# No type
Added new feature

# Wrong tense
fixed bug in authentication

# Too vague
update files

# Capitalized subject
feat: Add new endpoint

# Period at end
fix: resolve memory leak.

# Too long subject (>50 chars)
feat(api): implement comprehensive user authentication with JWT tokens
```

### ✅ Good Examples

```
# Clear and concise
feat(api): add user authentication endpoint

# Descriptive fix
fix(auth): prevent null pointer dereference in token validation

# Well-documented breaking change
feat(config)!: change configuration file format to YAML

# Informative refactor
refactor(core): simplify file upload logic
```

## Tips

1. **Start with the type**: Think about what category your change falls into
2. **Be specific with scope**: Help others understand what part of the codebase changed
3. **Use imperative mood**: Write as if giving a command ("add", not "added")
4. **Focus on why**: The diff shows what changed, your message should explain why
5. **Keep it atomic**: One commit should represent one logical change

## Tools

### Commitizen

For interactive commit message creation:

```bash
npm install -g commitizen cz-conventional-changelog

# Use it
git cz
```

### Commitlint

For commit message validation:

```bash
npm install -g @commitlint/cli @commitlint/config-conventional

# Validate
echo "feat: my message" | commitlint
```

## References

- [Conventional Commits Specification](https://www.conventionalcommits.org/)
- [Angular Commit Guidelines](https://github.com/angular/angular/blob/master/CONTRIBUTING.md#commit)
- [Semantic Release](https://semantic-release.gitbook.io/)

