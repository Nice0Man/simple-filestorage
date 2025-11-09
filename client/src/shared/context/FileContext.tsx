import { createContext, useContext, useState, useCallback } from 'react';
import type { ReactNode } from 'react';
import type { FileInfo } from '../types/file.types';

interface FileContextType {
  files: FileInfo[];
  isLoading: boolean;
  error: string | null;
  setFiles: (files: FileInfo[]) => void;
  addFile: (file: FileInfo) => void;
  removeFile: (filename: string) => void;
  setLoading: (loading: boolean) => void;
  setError: (error: string | null) => void;
}

const FileContext = createContext<FileContextType | undefined>(undefined);

interface FileProviderProps {
  children: ReactNode;
}

export function FileProvider({ children }: FileProviderProps) {
  const [files, setFilesState] = useState<FileInfo[]>([]);
  const [isLoading, setIsLoading] = useState(false);
  const [error, setErrorState] = useState<string | null>(null);

  const setFiles = useCallback((newFiles: FileInfo[]) => {
    setFilesState(newFiles);
    setErrorState(null);
  }, []);

  const addFile = useCallback((file: FileInfo) => {
    setFilesState((prev) => [...prev, file]);
  }, []);

  const removeFile = useCallback((filename: string) => {
    setFilesState((prev) => prev.filter((f) => f.name !== filename));
  }, []);

  const setLoading = useCallback((loading: boolean) => {
    setIsLoading(loading);
  }, []);

  const setError = useCallback((err: string | null) => {
    setErrorState(err);
  }, []);

  const value: FileContextType = {
    files,
    isLoading,
    error,
    setFiles,
    addFile,
    removeFile,
    setLoading,
    setError,
  };

  return <FileContext.Provider value={value}>{children}</FileContext.Provider>;
}

export function useFiles() {
  const context = useContext(FileContext);
  if (context === undefined) {
    throw new Error('useFiles must be used within a FileProvider');
  }
  return context;
}

