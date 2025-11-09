import { Component, type ReactNode } from 'react';
import { Button } from '../../shared/ui/Button';
import { Card, CardContent, CardDescription, CardHeader, CardTitle } from '../../shared/ui/Card';

interface Props {
    children: ReactNode;
    fallback?: ReactNode;
}

interface State {
    hasError: boolean;
    error: Error | null;
}

export class ErrorBoundary extends Component<Props, State> {
    constructor(props: Props) {
        super(props);
        this.state = { hasError: false, error: null };
    }

    static getDerivedStateFromError(error: Error): State {
        return { hasError: true, error };
    }

    componentDidCatch(error: Error, errorInfo: React.ErrorInfo) {
        console.error('Error caught by boundary:', error, errorInfo);

        // Here you can send error to logging service
        // Example: logErrorToService(error, errorInfo);
    }

    handleReset = () => {
        this.setState({ hasError: false, error: null });
        window.location.href = '/';
    };

    render() {
        if (this.state.hasError) {
            if (this.props.fallback) {
                return this.props.fallback;
            }

            return (
                <div className="flex min-h-screen items-center justify-center bg-background p-4">
                    <Card className="w-full max-w-lg">
                        <CardHeader>
                            <CardTitle className="text-destructive">Something went wrong</CardTitle>
                            <CardDescription>
                                An unexpected error occurred. Please try refreshing the page.
                            </CardDescription>
                        </CardHeader>
                        <CardContent className="space-y-4">
                            {this.state.error && (
                                <details className="rounded-lg bg-muted p-4 text-sm">
                                    <summary className="cursor-pointer font-medium">Error details</summary>
                                    <pre className="mt-2 overflow-auto text-xs">
                                        {this.state.error.message}
                                        {'\n\n'}
                                        {this.state.error.stack}
                                    </pre>
                                </details>
                            )}
                            <div className="flex space-x-2">
                                <Button onClick={this.handleReset} className="flex-1">
                                    Go to Home
                                </Button>
                                <Button
                                    variant="outline"
                                    onClick={() => window.location.reload()}
                                    className="flex-1"
                                >
                                    Reload Page
                                </Button>
                            </div>
                        </CardContent>
                    </Card>
                </div>
            );
        }

        return this.props.children;
    }
}

