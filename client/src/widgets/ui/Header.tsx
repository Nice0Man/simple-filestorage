import { memo } from 'react';
import { Link } from 'react-router-dom';
import { useTranslation } from 'react-i18next';
import { FileText, LogOut, User } from 'lucide-react';
import { Button } from '../../shared/ui/Button';
import { ThemeToggle } from '../../shared/ui/ThemeToggle';
import { useAuthStore } from '../../shared/model/auth.store';
import { ROUTES } from '../../shared/config/routes.config';

export const Header = memo(function Header() {
  const { t } = useTranslation();
  const { user, logout } = useAuthStore();

  return (
    <header className="sticky top-0 z-50 w-full border-b bg-background/95 backdrop-blur supports-[backdrop-filter]:bg-background/60">
      <div className="container flex h-16 items-center justify-between">
        <Link to={ROUTES.HOME} className="flex items-center space-x-2">
          <FileText className="h-6 w-6" />
          <span className="font-bold text-xl">FileStorage</span>
        </Link>

        <nav className="flex items-center space-x-6">
          {user && (
            <>
              <Link to={ROUTES.FILES} className="text-sm font-medium transition-colors hover:text-primary">
                {t('navigation.files')}
              </Link>
              {user.role === 'admin' && (
                <Link to={ROUTES.ADMIN} className="text-sm font-medium transition-colors hover:text-primary">
                  {t('navigation.admin')}
                </Link>
              )}
              <Link to={ROUTES.PROFILE} className="text-sm font-medium transition-colors hover:text-primary">
                <User className="h-5 w-5" />
              </Link>
              <Button variant="ghost" size="sm" onClick={logout}>
                <LogOut className="h-4 w-4 mr-2" />
                {t('auth.logout')}
              </Button>
            </>
          )}
          <ThemeToggle />
        </nav>
      </div>
    </header>
  );
});

