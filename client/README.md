# File Storage Client

Modern React-based client application for File Storage server.

## Tech Stack

- **React 19** - UI library
- **TypeScript** - Type safety
- **Vite** - Build tool and dev server
- **Tailwind CSS** - Styling with dark/light mode support
- **React Router** - Client-side routing
- **Zustand** - State management
- **i18next** - Internationalization (EN/RU)
- **Axios** - HTTP client
- **Lucide React** - Icon library

## Architecture

The project follows **Feature-Sliced Design (FSD)** architecture:

```
src/
├── app/                    # Application layer
│   ├── providers/          # App providers (Router, etc.)
│   ├── styles/            # Global styles
│   └── App.tsx            # Main app component
├── pages/                  # Pages (routes)
│   └── ui/                # Page components
├── widgets/                # Complex UI blocks
│   └── ui/                # Widget components
├── features/               # User features
│   └── ui/                # Feature components
├── entities/               # Business entities
│   ├── model/             # Entity stores
│   └── ui/                # Entity components
└── shared/                 # Shared resources
    ├── api/               # API clients
    ├── config/            # Configuration
    ├── lib/               # Utilities
    ├── model/             # Shared stores
    ├── types/             # TypeScript types
    ├── ui/                # UI components
    └── locales/           # Translations
```

## Features

- User authentication with JWT
- File upload/download/delete operations
- Dark/Light theme toggle
- Internationalization (EN/RU)
- Responsive design
- Type-safe API client
- Persistent state management

## Getting Started

### Prerequisites

- Node.js 20+ 
- npm 10+

### Installation

```bash
# Install dependencies
npm install

# Copy environment variables
cp .env.example .env

# Start development server
npm run dev
```

### Environment Variables

Create `.env` file:

```env
VITE_API_BASE_URL=http://localhost:8080/api/v1
```

### Development

```bash
# Start dev server (http://localhost:5173)
npm run dev

# Build for production
npm run build

# Preview production build
npm run preview

# Lint code
npm run lint
```

## Project Structure

### Layers (FSD)

1. **app** - Application initialization and providers
2. **processes** - Complex user flows (optional)
3. **pages** - Route components
4. **widgets** - Composite UI blocks (Header, Sidebar, etc.)
5. **features** - User interactions (Login, FileUpload, etc.)
6. **entities** - Business logic (User, File, etc.)
7. **shared** - Reusable code (UI kit, utils, API, etc.)

### Segments

Each layer can have:
- `ui/` - React components
- `model/` - State management (Zustand stores)
- `api/` - API integration
- `lib/` - Helper functions
- `types/` - TypeScript definitions
- `config/` - Configuration
- `styles/` - CSS modules

## Design System

### Colors

Theme variables (CSS custom properties):
- Light and Dark mode variants
- Primary, Secondary, Accent colors
- Semantic colors (success, error, warning)

### Components

Reusable UI components in `shared/ui/`:
- Button - Various variants and sizes
- Card - Content containers
- Input - Form inputs
- ThemeToggle - Dark/Light mode switcher

### Theming

Theme is managed via Zustand store with localStorage persistence.
Supports: `light`, `dark`, `system`

```tsx
import { useThemeStore } from '@/shared/model/theme.store';

const { theme, setTheme, toggleTheme } = useThemeStore();
```

## State Management

Uses Zustand for state management:

### Auth Store

```tsx
import { useAuthStore } from '@/shared/model/auth.store';

const { user, isAuthenticated, setUser, logout } = useAuthStore();
```

### File Store

```tsx
import { useFileStore } from '@/shared/model/file.store';

const { files, setFiles, addFile, removeFile } = useFileStore();
```

## Internationalization

Supported languages: English, Russian

```tsx
import { useTranslation } from 'react-i18next';

const { t, i18n } = useTranslation();

// Usage
{t('auth.login')}
{t('files.upload')}

// Change language
i18n.changeLanguage('ru');
```

## API Integration

API client is configured in `shared/api/`:

```tsx
import { authApi } from '@/shared/api/auth.api';
import { fileApi } from '@/shared/api/file.api';

// Login
await authApi.login({ username, password });

// Upload file
await fileApi.upload(file);
```

## Routing

Routes are defined in `shared/config/routes.config.ts`:

```tsx
import { ROUTES } from '@/shared/config/routes.config';

<Link to={ROUTES.FILES}>Files</Link>
```

## Type Safety

TypeScript types are defined in `shared/types/`:

- `auth.types.ts` - Authentication types
- `file.types.ts` - File operation types

## Building for Production

```bash
npm run build
```

Output: `dist/` directory

Serve with any static file server:

```bash
# Using serve
npx serve -s dist

# Using http-server
npx http-server dist
```

## Code Style

- Use TypeScript strict mode
- Follow FSD architecture principles
- Component names in PascalCase
- Utility functions in camelCase
- Use functional components with hooks
- Prefer named exports

## Contributing

1. Follow FSD architecture
2. Add types for new entities
3. Update i18n translations
4. Test in both light/dark themes
5. Ensure responsive design

## License

MIT
