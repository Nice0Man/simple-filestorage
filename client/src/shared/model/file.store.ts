import { create } from 'zustand';
import type { FileInfo } from '../types/file.types';

interface FileState {
  files: FileInfo[];
  isLoading: boolean;
  error: string | null;
  setFiles: (files: FileInfo[]) => void;
  addFile: (file: FileInfo) => void;
  removeFile: (filename: string) => void;
  setLoading: (loading: boolean) => void;
  setError: (error: string | null) => void;
}

export const useFileStore = create<FileState>((set) => ({
  files: [],
  isLoading: false,
  error: null,

  setFiles: (files) => set({ files, error: null }),

  addFile: (file) => set((state) => ({ files: [...state.files, file] })),

  removeFile: (filename) =>
    set((state) => ({
      files: state.files.filter((f) => f.name !== filename),
    })),

  setLoading: (isLoading) => set({ isLoading }),

  setError: (error) => set({ error }),
}));

