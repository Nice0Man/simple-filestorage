import { lazy, Suspense } from 'react';
import { Routes, Route, Navigate } from 'react-router-dom';
import { useAuth } from '../shared/context/AuthContext';
import { useLoader } from '../shared/context/LoaderContext';
import { ROUTES } from '../shared/config/routes.config';
import { Header } from '../widgets/ui/Header';
import { GlobalLoader } from '../shared/ui/GlobalLoader';
import { CardSkeleton } from '../shared/ui/Skeleton';

// Lazy load pages for code splitting
const LoginPage = lazy(() => import('../pages/ui/LoginPage'));
const FilesPage = lazy(() => import('../pages/ui/FilesPage'));
const ProfilePage = lazy(() => import('../pages/ui/ProfilePage'));
const AdminPage = lazy(() => import('../pages/ui/AdminPage'));

function ProtectedRoute({ children }: { children: React.ReactNode }) {
  const { isAuthenticated } = useAuth();
  return isAuthenticated ? <>{children}</> : <Navigate to={ROUTES.LOGIN} />;
}

function AdminRoute({ children }: { children: React.ReactNode }) {
  const { isAuthenticated, user } = useAuth();
  
  if (!isAuthenticated) {
    return <Navigate to={ROUTES.LOGIN} />;
  }
  
  if (user?.role !== 'admin') {
    return <Navigate to={ROUTES.FILES} />;
  }
  
  return <>{children}</>;
}

function App() {
  const { isAuthenticated } = useAuth();
  const { isLoading } = useLoader();

  return (
    <div className="min-h-screen bg-background text-foreground transition-colors duration-200">
      <GlobalLoader isLoading={isLoading} />
      {isAuthenticated && <Header />}
      <Suspense
        fallback={
          <div className="container mx-auto py-8 px-4">
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
            path={ROUTES.PROFILE}
            element={
              <ProtectedRoute>
                <ProfilePage />
              </ProtectedRoute>
            }
          />
          <Route
            path={ROUTES.ADMIN}
            element={
              <AdminRoute>
                <AdminPage />
              </AdminRoute>
            }
          />
        <Route
          path={ROUTES.HOME}
          element={
            isAuthenticated ? <Navigate to={ROUTES.FILES} /> : <Navigate to={ROUTES.LOGIN} />
          }
        />
          <Route path="*" element={<Navigate to={ROUTES.HOME} />} />
      </Routes>
      </Suspense>
    </div>
  );
}

export default App;

