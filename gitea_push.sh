#!/bin/bash
# Gitea Push Script
# This script pushes all repositories to Gitea using SSH

cd /projects

# Function to push a single repository
push_repo() {
    local repo_path="$1"
    local repo_name="$2"
    
    echo "Pushing $repo_name from $repo_path"
    
    cd "$repo_path" || return 1
    
    # Check if gitea remote exists
    if ! git remote | grep -q gitea; then
        echo "  No gitea remote found, skipping"
        return 1
    fi
    
    # Get current branch
    current_branch=$(git branch --show-current)
    if [ -z "$current_branch" ]; then
        echo "  No active branch, skipping"
        return 1
    fi
    
    # Push to gitea
    echo "  Pushing branch: $current_branch"
    echo "  Gitea remote URL: $(git remote get-url gitea)"
    if git push gitea "$current_branch:$current_branch"; then
        echo "  ✓ Successfully pushed $current_branch"
        return 0
    else
        echo "  ✗ Failed to push $current_branch"
        return 1
    fi
}

# Find all repositories with gitea remotes
echo "Finding repositories with gitea remotes..."
repos_with_gitea=()

for repo in $(find /projects -name ".git" -type d | head -20); do
    repo_path=$(dirname "$repo")
    repo_name=$(basename "$repo_path")
    
    # Check if this repo has a gitea remote
    if [ -d "$repo_path" ] && git -C "$repo_path" remote | grep -q gitea; then
        repos_with_gitea+=("$repo_path:$repo_name")
    fi
done

echo "Found ${#repos_with_gitea[@]} repositories with gitea remotes"

# Push each repository
success_count=0
total_count=${#repos_with_gitea[@]}

for repo_info in "${repos_with_gitea[@]}"; do
    IFS=':' read -r repo_path repo_name <<< "$repo_info"
    echo ""
    echo "Processing: $repo_name"
    
    if push_repo "$repo_path" "$repo_name"; then
        ((success_count++))
    fi
done

echo ""
echo "=== Push Summary ==="
echo "Total repositories: $total_count"
echo "Successfully pushed: $success_count"
echo "Failed: $((total_count - success_count))"
echo "Timestamp: $(date)"
