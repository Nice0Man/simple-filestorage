import { create } from 'zustand';

export type ToastType = 'success' | 'error' | 'warning' | 'info';

export interface Toast {
    id: string;
    type: ToastType;
    title: string;
    description?: string;
    duration?: number;
}

interface ToastState {
    toasts: Toast[];
    addToast: (toast: Omit<Toast, 'id'>) => void;
    removeToast: (id: string) => void;
    clearAll: () => void;
}

export const useToastStore = create<ToastState>((set) => ({
    toasts: [],

    addToast: (toast) => {
        const id = Math.random().toString(36).substring(7);
        const newToast = { ...toast, id };

        set((state) => ({
            toasts: [...state.toasts, newToast],
        }));

        // Auto-remove after duration
        const duration = toast.duration || 5000;
        if (duration > 0) {
            setTimeout(() => {
                set((state) => ({
                    toasts: state.toasts.filter((t) => t.id !== id),
                }));
            }, duration);
        }
    },

    removeToast: (id) =>
        set((state) => ({
            toasts: state.toasts.filter((t) => t.id !== id),
        })),

    clearAll: () => set({ toasts: [] }),
}));

// Helper functions for easier usage
export const toast = {
    success: (title: string, description?: string, duration?: number) => {
        useToastStore.getState().addToast({ type: 'success', title, description, duration });
    },
    error: (title: string, description?: string, duration?: number) => {
        useToastStore.getState().addToast({ type: 'error', title, description, duration });
    },
    warning: (title: string, description?: string, duration?: number) => {
        useToastStore.getState().addToast({ type: 'warning', title, description, duration });
    },
    info: (title: string, description?: string, duration?: number) => {
        useToastStore.getState().addToast({ type: 'info', title, description, duration });
    },
};

