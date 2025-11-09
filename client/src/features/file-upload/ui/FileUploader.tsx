import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Upload } from 'lucide-react';
import { Button } from '../../../shared/ui/Button';
import { Input } from '../../../shared/ui/Input';
import { toast } from '../../../shared/model/toast.store';
import { fileApi } from '../../../shared/api/file.api';
import { sanitizeFilename } from '../../../shared/lib/sanitize';

const MAX_FILE_SIZE = 100 * 1024 * 1024; // 100MB

interface FileUploaderProps {
    onUploadSuccess?: () => void;
}

export function FileUploader({ onUploadSuccess }: FileUploaderProps) {
    const { t } = useTranslation();
    const [file, setFile] = useState<File | null>(null);
    const [isUploading, setIsUploading] = useState(false);
    const [validationError, setValidationError] = useState<string>('');

    const validateFile = (file: File): string | null => {
        // Check file size
        if (file.size > MAX_FILE_SIZE) {
            return `File size must be less than ${MAX_FILE_SIZE / 1024 / 1024}MB`;
        }

        // Check filename
        const sanitized = sanitizeFilename(file.name);
        if (sanitized !== file.name) {
            return 'Invalid filename. Please use only letters, numbers, dots, hyphens, and underscores';
        }

        // Check for dangerous extensions
        const dangerousExtensions = ['.exe', '.bat', '.cmd', '.sh', '.app'];
        const extension = file.name.toLowerCase().slice(file.name.lastIndexOf('.'));
        if (dangerousExtensions.includes(extension)) {
            return 'This file type is not allowed';
        }

        return null;
    };

    const handleFileChange = (e: React.ChangeEvent<HTMLInputElement>) => {
        const selectedFile = e.target.files?.[0] || null;
        setValidationError('');

        if (selectedFile) {
            const error = validateFile(selectedFile);
            if (error) {
                setValidationError(error);
                setFile(null);
                return;
            }
            setFile(selectedFile);
        } else {
            setFile(null);
        }
    };

    const handleUpload = async () => {
        if (!file) return;

        setIsUploading(true);
        try {
            await fileApi.upload(file);
            toast.success(t('files.uploadSuccess'), `File "${file.name}" uploaded successfully`);
            setFile(null);
            // Reset input
            const input = document.querySelector<HTMLInputElement>('input[type="file"]');
            if (input) input.value = '';

            onUploadSuccess?.();
        } catch (error) {
            const message = error instanceof Error ? error.message : 'Upload failed';
            toast.error(t('files.uploadError'), message);
        } finally {
            setIsUploading(false);
        }
    };

    return (
        <div className="rounded-lg border-2 border-dashed border-border bg-muted/20 p-6 hover:border-primary/50 transition-colors">
            <div className="space-y-4">
                <div className="flex flex-col sm:flex-row gap-3">
                    <div className="flex-1">
                        <Input
                            type="file"
                            onChange={handleFileChange}
                            disabled={isUploading}
                            className={`${validationError ? 'border-destructive' : 'hover:border-primary'} transition-colors cursor-pointer`}
                        />
                    </div>
                    <Button 
                        onClick={handleUpload} 
                        disabled={!file || isUploading || !!validationError}
                        className="w-full sm:w-auto min-w-[120px] font-medium"
                        size="default"
                    >
                        <Upload className="h-4 w-4 mr-2" />
                        {isUploading ? t('common.loading') : t('files.upload')}
                    </Button>
                </div>
                {validationError && (
                    <div className="flex items-start gap-2 p-3 rounded-md bg-destructive/10 border border-destructive/20">
                        <svg className="w-5 h-5 text-destructive shrink-0 mt-0.5" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                            <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M12 8v4m0 4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
                        </svg>
                        <p className="text-sm text-destructive font-medium">{validationError}</p>
                    </div>
                )}
                {file && !validationError && (
                    <div className="flex items-center gap-2 p-3 rounded-md bg-primary/10 border border-primary/20">
                        <svg className="w-5 h-5 text-primary shrink-0" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                            <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z" />
                        </svg>
                        <p className="text-sm text-primary font-medium">
                        Selected: {file.name} ({(file.size / 1024).toFixed(2)} KB)
                    </p>
                    </div>
                )}
            </div>
        </div>
    );
}

