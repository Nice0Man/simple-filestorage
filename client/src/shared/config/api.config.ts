export const API_CONFIG = {
  BASE_URL: import.meta.env.VITE_API_BASE_URL || 'http://localhost:8080/api/v1',
  TIMEOUT: 30000,
  HEADERS: {
    'Content-Type': 'application/json',
  },
} as const;

export const API_ENDPOINTS = {
  // Auth
  AUTH: {
    LOGIN: '/auth/login',
    LOGOUT: '/auth/logout',
    ME: '/auth/me',
    CHANGE_PASSWORD: '/auth/change-password',
  },
  // Files
  FILES: {
    LIST: '/files',
    UPLOAD: '/files',
    DOWNLOAD: (filename: string) => `/files/${filename}`,
    DELETE: (filename: string) => `/files/${filename}`,
    INFO: (filename: string) => `/files/${filename}/info`,
  },
  // Admin
  ADMIN: {
    USERS: '/admin/users',
    USER: (username: string) => `/admin/users/${username}`,
    USER_ROLE: (username: string) => `/admin/users/${username}/role`,
  },
  // System
  SYSTEM: {
    HEALTH: '/health',
    METRICS: '/metrics',
    VERSION: '/version',
  },
} as const;

