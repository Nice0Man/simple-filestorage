# Feature-Sliced Design Structure

Complete project structure for the File Storage client application.

## Current Directory Tree

```
client/
├── public/                         # Static assets
├── src/
│   ├── app/                       # Application layer
│   │   ├── providers/
│   │   │   ├── AppProvider.tsx    # Main app provider (Router, Toast, ErrorBoundary)
│   │   │   └── ErrorBoundary.tsx  # Error boundary for error handling
│   │   ├── styles/
│   │   │   └── index.css          # Global styles with Tailwind
│   │   └── App.tsx                # Root component with routing & lazy loading
│   │
│   ├── pages/                     # Page components (routes)
│   │   └── ui/
│   │       ├── LoginPage.tsx      # Login page
│   │       └── FilesPage.tsx      # Files management page
│   │
│   ├── features/                  # Features layer (user interactions)
│   │   ├── auth/
│   │   │   └── ui/
│   │   │       └── LoginForm.tsx  # Login form with validation
│   │   └── file-upload/
│   │       └── ui/
│   │           └── FileUploader.tsx # File uploader with validation
│   │
│   ├── entities/                  # Entities layer (business models)
│   │   ├── user/
│   │   │   └── ui/
│   │   │       └── UserAvatar.tsx # User avatar component
│   │   └── file/
│   │       └── ui/
│   │           └── FileItem.tsx   # File item component
│   │
│   ├── widgets/                   # Composite UI blocks
│   │   └── ui/
│   │       └── Header.tsx         # App header with navigation
│   │
│   └── shared/                    # Shared resources
│       ├── api/
│       │   ├── client.ts          # Axios client instance
│       │   ├── auth.api.ts        # Auth API methods
│       │   └── file.api.ts        # File API methods
│       ├── config/
│       │   ├── api.config.ts      # API configuration
│       │   ├── routes.config.ts   # Routes constants
│       │   └── i18n.config.ts     # i18n configuration
│       ├── lib/
│       │   ├── cn.ts              # className utility
│       │   ├── format.ts          # Formatting helpers
│       │   ├── validation.ts      # Zod validation schemas
│       │   └── sanitize.ts        # Input sanitization functions
│       ├── model/
│       │   ├── auth.store.ts      # Global auth store
│       │   ├── theme.store.ts     # Theme store
│       │   ├── file.store.ts      # Files store
│       │   ├── toast.store.ts     # Toast notifications store
│       │   └── loader.store.ts    # Global loader store
│       ├── types/
│       │   ├── auth.types.ts      # Auth types
│       │   └── file.types.ts      # File types
│       ├── ui/
│       │   ├── Button.tsx         # Button component
│       │   ├── Card.tsx           # Card component
│       │   ├── Input.tsx          # Input component
│       │   ├── ThemeToggle.tsx    # Theme switcher
│       │   ├── Toast.tsx          # Toast notification component
│       │   ├── ConfirmDialog.tsx  # Confirmation dialog
│       │   ├── Skeleton.tsx       # Skeleton loaders
│       │   └── GlobalLoader.tsx   # Global loading indicator
│       └── locales/
│           ├── en.json            # English translations
│           └── ru.json            # Russian translations
│
├── index.html                     # HTML entry point
├── package.json                   # Dependencies
├── tsconfig.json                  # TypeScript config
├── tailwind.config.js             # Tailwind configuration
├── postcss.config.js              # PostCSS configuration
├── vite.config.ts                 # Vite configuration
├── Dockerfile                     # Docker image
├── README.md                      # Documentation
└── FSD-STRUCTURE.md               # This file
```

## Layer Descriptions

### 1. App Layer (`src/app/`)
**Purpose**: Application initialization, global providers, routing

**Current Structure**:
- `providers/` - Root providers (Router, etc.)
- `styles/` - Global CSS with Tailwind
- `App.tsx` - Main app component with routes

**Rules**:
- Can import from all layers
- No business logic
- Only app-wide concerns

### 2. Pages Layer (`src/pages/`)
**Purpose**: Route components, page layouts

**Current Structure**:
- `ui/` - Page components (LoginPage, FilesPage)

**Rules**:
- Can use Widgets, Features, Entities, Shared
- No business logic
- Only composition

### 3. Widgets Layer (`src/widgets/`)
**Purpose**: Large composite UI blocks

**Current Structure**:
- `ui/` - Widget components (Header)

**Rules**:
- Can use Features, Entities, Shared
- Should be reusable
- Can have own state

### 4. Shared Layer (`src/shared/`)
**Purpose**: Reusable code without business logic

**Current Segments**:
- `api/` - HTTP client and API methods
- `config/` - Configuration files
- `lib/` - Utility functions
- `model/` - Global stores (Zustand)
- `types/` - TypeScript definitions
- `ui/` - Reusable UI components
- `locales/` - i18n translations

**Rules**:
- No business logic
- No imports from other layers
- Maximum reusability

## Segments

Each layer contains only the segments it needs:

