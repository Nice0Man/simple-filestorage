import { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '../../shared/ui/Card';
import { ConfirmDialog } from '../../shared/ui/ConfirmDialog';
import { FileListSkeleton } from '../../shared/ui/Skeleton';
import { FileUploader } from '../../features/file-upload/ui/FileUploader';
import { FileItem } from '../../entities/file/ui/FileItem';
import { useFileStore } from '../../shared/model/file.store';
import { fileApi } from '../../shared/api/file.api';
import { toast } from '../../shared/model/toast.store';

function FilesPage() {
  const { t } = useTranslation();
  const { files, setFiles, removeFile, setLoading, isLoading } = useFileStore();
  const [isDeleting, setIsDeleting] = useState(false);
  const [deleteConfirm, setDeleteConfirm] = useState<{ open: boolean; filename: string }>({
    open: false,
    filename: '',
  });

  useEffect(() => {
    const loadFiles = async () => {
      setLoading(true);
      try {
        const response = await fileApi.list();
        setFiles(response.files);
      } catch (error) {
        console.error('Failed to load files:', error);
      } finally {
        setLoading(false);
      }
    };

    loadFiles();
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  const handleUploadSuccess = async () => {
    setLoading(true);
    try {
      const response = await fileApi.list();
      setFiles(response.files);
    } finally {
      setLoading(false);
    }
  };

  const handleDeleteClick = (filename: string) => {
    setDeleteConfirm({ open: true, filename });
  };

  const handleDeleteConfirm = async () => {
    setIsDeleting(true);
    try {
      await fileApi.delete(deleteConfirm.filename);
      removeFile(deleteConfirm.filename);
      toast.success(t('files.deleteSuccess'), `File "${deleteConfirm.filename}" deleted`);
      setDeleteConfirm({ open: false, filename: '' });
    } catch (error) {
      const message = error instanceof Error ? error.message : 'Delete failed';
      toast.error(t('files.deleteError'), message);
      console.error('Failed to delete file:', error);
    } finally {
      setIsDeleting(false);
    }
  };

  const handleDownload = async (filename: string) => {
    try {
      const blob = await fileApi.download(filename);
      const url = window.URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = filename;
      a.click();
      window.URL.revokeObjectURL(url);
    } catch (error) {
      console.error('Failed to download file:', error);
    }
  };

  return (
    <div className="container py-8">
      <Card>
        <CardHeader>
          <CardTitle>{t('files.title')}</CardTitle>
          <CardDescription>Manage your files</CardDescription>
        </CardHeader>
        <CardContent>
          <div className="space-y-4">
            <FileUploader onUploadSuccess={handleUploadSuccess} />

            {isLoading ? (
              <FileListSkeleton count={5} />
            ) : files.length === 0 ? (
              <p className="text-muted-foreground">{t('files.noFiles')}</p>
            ) : (
              <div className="space-y-2">
                {files.map((file) => (
                  <FileItem
                    key={file.name}
                    file={file}
                    onDownload={handleDownload}
                    onDelete={handleDeleteClick}
                    isDeleting={isDeleting}
                  />
                ))}
              </div>
            )}
          </div>
        </CardContent>
      </Card>

      <ConfirmDialog
        open={deleteConfirm.open}
        onClose={() => setDeleteConfirm({ open: false, filename: '' })}
        onConfirm={handleDeleteConfirm}
        title={t('files.delete')}
        description={t('files.deleteConfirm')}
        confirmText={t('common.delete')}
        cancelText={t('common.cancel')}
        isLoading={isDeleting}
        variant="destructive"
      />
    </div>
  );
}

export default FilesPage;
