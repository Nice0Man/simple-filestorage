import { createContext, useContext, useState, useEffect, useCallback } from 'react';
import type { ReactNode } from 'react';
import type { User } from '../types/auth.types';

interface AuthContextType {
  user: User | null;
  token: string | null;
  isAuthenticated: boolean;
  isLoading: boolean;
  login: (user: User, token: string) => void;
  logout: () => void;
  setLoading: (loading: boolean) => void;
}

const AuthContext = createContext<AuthContextType | undefined>(undefined);

const AUTH_STORAGE_KEY = 'auth-storage';

interface AuthProviderProps {
  children: ReactNode;
}

export function AuthProvider({ children }: AuthProviderProps) {
  const [isHydrated, setIsHydrated] = useState(false);
  const [user, setUser] = useState<User | null>(null);
  const [token, setToken] = useState<string | null>(null);
  const [isAuthenticated, setIsAuthenticated] = useState(false);
  const [isLoading, setIsLoading] = useState(false);

  // Load auth data from localStorage on mount (only once)
  useEffect(() => {
    try {
      const stored = localStorage.getItem(AUTH_STORAGE_KEY);
      console.log('[AuthContext] Loading from localStorage:', stored);
      if (stored) {
        const { user, token, isAuthenticated } = JSON.parse(stored);
        if (user && token && isAuthenticated) {
          console.log('[AuthContext] Restoring auth state:', { user: user.username, hasToken: !!token });
          setUser(user);
          setToken(token);
          setIsAuthenticated(true);
        }
      }
    } catch (error) {
      console.error('[AuthContext] Failed to load auth data:', error);
    } finally {
      // Mark as hydrated after a small delay to ensure all state updates are complete
      setTimeout(() => setIsHydrated(true), 0);
    }
  }, []);

  // Save auth data to localStorage whenever it changes (but only after hydration)
  useEffect(() => {
    if (!isHydrated) return;
    
    console.log('[AuthContext] Saving to localStorage:', { 
      hasUser: !!user, 
      hasToken: !!token, 
      isAuthenticated 
    });
    
    const data = { user, token, isAuthenticated };
    localStorage.setItem(AUTH_STORAGE_KEY, JSON.stringify(data));
  }, [user, token, isAuthenticated, isHydrated]);

  const login = useCallback((newUser: User, newToken: string) => {
    setUser(newUser);
    setToken(newToken);
    setIsAuthenticated(true);
  }, []);

  const logout = useCallback(() => {
    setUser(null);
    setToken(null);
    setIsAuthenticated(false);
    localStorage.removeItem(AUTH_STORAGE_KEY);
  }, []);

  const handleSetLoading = useCallback((loading: boolean) => {
    setIsLoading(loading);
  }, []);

  const value: AuthContextType = {
    user,
    token,
    isAuthenticated,
    isLoading,
    login,
    logout,
    setLoading: handleSetLoading,
  };

  return <AuthContext.Provider value={value}>{children}</AuthContext.Provider>;
}

export function useAuth() {
  const context = useContext(AuthContext);
  if (context === undefined) {
    throw new Error('useAuth must be used within an AuthProvider');
  }
  return context;
}

