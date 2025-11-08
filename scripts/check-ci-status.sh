#!/bin/bash

# Script to check CI status for a branch
# Usage: ./check-ci-status.sh [branch_name]

BRANCH="${1:-dev}"
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}   CI Status Check for Branch: ${CYAN}${BRANCH}${NC}"
echo -e "${BLUE}================================================${NC}"
echo ""

# Check if branch exists
if ! git rev-parse --verify "$BRANCH" &>/dev/null; then
    echo -e "${RED}✗ Branch '$BRANCH' does not exist locally${NC}"
    echo ""
    echo "Available branches:"
    git branch -a
    exit 1
fi

# Get branch info
echo -e "${CYAN}Branch Information:${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

CURRENT_BRANCH=$(git branch --show-current)
if [ "$CURRENT_BRANCH" = "$BRANCH" ]; then
    echo -e "Current branch: ${GREEN}$BRANCH ✓${NC}"
else
    echo -e "Current branch: $CURRENT_BRANCH"
    echo -e "Checking branch: $BRANCH"
fi

# Get last commit info
LAST_COMMIT=$(git log -1 --format="%H" "$BRANCH" 2>/dev/null)
COMMIT_MSG=$(git log -1 --format="%s" "$BRANCH" 2>/dev/null)
COMMIT_AUTHOR=$(git log -1 --format="%an" "$BRANCH" 2>/dev/null)
COMMIT_DATE=$(git log -1 --format="%ar" "$BRANCH" 2>/dev/null)

echo "Last commit: ${LAST_COMMIT:0:8}"
echo "Message: $COMMIT_MSG"
echo "Author: $COMMIT_AUTHOR"
echo "Date: $COMMIT_DATE"
echo ""

# Check if branch is pushed to remote
echo -e "${CYAN}Remote Status:${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if git ls-remote --heads origin "$BRANCH" | grep -q "$BRANCH"; then
    echo -e "${GREEN}✓${NC} Branch exists on remote (origin/$BRANCH)"
    
    # Check if local is up to date with remote
    git fetch origin "$BRANCH" --quiet 2>/dev/null
    LOCAL=$(git rev-parse "$BRANCH" 2>/dev/null)
    REMOTE=$(git rev-parse "origin/$BRANCH" 2>/dev/null)
    
    if [ "$LOCAL" = "$REMOTE" ]; then
        echo -e "${GREEN}✓${NC} Local branch is up to date with remote"
    else
        echo -e "${YELLOW}⚠${NC} Local and remote branches have diverged"
        AHEAD=$(git rev-list --count "origin/$BRANCH..$BRANCH" 2>/dev/null)
        BEHIND=$(git rev-list --count "$BRANCH..origin/$BRANCH" 2>/dev/null)
        echo "  Ahead: $AHEAD commits"
        echo "  Behind: $BEHIND commits"
    fi
else
    echo -e "${YELLOW}⚠${NC} Branch not pushed to remote yet"
    echo "  Run: git push origin $BRANCH"
fi
echo ""

# Check GitHub Actions workflows
echo -e "${CYAN}GitHub Actions Workflows:${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ -d ".github/workflows" ]; then
    WORKFLOW_COUNT=$(ls -1 .github/workflows/*.yml 2>/dev/null | wc -l)
    echo -e "${GREEN}✓${NC} Found $WORKFLOW_COUNT workflow(s)"
    
    for workflow in .github/workflows/*.yml; do
        if [ -f "$workflow" ]; then
            WORKFLOW_NAME=$(basename "$workflow")
            echo "  - $WORKFLOW_NAME"
        fi
    done
else
    echo -e "${RED}✗${NC} No workflows found in .github/workflows/"
fi
echo ""

# Try to get CI status from GitHub API (requires gh CLI or GitHub token)
echo -e "${CYAN}CI Status (GitHub Actions):${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Check if gh CLI is available
if command -v gh &> /dev/null; then
    echo "Fetching status from GitHub..."
    
    if gh run list --branch "$BRANCH" --limit 5 2>/dev/null; then
        echo ""
        echo -e "${GREEN}✓${NC} Successfully fetched CI status"
    else
        echo -e "${YELLOW}⚠${NC} Could not fetch status (may need authentication)"
        echo "  Run: gh auth login"
    fi
else
    echo -e "${YELLOW}⚠${NC} GitHub CLI (gh) not installed"
    echo ""
    echo "To check CI status:"
    echo "  1. Install gh CLI: https://cli.github.com/"
    echo "  2. Or visit GitHub Actions page manually"
fi
echo ""

# Get repository URL
echo -e "${CYAN}Quick Links:${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

REPO_URL=$(git config --get remote.origin.url 2>/dev/null)
if [ -n "$REPO_URL" ]; then
    # Convert SSH to HTTPS
    REPO_URL=$(echo "$REPO_URL" | sed 's/git@github.com:/https:\/\/github.com\//' | sed 's/\.git$//')
    
    echo "Repository: $REPO_URL"
    echo ""
    echo "Check CI status at:"
    echo "  ${BLUE}$REPO_URL/actions${NC}"
    echo ""
    echo "Branch commits:"
    echo "  ${BLUE}$REPO_URL/commits/$BRANCH${NC}"
    echo ""
    echo "Compare branches:"
    echo "  ${BLUE}$REPO_URL/compare/master...$BRANCH${NC}"
else
    echo -e "${YELLOW}⚠${NC} No remote origin configured"
fi
echo ""

# Local validation
echo -e "${CYAN}Local Validation:${NC}"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Check if workflows are valid
echo "Validating workflow files..."
INVALID_COUNT=0

for workflow in .github/workflows/*.yml; do
    if [ -f "$workflow" ]; then
        if python3 -c "import yaml; yaml.safe_load(open('$workflow'))" 2>/dev/null; then
            echo -e "${GREEN}✓${NC} $(basename $workflow)"
        else
            echo -e "${RED}✗${NC} $(basename $workflow) - Invalid YAML"
            ((INVALID_COUNT++))
        fi
    fi
done

if [ $INVALID_COUNT -eq 0 ]; then
    echo -e "\n${GREEN}✓ All workflow files are valid${NC}"
else
    echo -e "\n${RED}✗ Found $INVALID_COUNT invalid workflow file(s)${NC}"
fi

echo ""
echo -e "${BLUE}================================================${NC}"
echo -e "${GREEN}Done!${NC}"
echo -e "${BLUE}================================================${NC}"

