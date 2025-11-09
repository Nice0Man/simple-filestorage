import { type ReactNode } from 'react';
import { BrowserRouter } from 'react-router-dom';
import { ErrorBoundary } from './ErrorBoundary';
import { ToastContainer } from '../../shared/ui/Toast';
import { AuthProvider, FileProvider, ThemeProvider, LoaderProvider } from '../../shared/context';
import '../styles/index.css';
import '../../shared/config/i18n.config';

interface AppProviderProps {
  children: ReactNode;
}

export function AppProvider({ children }: AppProviderProps) {
  return (
    <ErrorBoundary>
    <BrowserRouter>
        <ThemeProvider>
          <AuthProvider>
            <LoaderProvider>
              <FileProvider>
      {children}
        <ToastContainer />
              </FileProvider>
            </LoaderProvider>
          </AuthProvider>
        </ThemeProvider>
    </BrowserRouter>
    </ErrorBoundary>
  );
}