- **ui/** - React components
- **model/** - State management (Zustand stores)
- **api/** - API integration
- **lib/** - Helper functions
- **types/** - TypeScript types
- **config/** - Configuration
- **styles/** - CSS files
- **providers/** - React context providers
- **locales/** - i18n translations

## Import Rules

```
app → pages, widgets, shared
pages → widgets, shared
widgets → shared
shared → nothing (fully isolated)
```

## Naming Conventions

### Files
- Components: `PascalCase.tsx` (Button.tsx, UserCard.tsx)
- Stores: `camelCase.store.ts` (auth.store.ts)
- Types: `camelCase.types.ts` (user.types.ts)
- API: `camelCase.api.ts` (auth.api.ts)
- Utils: `camelCase.ts` (format.ts)
- Config: `camelCase.config.ts` (api.config.ts)

### Components
```tsx
// Preferred: Named export
export function Button() {}

// Also acceptable: Const export
export const Button = () => {}
```

### Imports
Use absolute imports with path aliases:

```tsx
// vite.config.ts
{
  resolve: {
    alias: {
      '@': path.resolve(__dirname, './src'),
    }
  }
}

// Usage
import { Button } from '@/shared/ui/Button';
import { useAuthStore } from '@/shared/model/auth.store';
```

## Adding New Features

### Example: Add User Profile Feature

1. **Create feature pages** (if new route needed):
```
pages/ui/ProfilePage.tsx
```

2. **Add to shared if needed**:
```
shared/api/user.api.ts
shared/types/user.types.ts
```

3. **Create UI components**:
```
shared/ui/UserAvatar.tsx
```

4. **Update routing** in `app/App.tsx`

### Example: Add File Upload Widget

1. **Create widget**:
```
widgets/ui/FileUpload.tsx
```

2. **Use in page**:
```tsx
import { FileUpload } from '@/widgets/ui/FileUpload';

<FileUpload onUpload={handleUpload} />
```

## When to Create New Layers

### Features Layer (`src/features/`)
Create when you have reusable user interactions:
- LoginForm (separate from LoginPage)
- FileUploader (if complex)
- SearchBar with filters

Structure:
```
features/
  ├── auth/
  │   └── ui/
  │       └── LoginForm.tsx
  └── file-upload/
      └── ui/
          └── UploadZone.tsx
```

### Entities Layer (`src/entities/`)
Create when you have business entities with their own logic:
- User entity with store and UI
- File entity with model and components

Structure:
```
entities/
  ├── user/
  │   ├── model/
  │   │   └── user.store.ts
  │   ├── types/
  │   │   └── user.types.ts
  │   └── ui/
  │       └── UserCard.tsx
  └── file/
      └── ui/
          └── FileItem.tsx
```

### Processes Layer (`src/processes/`)
Create when you have complex multi-step flows:
- Multi-step file upload with preview
- Wizard-style forms

## Best Practices

1. **Keep it minimal**: Only create folders when needed
2. **One responsibility**: Each component has one clear purpose
3. **Compose up**: Lower layers compose into higher layers
4. **No cross-imports**: Never import between features or widgets
5. **Shared layer**: Keep it business-logic free
6. **Type safety**: Always define TypeScript types
7. **Testing**: Test each layer independently

## Current Tech Stack

- **React 19** - UI library with Suspense & lazy loading
- **TypeScript** - Type safety
- **Vite** - Build tool with code splitting
- **Tailwind CSS** - Styling
- **React Router 7** - Routing
- **Zustand** - State management with persist
- **i18next** - Internationalization
- **Axios** - HTTP client
- **Lucide React** - Icons
- **React Hook Form** - Form validation
- **Zod** - Schema validation
- **DOMPurify** - Input sanitization

## Architecture Features

✅ **Error Handling**: Error Boundary + Toast notifications
✅ **Loading States**: Skeleton loaders + Global loader
✅ **Form Validation**: React Hook Form + Zod schemas
✅ **Code Splitting**: React.lazy for route-based splitting
✅ **Optimization**: React.memo for expensive components
✅ **Security**: Input sanitization + XSS protection
✅ **FSD Architecture**: Features + Entities layers
✅ **Type Safety**: Full TypeScript coverage

## Migration Path

When the project grows:

1. **Small project** (current):
   - app/, pages/, widgets/, shared/
   
2. **Medium project**:
   - Add features/ for reusable interactions
   - Move LoginForm from LoginPage to features/auth/
   
3. **Large project**:
   - Add entities/ for business models
   - Add processes/ for complex flows
   - Split features into smaller slices

## Directory Commands

```bash
# View current structure
tree -d -L 3 src/

# Find all TypeScript files
find src -name "*.tsx" -o -name "*.ts"

# Count lines of code by layer
find src/app -name "*.tsx" | xargs wc -l
find src/pages -name "*.tsx" | xargs wc -l
find src/shared -name "*.tsx" | xargs wc -l
```

## Resources

- [Feature-Sliced Design](https://feature-sliced.design/)
- [FSD Documentation](https://feature-sliced.design/docs)
- [Best Practices](https://feature-sliced.design/docs/guides/examples)
- [React Best Practices](https://react.dev/learn)
