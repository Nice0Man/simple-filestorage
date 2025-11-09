import { useTranslation } from 'react-i18next';
import { Shield, Users, Database, Settings } from 'lucide-react';
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '../../shared/ui/Card';

function AdminPage() {
  const { t } = useTranslation();

  const adminSections = [
    {
      icon: Users,
      title: 'User Management',
      description: 'Manage users, roles and permissions',
      color: 'text-blue-500',
      bgColor: 'bg-blue-500/10',
    },
    {
      icon: Database,
      title: 'Storage Management',
      description: 'Monitor storage usage and file system',
      color: 'text-green-500',
      bgColor: 'bg-green-500/10',
    },
    {
      icon: Settings,
      title: 'System Settings',
      description: 'Configure server and application settings',
      color: 'text-purple-500',
      bgColor: 'bg-purple-500/10',
    },
    {
      icon: Shield,
      title: 'Security',
      description: 'View logs, audit trails and security settings',
      color: 'text-red-500',
      bgColor: 'bg-red-500/10',
    },
  ];

  return (
    <div className="container mx-auto max-w-6xl py-6 px-4 md:py-8">
      <Card className="shadow-lg mb-6">
        <CardHeader className="space-y-1">
          <div className="flex items-center gap-3">
            <div className="rounded-full bg-primary/10 p-3">
              <Shield className="h-8 w-8 text-primary" />
            </div>
            <div>
              <CardTitle className="text-2xl md:text-3xl font-bold">
                {t('admin.title') || 'Admin Panel'}
              </CardTitle>
              <CardDescription>System administration and management</CardDescription>
            </div>
          </div>
        </CardHeader>
      </Card>

      <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
        {adminSections.map((section) => {
          const Icon = section.icon;
          return (
            <Card
              key={section.title}
              className="shadow-md hover:shadow-lg transition-all duration-200 cursor-pointer hover:border-primary/50"
            >
              <CardContent className="p-6">
                <div className="flex items-start gap-4">
                  <div className={`rounded-lg ${section.bgColor} p-3`}>
                    <Icon className={`h-6 w-6 ${section.color}`} />
                  </div>
                  <div className="flex-1">
                    <h3 className="text-lg font-semibold mb-1">{section.title}</h3>
                    <p className="text-sm text-muted-foreground">{section.description}</p>
                  </div>
                </div>
              </CardContent>
            </Card>
          );
        })}
      </div>

      <Card className="shadow-lg mt-6">
        <CardContent className="p-6">
          <div className="flex items-center justify-center gap-2 text-muted-foreground">
            <Settings className="h-5 w-5" />
            <p className="text-sm">Admin features are under development</p>
          </div>
        </CardContent>
      </Card>
    </div>
  );
}

export default AdminPage;

