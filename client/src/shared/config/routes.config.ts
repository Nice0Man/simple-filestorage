export const ROUTES = {
  HOME: '/',
  LOGIN: '/login',
  FILES: '/files',
  ADMIN: '/admin',
  ADMIN_USERS: '/admin/users',
  PROFILE: '/profile',
  NOT_FOUND: '/404',
} as const;

export type RouteKey = keyof typeof ROUTES;
export type RoutePath = typeof ROUTES[RouteKey];

