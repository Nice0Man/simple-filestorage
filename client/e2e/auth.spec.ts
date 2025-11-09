import { test, expect } from '@playwright/test';

test.describe('Authentication Flow', () => {
  test.beforeEach(async ({ page }) => {
    // Navigate to the login page
    await page.goto('/');
  });

  test('should display login page', async ({ page }) => {
    // Check if we're redirected to login
    await expect(page).toHaveURL(/.*login/);
    
    // Check for login form elements
    await expect(page.getByRole('heading', { name: /sign in/i })).toBeVisible();
    await expect(page.getByLabel(/username/i)).toBeVisible();
    await expect(page.getByLabel(/password/i)).toBeVisible();
    await expect(page.getByRole('button', { name: /sign in/i })).toBeVisible();
  });

  test('should show validation errors for empty fields', async ({ page }) => {
    await page.goto('/login');
    
    // Try to submit empty form
    await page.getByRole('button', { name: /sign in/i }).click();
    
    // Wait for validation errors
    await expect(page.getByText(/username must be at least 3 characters/i)).toBeVisible({ timeout: 2000 });
    await expect(page.getByText(/password must be at least 6 characters/i)).toBeVisible({ timeout: 2000 });
  });

  test('should show validation error for invalid username', async ({ page }) => {
    await page.goto('/login');
    
    // Enter invalid username
    await page.getByLabel(/username/i).fill('ab'); // Too short
    await page.getByLabel(/password/i).fill('password123');
    await page.getByRole('button', { name: /sign in/i }).click();
    
    // Check for validation error
    await expect(page.getByText(/username must be at least 3 characters/i)).toBeVisible({ timeout: 2000 });
  });

  test('should login successfully with valid credentials', async ({ page }) => {
    await page.goto('/login');
    
    // Fill in credentials
    await page.getByLabel(/username/i).fill('admin');
    await page.getByLabel(/password/i).fill('admin123');
    
    // Submit form
    await page.getByRole('button', { name: /sign in/i }).click();
    
    // Wait for navigation to files page
    await expect(page).toHaveURL(/.*files/, { timeout: 15000 });
    
    // Check for success toast notification (use first() to avoid strict mode violation)
    await expect(page.getByText(/successfully/i).first()).toBeVisible({ timeout: 5000 });
    
    // Check for header with logout button
    await expect(page.getByRole('button', { name: /logout/i })).toBeVisible();
  });

  test('should show error for invalid credentials', async ({ page }) => {
    await page.goto('/login');
    
    // Fill in invalid credentials
    await page.getByLabel(/username/i).fill('wronguser');
    await page.getByLabel(/password/i).fill('wrongpass123');
    
    // Submit form
    await page.getByRole('button', { name: /sign in/i }).click();
    
    // Wait for error toast notification (from API or validation)
    const errorToast = page.locator('[role="alert"]').filter({ hasText: /error|invalid/i });
    await expect(errorToast).toBeVisible({ timeout: 5000 });
    
    // Should still be on login page
    await expect(page).toHaveURL(/.*login/);
  });

  test('should logout successfully', async ({ page }) => {
    // Login first
    await page.goto('/login');
    await page.getByLabel(/username/i).fill('admin');
    await page.getByLabel(/password/i).fill('admin123');
    await page.getByRole('button', { name: /sign in/i }).click();
    
    // Wait for navigation and verify we're on files page
    await expect(page).toHaveURL(/.*files/, { timeout: 15000 });
    await expect(page.getByRole('button', { name: /logout/i })).toBeVisible({ timeout: 5000 });
    
    // Click logout
    await page.getByRole('button', { name: /logout/i }).click();
    
    // Should redirect to login
    await expect(page).toHaveURL(/.*login/, { timeout: 5000 });
    
    // Should not be able to access files page
    await page.goto('/files');
    await expect(page).toHaveURL(/.*login/);
  });

  test('should persist authentication after page reload', async ({ page }) => {
    // Login
    await page.goto('/login');
    await page.getByLabel(/username/i).fill('admin');
    await page.getByLabel(/password/i).fill('admin123');
    await page.getByRole('button', { name: /sign in/i }).click();
    
    // Wait for navigation and verify we're on files page
    await expect(page).toHaveURL(/.*files/, { timeout: 15000 });
    await expect(page.getByRole('button', { name: /logout/i })).toBeVisible({ timeout: 5000 });
    
    // Reload page
    await page.reload();
    
    // Should still be authenticated
    await expect(page).toHaveURL(/.*files/);
    await expect(page.getByRole('button', { name: /logout/i })).toBeVisible();
  });

  test('should protect files route from unauthenticated access', async ({ page }) => {
    // Try to access files page without logging in
    await page.goto('/files');
    
    // Should redirect to login
    await expect(page).toHaveURL(/.*login/);
  });
});

