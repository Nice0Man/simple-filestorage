import { API_ENDPOINTS } from '../config/api.config';
import type { LoginRequest, LoginResponse, User, ChangePasswordRequest } from '../types/auth.types';
import { apiClient } from './client';

export const authApi = {
  login: async (credentials: LoginRequest): Promise<LoginResponse> => {
    return apiClient.post<LoginResponse>(API_ENDPOINTS.AUTH.LOGIN, credentials);
  },

  logout: async (): Promise<void> => {
    return apiClient.post<void>(API_ENDPOINTS.AUTH.LOGOUT);
  },

  getMe: async (): Promise<User> => {
    return apiClient.get<User>(API_ENDPOINTS.AUTH.ME);
  },

  changePassword: async (data: ChangePasswordRequest): Promise<void> => {
    return apiClient.post<void>(API_ENDPOINTS.AUTH.CHANGE_PASSWORD, data);
  },
};

