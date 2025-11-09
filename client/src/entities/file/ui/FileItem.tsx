import { memo } from 'react';
import { Download, Trash2, File } from 'lucide-react';
import { Button } from '../../../shared/ui/Button';
import { formatBytes, formatDate } from '../../../shared/lib/format';
import type { FileInfo } from '../../../shared/types/file.types';

interface FileItemProps {
    file: FileInfo;
    onDownload: (filename: string) => void;
    onDelete: (filename: string) => void;
    isDeleting?: boolean;
}

export const FileItem = memo(function FileItem({
    file,
    onDownload,
    onDelete,
    isDeleting = false
}: FileItemProps) {
    return (
        <div className="group flex flex-col sm:flex-row sm:items-center justify-between gap-3 p-4 border rounded-lg bg-card hover:bg-muted/50 hover:border-primary/50 transition-all duration-200 shadow-sm hover:shadow-md">
            <div className="flex items-start gap-3 flex-1 min-w-0">
                <div className="rounded-md bg-primary/10 p-2 mt-0.5 shrink-0">
                    <File className="h-5 w-5 text-primary" />
                </div>
            <div className="flex-1 min-w-0">
                    <p className="font-medium truncate text-foreground group-hover:text-primary transition-colors">
                        {file.name}
                    </p>
                    <div className="flex flex-wrap gap-x-3 gap-y-1 mt-1">
                        <p className="text-sm text-muted-foreground">
                            {formatBytes(file.size)}
                        </p>
                <p className="text-sm text-muted-foreground">
                            {formatDate(file.uploaded_at)}
                </p>
                    </div>
                </div>
            </div>
            <div className="flex gap-2 shrink-0">
                <Button
                    size="sm"
                    variant="outline"
                    onClick={() => onDownload(file.name)}
                    title="Download file"
                    className="flex-1 sm:flex-initial hover:bg-primary hover:text-primary-foreground transition-colors"
                >
                    <Download className="h-4 w-4 sm:mr-0 mr-2" />
                    <span className="sm:hidden">Download</span>
                </Button>
                <Button
                    size="sm"
                    variant="destructive"
                    onClick={() => onDelete(file.name)}
                    disabled={isDeleting}
                    title="Delete file"
                    className="flex-1 sm:flex-initial"
                >
                    <Trash2 className="h-4 w-4 sm:mr-0 mr-2" />
                    <span className="sm:hidden">Delete</span>
                </Button>
            </div>
        </div>
    );
});

