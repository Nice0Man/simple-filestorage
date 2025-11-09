import { useTranslation } from 'react-i18next';
import { Navigate } from 'react-router-dom';
import { FileText } from 'lucide-react';
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '../../shared/ui/Card';
import { LoginForm } from '../../features/auth/ui/LoginForm';
import { useAuth } from '../../shared/context/AuthContext';
import { ROUTES } from '../../shared/config/routes.config';

function LoginPage() {
  const { t } = useTranslation();
  const { isAuthenticated } = useAuth();

  // Redirect to files if already authenticated
  if (isAuthenticated) {
    return <Navigate to={ROUTES.FILES} replace />;
  }

  return (
    <div className="min-h-screen bg-background flex flex-col items-center justify-center px-4 py-8 overflow-y-auto">
      <div className="w-full max-w-md space-y-8 my-auto">
        <div className="flex flex-col items-center justify-center space-y-2">
          <div className="rounded-full bg-primary/10 p-4">
            <FileText className="h-10 w-10 text-primary" />
          </div>
          <h1 className="text-3xl font-bold tracking-tight">FileStorage</h1>
          <p className="text-muted-foreground text-center">Secure and simple file management</p>
        </div>
        
        <Card className="shadow-lg border-2">
          <CardHeader className="space-y-1">
            <CardTitle className="text-2xl text-center">{t('auth.loginTitle')}</CardTitle>
            <CardDescription className="text-center">
              Enter your credentials to access your account
            </CardDescription>
          </CardHeader>
          <CardContent>
            <LoginForm />
          </CardContent>
        </Card>
      </div>
    </div>
  );
}

export default LoginPage;

