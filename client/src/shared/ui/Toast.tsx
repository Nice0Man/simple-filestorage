import { useEffect } from 'react';
import { X, CheckCircle, AlertCircle, AlertTriangle, Info } from 'lucide-react';
import { useToastStore, type Toast as ToastType } from '../model/toast.store';
import { cn } from '../lib/cn';

interface ToastProps {
    toast: ToastType;
}

function ToastItem({ toast }: ToastProps) {
    const { removeToast } = useToastStore();

    const icons = {
        success: <CheckCircle className="h-5 w-5" />,
        error: <AlertCircle className="h-5 w-5" />,
        warning: <AlertTriangle className="h-5 w-5" />,
        info: <Info className="h-5 w-5" />,
    };

    const styles = {
        success: 'bg-green-50 dark:bg-green-950 border-green-200 dark:border-green-800 text-green-900 dark:text-green-100',
        error: 'bg-red-50 dark:bg-red-950 border-red-200 dark:border-red-800 text-red-900 dark:text-red-100',
        warning: 'bg-yellow-50 dark:bg-yellow-950 border-yellow-200 dark:border-yellow-800 text-yellow-900 dark:text-yellow-100',
        info: 'bg-blue-50 dark:bg-blue-950 border-blue-200 dark:border-blue-800 text-blue-900 dark:text-blue-100',
    };

    useEffect(() => {
        const duration = toast.duration || 5000;
        if (duration > 0) {
            const timer = setTimeout(() => {
                removeToast(toast.id);
            }, duration);
            return () => clearTimeout(timer);
        }
    }, [toast.id, toast.duration, removeToast]);

    return (
        <div
            className={cn(
                'pointer-events-auto w-full max-w-sm overflow-hidden rounded-lg border shadow-lg transition-all animate-in slide-in-from-top-full',
                styles[toast.type]
            )}
            role="alert"
        >
            <div className="p-4">
                <div className="flex items-start">
                    <div className="flex-shrink-0">{icons[toast.type]}</div>
                    <div className="ml-3 w-0 flex-1">
                        <p className="text-sm font-medium">{toast.title}</p>
                        {toast.description && (
                            <p className="mt-1 text-sm opacity-90">{toast.description}</p>
                        )}
                    </div>
                    <div className="ml-4 flex flex-shrink-0">
                        <button
                            className="inline-flex rounded-md hover:opacity-75 focus:outline-none focus:ring-2 focus:ring-offset-2"
                            onClick={() => removeToast(toast.id)}
                        >
                            <span className="sr-only">Close</span>
                            <X className="h-5 w-5" />
                        </button>
                    </div>
                </div>
            </div>
        </div>
    );
}

export function ToastContainer() {
    const { toasts } = useToastStore();

    if (toasts.length === 0) return null;

    return (
        <div
            className="pointer-events-none fixed inset-0 z-50 flex flex-col items-end justify-start gap-2 p-4 sm:p-6"
            aria-live="assertive"
            aria-atomic="true"
        >
            {toasts.map((toast) => (
                <ToastItem key={toast.id} toast={toast} />
            ))}
        </div>
    );
}

