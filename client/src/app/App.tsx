import { lazy, Suspense } from 'react';
import { Routes, Route, Navigate } from 'react-router-dom';
import { useAuthStore } from '../shared/model/auth.store';
import { useLoaderStore } from '../shared/model/loader.store';
import { ROUTES } from '../shared/config/routes.config';
import { Header } from '../widgets/ui/Header';
import { GlobalLoader } from '../shared/ui/GlobalLoader';
import { CardSkeleton } from '../shared/ui/Skeleton';

// Lazy load pages for code splitting
const LoginPage = lazy(() => import('../pages/ui/LoginPage'));
const FilesPage = lazy(() => import('../pages/ui/FilesPage'));

function ProtectedRoute({ children }: { children: React.ReactNode }) {
  const { isAuthenticated } = useAuthStore();
  return isAuthenticated ? <>{children}</> : <Navigate to={ROUTES.LOGIN} />;
}

function App() {
  const { isAuthenticated } = useAuthStore();
  const { isLoading } = useLoaderStore();

  return (
    <div className="min-h-screen bg-background">
      <GlobalLoader isLoading={isLoading} />
      {isAuthenticated && <Header />}
      <Suspense
        fallback={
          <div className="container py-8">
            <CardSkeleton />
          </div>
        }
      >
        <Routes>
          <Route path={ROUTES.LOGIN} element={<LoginPage />} />
          <Route
            path={ROUTES.FILES}
            element={
              <ProtectedRoute>
                <FilesPage />
              </ProtectedRoute>
            }
          />
          <Route
            path={ROUTES.HOME}
            element={
              isAuthenticated ? <Navigate to={ROUTES.FILES} /> : <Navigate to={ROUTES.LOGIN} />
            }
          />
        </Routes>
      </Suspense>
    </div>
  );
}

export default App;

