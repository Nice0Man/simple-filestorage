import { createContext, useContext, useState, useCallback } from 'react';
import type { ReactNode } from 'react';

interface LoaderContextType {
  isLoading: boolean;
  loadingCount: number;
  startLoading: () => void;
  stopLoading: () => void;
}

const LoaderContext = createContext<LoaderContextType | undefined>(undefined);

interface LoaderProviderProps {
  children: ReactNode;
}

export function LoaderProvider({ children }: LoaderProviderProps) {
  const [loadingCount, setLoadingCount] = useState(0);
  const [isLoading, setIsLoading] = useState(false);

  const startLoading = useCallback(() => {
    setLoadingCount((prev) => {
      const newCount = prev + 1;
      setIsLoading(true);
      return newCount;
    });
  }, []);

  const stopLoading = useCallback(() => {
    setLoadingCount((prev) => {
      const newCount = Math.max(0, prev - 1);
      setIsLoading(newCount > 0);
      return newCount;
    });
  }, []);

  const value: LoaderContextType = {
    isLoading,
    loadingCount,
    startLoading,
    stopLoading,
  };

  return <LoaderContext.Provider value={value}>{children}</LoaderContext.Provider>;
}

export function useLoader() {
  const context = useContext(LoaderContext);
  if (context === undefined) {
    throw new Error('useLoader must be used within a LoaderProvider');
  }
  return context;
}

