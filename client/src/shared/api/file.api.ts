import { API_ENDPOINTS } from '../config/api.config';
import type { FileListResponse, FileInfo, UploadFileResponse, DeleteFileResponse } from '../types/file.types';
import { apiClient } from './client';

export const fileApi = {
  list: async (): Promise<FileListResponse> => {
    return apiClient.get<FileListResponse>(API_ENDPOINTS.FILES.LIST);
  },

  upload: async (file: File): Promise<UploadFileResponse> => {
    const formData = new FormData();
    formData.append('file', file);
    return apiClient.upload<UploadFileResponse>(API_ENDPOINTS.FILES.UPLOAD, formData);
  },

  download: async (filename: string): Promise<Blob> => {
    return apiClient.get<Blob>(API_ENDPOINTS.FILES.DOWNLOAD(filename), {
      responseType: 'blob',
    });
  },

  delete: async (filename: string): Promise<DeleteFileResponse> => {
    return apiClient.delete<DeleteFileResponse>(API_ENDPOINTS.FILES.DELETE(filename));
  },

  getInfo: async (filename: string): Promise<FileInfo> => {
    return apiClient.get<FileInfo>(API_ENDPOINTS.FILES.INFO(filename));
  },
};

