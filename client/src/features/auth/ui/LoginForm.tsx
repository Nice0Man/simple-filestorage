import { useForm } from 'react-hook-form';
import { zodResolver } from '@hookform/resolvers/zod';
import { useNavigate } from 'react-router-dom';
import { useTranslation } from 'react-i18next';
import { loginSchema, type LoginFormData } from '../../../shared/lib/validation';
import { sanitizeText } from '../../../shared/lib/sanitize';
import { Input } from '../../../shared/ui/Input';
import { Button } from '../../../shared/ui/Button';
import { useAuthStore } from '../../../shared/model/auth.store';
import { toast } from '../../../shared/model/toast.store';
import { authApi } from '../../../shared/api/auth.api';
import { ROUTES } from '../../../shared/config/routes.config';

export function LoginForm() {
    const { t } = useTranslation();
    const navigate = useNavigate();
    const { setUser, setToken } = useAuthStore();

    const {
        register,
        handleSubmit,
        formState: { errors, isSubmitting },
    } = useForm<LoginFormData>({
        resolver: zodResolver(loginSchema),
    });

    const onSubmit = async (data: LoginFormData) => {
        try {
            // Sanitize inputs before sending
            const sanitizedData = {
                username: sanitizeText(data.username),
                password: data.password, // Don't sanitize password as it may contain special chars
            };

            const response = await authApi.login(sanitizedData);
            setUser(response.user);
            setToken(response.token);
            toast.success(t('auth.loginSuccess'), `Welcome, ${response.user.username}!`);
            navigate(ROUTES.FILES);
        } catch (error) {
            const errorMessage = error instanceof Error ? error.message : t('auth.loginError');
            toast.error(t('auth.loginError'), errorMessage);
        }
    };

    return (
        <form onSubmit={handleSubmit(onSubmit)} className="space-y-4">
            <div className="space-y-2">
                <label htmlFor="username" className="text-sm font-medium">
                    {t('auth.username')}
                </label>
                <Input
                    id="username"
                    type="text"
                    {...register('username')}
                    disabled={isSubmitting}
                    className={errors.username ? 'border-destructive' : ''}
                />
                {errors.username && (
                    <p className="text-sm text-destructive">{errors.username.message}</p>
                )}
            </div>

            <div className="space-y-2">
                <label htmlFor="password" className="text-sm font-medium">
                    {t('auth.password')}
                </label>
                <Input
                    id="password"
                    type="password"
                    {...register('password')}
                    disabled={isSubmitting}
                    className={errors.password ? 'border-destructive' : ''}
                />
                {errors.password && (
                    <p className="text-sm text-destructive">{errors.password.message}</p>
                )}
            </div>

            <Button type="submit" className="w-full" disabled={isSubmitting}>
                {isSubmitting ? t('common.loading') : t('auth.loginButton')}
            </Button>
        </form>
    );
}

