import { cn } from '../lib/cn';

export function Skeleton({ 
    className, 
    ...props 
}: React.HTMLAttributes<HTMLDivElement>) {
    return (
        <div
            className={cn('animate-pulse rounded-md bg-muted', className)}
            {...props}
        />
    );
}

export function FileItemSkeleton() {
    return (
        <div className="flex items-center justify-between p-4 border rounded-lg">
            <div className="space-y-2 flex-1">
                <Skeleton className="h-5 w-1/3" />
                <Skeleton className="h-4 w-1/2" />
            </div>
            <div className="flex space-x-2">
                <Skeleton className="h-9 w-9" />
                <Skeleton className="h-9 w-9" />
            </div>
        </div>
    );
}

export function FileListSkeleton({ count = 3 }: { count?: number }) {
    return (
        <div className="space-y-2">
            {Array.from({ length: count }).map((_, i) => (
                <FileItemSkeleton key={i} />
            ))}
        </div>
    );
}

export function CardSkeleton() {
    return (
        <div className="rounded-lg border bg-card p-6 shadow-sm space-y-4">
            <div className="space-y-2">
                <Skeleton className="h-8 w-1/3" />
                <Skeleton className="h-4 w-2/3" />
            </div>
            <Skeleton className="h-32 w-full" />
        </div>
    );
}

