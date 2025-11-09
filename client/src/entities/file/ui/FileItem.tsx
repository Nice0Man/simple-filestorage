import { memo } from 'react';
import { Download, Trash2 } from 'lucide-react';
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
        <div className="flex items-center justify-between p-4 border rounded-lg hover:bg-muted/50 transition-colors">
            <div>
                <p className="font-medium">{file.name}</p>
                <p className="text-sm text-muted-foreground">
                    {formatBytes(file.size)} • {formatDate(file.uploaded_at)}
                </p>
            </div>
            <div className="flex space-x-2">
                <Button
                    size="sm"
                    variant="outline"
                    onClick={() => onDownload(file.name)}
                    title="Download file"
                >
                    <Download className="h-4 w-4" />
                </Button>
                <Button
                    size="sm"
                    variant="destructive"
                    onClick={() => onDelete(file.name)}
                    disabled={isDeleting}
                    title="Delete file"
                >
                    <Trash2 className="h-4 w-4" />
                </Button>
            </div>
        </div>
    );
});

