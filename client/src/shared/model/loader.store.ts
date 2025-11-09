import { create } from 'zustand';

interface LoaderState {
  isLoading: boolean;
  loadingCount: number;
  startLoading: () => void;
  stopLoading: () => void;
}

export const useLoaderStore = create<LoaderState>((set) => ({
  isLoading: false,
  loadingCount: 0,

  startLoading: () =>
    set((state) => ({
      loadingCount: state.loadingCount + 1,
      isLoading: true,
    })),

  stopLoading: () =>
    set((state) => {
      const newCount = Math.max(0, state.loadingCount - 1);
      return {
        loadingCount: newCount,
        isLoading: newCount > 0,
      };
    }),
}));

