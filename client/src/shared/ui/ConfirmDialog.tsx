import { forwardRef, type HTMLAttributes } from 'react';
import { X } from 'lucide-react';
import { cn } from '../lib/cn';
import { Button } from './Button';

interface ConfirmDialogProps {
    open: boolean;
    onClose: () => void;
    onConfirm: () => void;
    title: string;
    description?: string;
    confirmText?: string;
    cancelText?: string;
    isLoading?: boolean;
    variant?: 'default' | 'destructive';
}

export function ConfirmDialog({
    open,
    onClose,
    onConfirm,
    title,
    description,
    confirmText = 'Confirm',
    cancelText = 'Cancel',
    isLoading = false,
    variant = 'default',
}: ConfirmDialogProps) {
    if (!open) return null;

    const handleBackdropClick = (e: React.MouseEvent) => {
        if (e.target === e.currentTarget) {
            onClose();
        }
    };

    const handleConfirm = () => {
        onConfirm();
        if (!isLoading) {
            onClose();
        }
    };

    return (
        <div
            className="fixed inset-0 z-50 flex items-center justify-center bg-black/50 backdrop-blur-sm"
            onClick={handleBackdropClick}
        >
            <div
                className="relative w-full max-w-md rounded-lg border bg-background p-6 shadow-lg"
                role="dialog"
                aria-modal="true"
                aria-labelledby="dialog-title"
            >
                <button
                    onClick={onClose}
                    className="absolute right-4 top-4 rounded-sm opacity-70 ring-offset-background transition-opacity hover:opacity-100 focus:outline-none focus:ring-2 focus:ring-ring focus:ring-offset-2 disabled:pointer-events-none"
                    disabled={isLoading}
                >
                    <X className="h-4 w-4" />
                    <span className="sr-only">Close</span>
                </button>

                <div className="space-y-4">
                    <div className="space-y-2">
                        <h2
                            id="dialog-title"
                            className="text-lg font-semibold leading-none tracking-tight"
                        >
                            {title}
                        </h2>
                        {description && (
                            <p className="text-sm text-muted-foreground">{description}</p>
                        )}
                    </div>

                    <div className="flex justify-end space-x-2">
                        <Button
                            variant="outline"
                            onClick={onClose}
                            disabled={isLoading}
                        >
                            {cancelText}
                        </Button>
                        <Button
                            variant={variant === 'destructive' ? 'destructive' : 'default'}
                            onClick={handleConfirm}
                            disabled={isLoading}
                        >
                            {isLoading ? 'Loading...' : confirmText}
                        </Button>
                    </div>
                </div>
            </div>
        </div>
    );
}

// Dialog container components for more complex dialogs
const Dialog = forwardRef<HTMLDivElement, HTMLAttributes<HTMLDivElement>>(
    ({ className, ...props }, ref) => (
        <div
            ref={ref}
            className={cn(
                'fixed inset-0 z-50 flex items-center justify-center bg-black/50 backdrop-blur-sm',
                className
            )}
            {...props}
        />
    )
);
Dialog.displayName = 'Dialog';

const DialogContent = forwardRef<HTMLDivElement, HTMLAttributes<HTMLDivElement>>(
    ({ className, ...props }, ref) => (
        <div
            ref={ref}
            className={cn(
                'relative w-full max-w-lg rounded-lg border bg-background p-6 shadow-lg',
                className
            )}
            role="dialog"
            aria-modal="true"
            {...props}
        />
    )
);
DialogContent.displayName = 'DialogContent';

const DialogHeader = forwardRef<HTMLDivElement, HTMLAttributes<HTMLDivElement>>(
    ({ className, ...props }, ref) => (
        <div
            ref={ref}
            className={cn('flex flex-col space-y-1.5 text-center sm:text-left', className)}
            {...props}
        />
    )
);
DialogHeader.displayName = 'DialogHeader';

const DialogTitle = forwardRef<HTMLHeadingElement, HTMLAttributes<HTMLHeadingElement>>(
    ({ className, ...props }, ref) => (
        <h2
            ref={ref}
            className={cn('text-lg font-semibold leading-none tracking-tight', className)}
            {...props}
        />
    )
);
DialogTitle.displayName = 'DialogTitle';

const DialogDescription = forwardRef<HTMLParagraphElement, HTMLAttributes<HTMLParagraphElement>>(
    ({ className, ...props }, ref) => (
        <p
            ref={ref}
            className={cn('text-sm text-muted-foreground', className)}
            {...props}
        />
    )
);
DialogDescription.displayName = 'DialogDescription';

const DialogFooter = forwardRef<HTMLDivElement, HTMLAttributes<HTMLDivElement>>(
    ({ className, ...props }, ref) => (
        <div
            ref={ref}
            className={cn('flex flex-col-reverse sm:flex-row sm:justify-end sm:space-x-2', className)}
            {...props}
        />
    )
);
DialogFooter.displayName = 'DialogFooter';

export {
    Dialog,
    DialogContent,
    DialogHeader,
    DialogTitle,
    DialogDescription,
    DialogFooter,
};

