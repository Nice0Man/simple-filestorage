import { memo } from 'react';
import { User } from 'lucide-react';
import { cn } from '../../../shared/lib/cn';
import type { User as UserType } from '../../../shared/types/auth.types';

interface UserAvatarProps {
    user: UserType;
    size?: 'sm' | 'md' | 'lg';
    showName?: boolean;
    className?: string;
}

const sizeClasses = {
    sm: 'h-8 w-8',
    md: 'h-10 w-10',
    lg: 'h-12 w-12',
};

export const UserAvatar = memo(function UserAvatar({
    user,
    size = 'md',
    showName = false,
    className,
}: UserAvatarProps) {
    const initials = user.username
        .split(' ')
        .map((n) => n[0])
        .join('')
        .toUpperCase()
        .slice(0, 2);

    return (
        <div className={cn('flex items-center gap-2', className)}>
            <div
                className={cn(
                    'flex items-center justify-center rounded-full bg-primary text-primary-foreground font-medium',
                    sizeClasses[size]
                )}
            >
                {initials || <User className="h-4 w-4" />}
            </div>
            {showName && (
                <div className="flex flex-col">
                    <span className="text-sm font-medium">{user.username}</span>
                    {user.email && <span className="text-xs text-muted-foreground">{user.email}</span>}
                </div>
            )}
        </div>
    );
});

