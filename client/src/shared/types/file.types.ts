export interface FileInfo {
  name: string;
  size: number;
  type: string;
  uploaded_at: string;
  path?: string;
}

export interface FileListResponse {
  success: boolean;
  data: {
    files: FileInfo[];
    count: number;
  };
}

export interface UploadFileResponse {
  success: boolean;
  file: FileInfo;
  message?: string;
}

export interface DeleteFileResponse {
  success: boolean;
  message: string;
}

