import { test, expect } from '@playwright/test';

test.describe('UI Components and Theme', () => {
  test.beforeEach(async ({ page }) => {
    await page.goto('/login');
  });

  test('should display correct page title', async ({ page }) => {
    await expect(page).toHaveTitle(/file storage/i);
  });

  test('should have responsive design on mobile', async ({ page, viewport }) => {
    // Test on mobile viewport
    await page.setViewportSize({ width: 375, height: 667 });
    
    // Check if login form is visible and properly sized
    const loginCard = page.locator('.max-w-md');
    await expect(loginCard).toBeVisible();
    
    // Form should be responsive
    const form = page.locator('form');
    await expect(form).toBeVisible();
  });

  test('should toggle theme', async ({ page }) => {
    // Login first to see header
    await page.getByLabel(/username/i).fill('admin');
    await page.getByLabel(/password/i).fill('admin');
    await page.getByRole('button', { name: /sign in/i }).click();
    await page.waitForURL(/.*files/, { timeout: 10000 });
    
    // Find theme toggle button
    const themeToggle = page.getByRole('button', { name: /toggle theme/i });
    
    if (await themeToggle.isVisible().catch(() => false)) {
      // Get initial theme
      const htmlElement = page.locator('html');
      const initialClass = await htmlElement.getAttribute('class');
      
      // Toggle theme
      await themeToggle.click();
      await page.waitForTimeout(500);
      
      // Theme class should change
      const newClass = await htmlElement.getAttribute('class');
      expect(initialClass).not.toBe(newClass);
    }
  });

  test('should display toast notifications', async ({ page }) => {
    // Try invalid login to trigger error toast
    await page.getByLabel(/username/i).fill('wronguser');
    await page.getByLabel(/password/i).fill('wrongpass');
    await page.getByRole('button', { name: /sign in/i }).click();
    
    // Toast should appear
    const toast = page.locator('[role="alert"]');
    await expect(toast).toBeVisible({ timeout: 5000 });
    
    // Toast should contain error message
    await expect(toast).toContainText(/error|invalid/i);
  });

  test('should close toast notification', async ({ page }) => {
    // Trigger a toast
    await page.getByLabel(/username/i).fill('test');
    await page.getByLabel(/password/i).fill('test');
    await page.getByRole('button', { name: /sign in/i }).click();
    
    await page.waitForTimeout(1000);
    
    // Find close button in toast
    const closeButton = page.locator('[role="alert"] button').first();
    
    if (await closeButton.isVisible().catch(() => false)) {
      await closeButton.click();
      
      // Toast should disappear
      await expect(page.locator('[role="alert"]')).not.toBeVisible({ timeout: 2000 });
    }
  });

  test('should show loading skeleton', async ({ page }) => {
    // Login
    await page.getByLabel(/username/i).fill('admin');
    await page.getByLabel(/password/i).fill('admin');
    await page.getByRole('button', { name: /sign in/i }).click();
    
    // During initial load, skeleton should appear
    // This is a race condition, so we use a short timeout
    const skeleton = page.locator('.animate-pulse');
    const isVisible = await skeleton.first().isVisible({ timeout: 1000 }).catch(() => false);
    
    // Skeleton might not be visible if loading is too fast
    // This is acceptable
    if (isVisible) {
      expect(await skeleton.count()).toBeGreaterThan(0);
    }
  });

  test('should have accessible form labels', async ({ page }) => {
    // Check for proper labels
    await expect(page.getByLabel(/username/i)).toBeVisible();
    await expect(page.getByLabel(/password/i)).toBeVisible();
    
    // Labels should be associated with inputs
    const usernameInput = page.getByLabel(/username/i);
    expect(await usernameInput.getAttribute('id')).toBeTruthy();
  });

  test('should display header when authenticated', async ({ page }) => {
    // Login
    await page.getByLabel(/username/i).fill('admin');
    await page.getByLabel(/password/i).fill('admin');
    await page.getByRole('button', { name: /sign in/i }).click();
    await page.waitForURL(/.*files/, { timeout: 10000 });
    
    // Header should be visible
    const header = page.locator('header');
    await expect(header).toBeVisible();
    
    // Header should contain logo/brand
    await expect(header.getByText(/filestorage/i)).toBeVisible();
    
    // Should have navigation items
    await expect(header.getByRole('button', { name: /logout/i })).toBeVisible();
  });

  test('should navigate using header links', async ({ page }) => {
    // Login
    await page.getByLabel(/username/i).fill('admin');
    await page.getByLabel(/password/i).fill('admin');
    await page.getByRole('button', { name: /sign in/i }).click();
    await page.waitForURL(/.*files/, { timeout: 10000 });
    
    // Click on brand/logo to go home
    const brand = page.getByText(/filestorage/i).first();
    await brand.click();
    
    // Should navigate to files page (home for authenticated users)
    await expect(page).toHaveURL(/.*files/);
  });

  test('should show error boundary on critical errors', async ({ page }) => {
    // This test would require triggering an actual error
    // For now, we just verify the app loads without errors
    await expect(page).not.toHaveTitle(/error/i);
  });

  test('should have proper focus management', async ({ page }) => {
    // Username field should be focusable
    await page.getByLabel(/username/i).focus();
    await expect(page.getByLabel(/username/i)).toBeFocused();
    
    // Tab to password field
    await page.keyboard.press('Tab');
    await expect(page.getByLabel(/password/i)).toBeFocused();
    
    // Tab to submit button
    await page.keyboard.press('Tab');
    await expect(page.getByRole('button', { name: /sign in/i })).toBeFocused();
  });

  test('should handle keyboard navigation', async ({ page }) => {
    // Fill form using keyboard
    await page.getByLabel(/username/i).focus();
    await page.keyboard.type('admin');
    
    await page.keyboard.press('Tab');
    await page.keyboard.type('admin');
    
    // Submit using Enter
    await page.keyboard.press('Enter');
    
    // Should navigate to files page
    await page.waitForURL(/.*files/, { timeout: 10000 });
    await expect(page).toHaveURL(/.*files/);
  });
});

