import { useTranslation } from 'react-i18next';
import { User, Mail, Shield } from 'lucide-react';
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '../../shared/ui/Card';
import { useAuth } from '../../shared/context/AuthContext';

function ProfilePage() {
  const { t } = useTranslation();
  const { user } = useAuth();

  if (!user) {
    return null;
  }

  return (
    <div className="container mx-auto max-w-4xl py-6 px-4 md:py-8">
      <Card className="shadow-lg">
        <CardHeader className="space-y-1">
          <div className="flex items-center gap-3">
            <div className="rounded-full bg-primary/10 p-3">
              <User className="h-8 w-8 text-primary" />
            </div>
            <div>
              <CardTitle className="text-2xl md:text-3xl font-bold">
                {t('profile.title') || 'Profile'}
              </CardTitle>
              <CardDescription>View and manage your account information</CardDescription>
            </div>
          </div>
        </CardHeader>
        <CardContent className="space-y-6">
          <div className="space-y-4">
            <div className="flex items-start gap-4 p-4 rounded-lg border bg-card hover:bg-muted/50 transition-colors">
              <div className="rounded-md bg-primary/10 p-2 mt-1">
                <User className="h-5 w-5 text-primary" />
              </div>
              <div className="flex-1">
                <p className="text-sm font-medium text-muted-foreground">Username</p>
                <p className="text-lg font-semibold">{user.username}</p>
              </div>
            </div>

            <div className="flex items-start gap-4 p-4 rounded-lg border bg-card hover:bg-muted/50 transition-colors">
              <div className="rounded-md bg-primary/10 p-2 mt-1">
                <Mail className="h-5 w-5 text-primary" />
              </div>
              <div className="flex-1">
                <p className="text-sm font-medium text-muted-foreground">Email</p>
                <p className="text-lg font-semibold">{user.email || 'Not set'}</p>
              </div>
            </div>

            <div className="flex items-start gap-4 p-4 rounded-lg border bg-card hover:bg-muted/50 transition-colors">
              <div className="rounded-md bg-primary/10 p-2 mt-1">
                <Shield className="h-5 w-5 text-primary" />
              </div>
              <div className="flex-1">
                <p className="text-sm font-medium text-muted-foreground">Role</p>
                <p className="text-lg font-semibold capitalize">{user.role}</p>
              </div>
            </div>
          </div>

          <div className="pt-4 border-t">
            <p className="text-sm text-muted-foreground text-center">
              Account management features coming soon
            </p>
          </div>
        </CardContent>
      </Card>
    </div>
  );
}

export default ProfilePage;

