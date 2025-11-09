import { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '../../shared/ui/Card';
import { ConfirmDialog } from '../../shared/ui/ConfirmDialog';
import { FileListSkeleton } from '../../shared/ui/Skeleton';
import { FileUploader } from '../../features/file-upload/ui/FileUploader';
import { FileItem } from '../../entities/file/ui/FileItem';
import { useFiles } from '../../shared/context/FileContext';
import { fileApi } from '../../shared/api/file.api';
import { toast } from '../../shared/model/toast.store';

function FilesPage() {
  const { t } = useTranslation();
  const { files, setFiles, removeFile, setLoading, isLoading } = useFiles();
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
      setFiles(response.data?.files || []);
    } catch (error) {
      console.error('Failed to load files:', error);
      setFiles([]); // Set empty array on error
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
      setFiles(response.data?.files || []);
    } catch (error) {
      console.error('Failed to reload files:', error);
      setFiles([]);
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
      toast.success(t('files.downloadSuccess') || 'Download started', `Downloading "${filename}"`);
    } catch (error) {
      console.error('Failed to download file:', error);
      const message = error instanceof Error ? error.message : 'Download failed';
      toast.error(t('files.downloadError') || 'Download failed', message);
      
      // If file not found (404), refresh the file list to sync state
      if (message.includes('404')) {
        setLoading(true);
        try {
          const response = await fileApi.list();
          setFiles(response.data?.files || []);
        } catch (refreshError) {
          console.error('Failed to refresh files:', refreshError);
        } finally {
          setLoading(false);
        }
      }
    }
  };

  return (
    <div className="container mx-auto max-w-6xl py-6 px-4 md:py-8">
      <Card className="shadow-lg">
        <CardHeader className="space-y-1">
          <CardTitle className="text-2xl md:text-3xl font-bold">{t('files.title')}</CardTitle>
          <CardDescription className="text-base">Manage your files - upload, download, and delete</CardDescription>
        </CardHeader>
        <CardContent className="space-y-6">
          <FileUploader onUploadSuccess={handleUploadSuccess} />

          <div className="space-y-4">
            {isLoading ? (
              <FileListSkeleton count={5} />
            ) : !files || files.length === 0 ? (
              <div className="flex flex-col items-center justify-center py-16 text-center rounded-lg border-2 border-dashed border-border bg-muted/20">
                <div className="w-16 h-16 rounded-full bg-muted flex items-center justify-center mb-4">
                  <svg className="w-8 h-8 text-muted-foreground" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                    <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M7 21h10a2 2 0 002-2V9.414a1 1 0 00-.293-.707l-5.414-5.414A1 1 0 0012.586 3H7a2 2 0 00-2 2v14a2 2 0 002 2z" />
                  </svg>
                </div>
                <p className="text-muted-foreground text-lg font-medium">{t('files.noFiles')}</p>
                <p className="text-sm text-muted-foreground mt-2 max-w-sm">Upload your first file to get started with file management</p>
              </div>
            ) : (
              <>
                <div className="flex items-center justify-between pb-2 border-b">
                  <p className="text-sm font-medium text-muted-foreground">
                    {files.length} {files.length === 1 ? 'file' : 'files'}
                  </p>
                </div>
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
              </>
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
