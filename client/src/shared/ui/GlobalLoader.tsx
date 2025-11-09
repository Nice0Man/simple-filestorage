import { useEffect, useState } from 'react';
import { cn } from '../lib/cn';

interface GlobalLoaderProps {
    isLoading: boolean;
}

export function GlobalLoader({ isLoading }: GlobalLoaderProps) {
    const [show, setShow] = useState(false);

    useEffect(() => {
        if (isLoading) {
            // Small delay to avoid showing loader for fast requests
            const timer = setTimeout(() => setShow(true), 200);
            return () => clearTimeout(timer);
        } else {
            setShow(false);
        }
    }, [isLoading]);

    if (!show) return null;

    return (
        <div className="fixed top-0 left-0 right-0 z-50">
            <div className={cn(
                "h-1 bg-primary transition-all duration-300 ease-out",
                "animate-pulse"
            )}>
                <div className="h-full w-full bg-gradient-to-r from-transparent via-primary-foreground to-transparent animate-shimmer" />
            </div>
        </div>
    );
}

