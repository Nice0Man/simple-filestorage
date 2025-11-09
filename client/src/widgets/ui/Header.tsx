import { memo } from 'react';
import { Link } from 'react-router-dom';
import { useTranslation } from 'react-i18next';
import { FileText, LogOut, User } from 'lucide-react';
import { Button } from '../../shared/ui/Button';
import { ThemeToggle } from '../../shared/ui/ThemeToggle';
import { useAuth } from '../../shared/context/AuthContext';
import { ROUTES } from '../../shared/config/routes.config';

export const Header = memo(function Header() {
  const { t } = useTranslation();
  const { user, logout } = useAuth();

  return (
    <header className="sticky top-0 z-50 w-full border-b bg-background/95 backdrop-blur supports-[backdrop-filter]:bg-background/60 shadow-sm">
      <div className="container mx-auto flex h-16 items-center justify-between px-4 md:px-6">
        <Link to={ROUTES.HOME} className="flex items-center space-x-2 hover:opacity-80 transition-opacity">
          <FileText className="h-6 w-6 text-primary" />
          <span className="font-bold text-xl hidden sm:inline">FileStorage</span>
        </Link>

        <nav className="flex items-center gap-3 md:gap-6">
          {user && (
            <>
              <Link 
                to={ROUTES.FILES} 
                className="text-sm font-medium transition-colors hover:text-primary hidden sm:inline-block"
              >
                {t('navigation.files')}
              </Link>
              {user.role === 'admin' && (
                <Link 
                  to={ROUTES.ADMIN} 
                  className="text-sm font-medium transition-colors hover:text-primary hidden md:inline-block"
                >
                  {t('navigation.admin')}
                </Link>
              )}
              <Link 
                to={ROUTES.PROFILE} 
                className="text-sm font-medium transition-colors hover:text-primary p-2 rounded-md hover:bg-muted"
                title="Profile"
              >
                <User className="h-5 w-5" />
              </Link>
              <Button variant="ghost" size="sm" onClick={logout} className="hidden sm:flex">
                <LogOut className="h-4 w-4 mr-2" />
                {t('auth.logout')}
              </Button>
              <Button variant="ghost" size="sm" onClick={logout} className="sm:hidden p-2">
                <LogOut className="h-4 w-4" />
              </Button>
            </>
          )}
          <ThemeToggle />
        </nav>
      </div>
    </header>
  );
});

