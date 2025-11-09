# E2E Tests with Playwright

This directory contains end-to-end tests for the File Storage application using Playwright.

## Test Structure

- `auth.spec.ts` - Authentication flow tests (login, logout, session persistence)
- `files.spec.ts` - File management tests (upload, download, delete)
- `ui.spec.ts` - UI components and theme tests

## Running Tests

### Prerequisites

Install Playwright browsers:
```bash
npx playwright install
```

For Linux, you may need to install system dependencies:
```bash
sudo npx playwright install-deps
```

### Test Commands

```bash
# Run all tests (headless)
npm run test:e2e

# Run tests with UI mode (interactive)
npm run test:e2e:ui

# Run tests in headed mode (see browser)
npm run test:e2e:headed

# Run tests in debug mode
npm run test:e2e:debug

# View test report
npm run test:e2e:report
```

### Run Specific Test File

```bash
npx playwright test e2e/auth.spec.ts
npx playwright test e2e/files.spec.ts
npx playwright test e2e/ui.spec.ts
```

### Run Specific Test

```bash
npx playwright test -g "should login successfully"
```

### Run Tests on Specific Browser

```bash
npx playwright test --project=chromium
npx playwright test --project=firefox
npx playwright test --project=webkit
```

## Test Coverage

### Authentication Tests (`auth.spec.ts`)
- ✅ Display login page
- ✅ Form validation (empty fields, invalid username)
- ✅ Successful login with valid credentials
- ✅ Error handling for invalid credentials
- ✅ Logout functionality
- ✅ Session persistence after reload
- ✅ Protected route access control

### File Management Tests (`files.spec.ts`)
- ✅ Display files page
- ✅ Empty state handling
- ✅ File upload with validation
- ✅ Oversized file validation
- ✅ Invalid filename validation
- ✅ File download
- ✅ File deletion with confirmation dialog
- ✅ Cancel deletion
- ✅ Loading states
- ✅ File metadata display

### UI Tests (`ui.spec.ts`)
- ✅ Page title
- ✅ Responsive design (mobile)
- ✅ Theme toggle
- ✅ Toast notifications
- ✅ Loading skeletons
- ✅ Form accessibility (labels, ARIA)
- ✅ Header display when authenticated
- ✅ Navigation
- ✅ Focus management
- ✅ Keyboard navigation

## Configuration

Test configuration is in `playwright.config.ts`:
- Base URL: `http://localhost:80`
- Browsers: Chromium, Firefox, WebKit, Mobile Chrome, Mobile Safari
- Retries: 2 on CI, 0 locally
- Trace: On first retry
- Screenshots: On failure only
- Videos: Retained on failure

## Test Credentials

Default test credentials (admin user):
- Username: `admin`
- Password: `admin`

## CI/CD Integration

Tests are configured to run in CI with:
- Automatic browser installation
- Parallel execution disabled on CI
- HTML and list reporters
- Screenshots and videos on failure

## Debugging Tests

### VS Code Extension
Install the Playwright Test for VSCode extension for:
- Running tests from editor
- Debugging with breakpoints
- Test explorer integration

### Debug Mode
```bash
npm run test:e2e:debug
```

This opens Playwright Inspector where you can:
- Step through tests
- Inspect DOM
- View network requests
- Check console logs

### UI Mode
```bash
npm run test:e2e:ui
```

Interactive mode with:
- Watch mode
- Time travel debugging
- Trace viewer
- Test picker

## Best Practices

1. **Use data-testid for stable selectors** when needed
2. **Wait for navigation** with `page.waitForURL()`
3. **Use user-facing selectors** (role, label, text)
4. **Handle async operations** with proper timeouts
5. **Clean up test data** when possible
6. **Use beforeEach** for common setup (login)
7. **Test user flows**, not implementation details

## Troubleshooting

### Tests timeout
- Increase timeout in test config
- Check if application is running
- Verify network connectivity

### Browser not found
```bash
npx playwright install chromium
```

### System dependencies missing (Linux)
```bash
sudo npx playwright install-deps
```

### Port already in use
Change port in `playwright.config.ts` or stop conflicting service

## Resources

- [Playwright Documentation](https://playwright.dev/)
- [Best Practices](https://playwright.dev/docs/best-practices)
- [Debugging Guide](https://playwright.dev/docs/debug)

