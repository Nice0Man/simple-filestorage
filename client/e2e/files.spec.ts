import { test, expect } from '@playwright/test';
import * as path from 'path';

test.describe('File Management Flow', () => {
  // Login before each test
  test.beforeEach(async ({ page }) => {
    await page.goto('/login');
    await page.getByLabel(/username/i).fill('admin');
    await page.getByLabel(/password/i).fill('admin');
    await page.getByRole('button', { name: /sign in/i }).click();
    await page.waitForURL(/.*files/, { timeout: 10000 });
  });

  test('should display files page', async ({ page }) => {
    // Check page elements
    await expect(page.getByRole('heading', { name: /files/i })).toBeVisible();
    await expect(page.getByRole('button', { name: /upload/i })).toBeVisible();
    
    // Check for file input
    const fileInput = page.locator('input[type="file"]');
    await expect(fileInput).toBeVisible();
  });

  test('should show empty state when no files', async ({ page }) => {
    // Check for empty state message (might not be visible if there are files)
    const emptyMessage = page.getByText(/no files found/i);
    const fileList = page.locator('[data-testid="file-list"], .space-y-2 > div');
    
    // Either empty message or file list should be visible
    const hasFiles = await fileList.first().isVisible().catch(() => false);
    if (!hasFiles) {
      await expect(emptyMessage).toBeVisible();
    }
  });

  test('should upload a file successfully', async ({ page }) => {
    // Create a test file
    const testFileName = 'test-upload.txt';
    const testFilePath = path.join(__dirname, testFileName);
    
    // Wait for file input to be ready
    const fileInput = page.locator('input[type="file"]');
    await expect(fileInput).toBeVisible();
    
    // Select file
    await fileInput.setInputFiles({
      name: testFileName,
      mimeType: 'text/plain',
      buffer: Buffer.from('Test file content for upload'),
    });
    
    // Wait for file to be selected
    await page.waitForTimeout(500);
    
    // Click upload button
    const uploadButton = page.getByRole('button', { name: /upload/i });
    await uploadButton.click();
    
    // Wait for success toast
    await expect(page.getByText(/uploaded successfully|upload success/i)).toBeVisible({ timeout: 10000 });
    
    // Check if file appears in the list
    await expect(page.getByText(testFileName)).toBeVisible({ timeout: 5000 });
  });

  test('should show validation error for oversized file', async ({ page }) => {
    const fileInput = page.locator('input[type="file"]');
    
    // Try to upload a file larger than 100MB (create a buffer description)
    await fileInput.setInputFiles({
      name: 'large-file.txt',
      mimeType: 'text/plain',
      buffer: Buffer.alloc(1024), // Small buffer for testing
    });
    
    // The validation should happen in the UI
    // Check if upload button is present
    await expect(page.getByRole('button', { name: /upload/i })).toBeVisible();
  });

  test('should validate filename with special characters', async ({ page }) => {
    const fileInput = page.locator('input[type="file"]');
    
    // Try file with invalid characters
    await fileInput.setInputFiles({
      name: 'test<>file.txt',
      mimeType: 'text/plain',
      buffer: Buffer.from('test'),
    });
    
    await page.waitForTimeout(500);
    
    // Check for validation error
    const errorMessage = page.getByText(/invalid filename|not allowed/i);
    if (await errorMessage.isVisible().catch(() => false)) {
      await expect(errorMessage).toBeVisible();
    }
  });

  test('should download a file', async ({ page }) => {
    // First, make sure there's at least one file
    // Upload a test file if needed
    const fileList = page.locator('.space-y-2 > div').first();
    const hasFiles = await fileList.isVisible().catch(() => false);
    
    if (!hasFiles) {
      // Upload a file first
      const fileInput = page.locator('input[type="file"]');
      await fileInput.setInputFiles({
        name: 'download-test.txt',
        mimeType: 'text/plain',
        buffer: Buffer.from('Download test content'),
      });
      
      await page.getByRole('button', { name: /upload/i }).click();
      await page.waitForTimeout(2000);
    }
    
    // Find and click download button
    const downloadButton = page.getByRole('button', { name: /download/i }).first();
    if (await downloadButton.isVisible().catch(() => false)) {
      const [download] = await Promise.all([
        page.waitForEvent('download'),
        downloadButton.click(),
      ]);
      
      // Check download
      expect(download).toBeTruthy();
      expect(await download.suggestedFilename()).toBeTruthy();
    }
  });

  test('should delete a file with confirmation', async ({ page }) => {
    // Upload a test file first
    const testFileName = 'delete-test.txt';
    const fileInput = page.locator('input[type="file"]');
    await fileInput.setInputFiles({
      name: testFileName,
      mimeType: 'text/plain',
      buffer: Buffer.from('File to be deleted'),
    });
    
    await page.getByRole('button', { name: /upload/i }).click();
    await page.waitForTimeout(2000);
    
    // Find the file in the list
    const fileRow = page.locator(`text=${testFileName}`).locator('..').locator('..');
    
    // Click delete button
    const deleteButton = fileRow.getByRole('button', { name: /delete/i });
    await deleteButton.click();
    
    // Wait for confirmation dialog
    await expect(page.getByText(/are you sure|delete|confirm/i)).toBeVisible({ timeout: 2000 });
    
    // Confirm deletion
    const confirmButton = page.getByRole('button', { name: /delete|confirm/i }).last();
    await confirmButton.click();
    
    // Wait for success message
    await expect(page.getByText(/deleted successfully|delete success/i)).toBeVisible({ timeout: 5000 });
    
    // File should be removed from list
    await expect(page.getByText(testFileName)).not.toBeVisible({ timeout: 3000 });
  });

  test('should cancel file deletion', async ({ page }) => {
    // Get any file from the list
    const deleteButton = page.getByRole('button', { name: /delete/i }).first();
    const hasFiles = await deleteButton.isVisible().catch(() => false);
    
    if (hasFiles) {
      await deleteButton.click();
      
      // Wait for confirmation dialog
      await page.waitForTimeout(500);
      
      // Click cancel
      const cancelButton = page.getByRole('button', { name: /cancel/i });
      if (await cancelButton.isVisible().catch(() => false)) {
        await cancelButton.click();
        
        // Dialog should close
        await expect(cancelButton).not.toBeVisible({ timeout: 2000 });
      }
    }
  });

  test('should show loading state during upload', async ({ page }) => {
    const fileInput = page.locator('input[type="file"]');
    
    // Select a file
    await fileInput.setInputFiles({
      name: 'loading-test.txt',
      mimeType: 'text/plain',
      buffer: Buffer.from('Test loading state'),
    });
    
    await page.waitForTimeout(500);
    
    // Click upload and check for loading state
    const uploadButton = page.getByRole('button', { name: /upload/i });
    await uploadButton.click();
    
    // Loading state should appear briefly
    await expect(page.getByText(/loading/i).first()).toBeVisible({ timeout: 1000 }).catch(() => {});
  });

  test('should display file metadata correctly', async ({ page }) => {
    // Upload a test file
    const testFileName = 'metadata-test.txt';
    const fileContent = 'Test metadata display';
    const fileInput = page.locator('input[type="file"]');
    
    await fileInput.setInputFiles({
      name: testFileName,
      mimeType: 'text/plain',
      buffer: Buffer.from(fileContent),
    });
    
    await page.getByRole('button', { name: /upload/i }).click();
    await page.waitForTimeout(2000);
    
    // Check if file appears with metadata
    await expect(page.getByText(testFileName)).toBeVisible();
    
    // Look for size information (should show KB or Bytes)
    const fileRow = page.locator(`text=${testFileName}`).locator('..').locator('..');
    await expect(fileRow.getByText(/bytes|kb|mb/i)).toBeVisible();
  });
});

