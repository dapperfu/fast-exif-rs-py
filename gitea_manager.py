#!/usr/bin/env python3
"""
Gitea Repository Manager

This script manages Gitea repositories to mirror GitHub repositories.
It creates corresponding repositories on the Gitea server and adds them as remotes.

Author: Claude Sonnet 4 (claude-3-5-sonnet-20241022)
Generated via Cursor IDE (cursor.sh) with AI assistance
Model: Anthropic Claude 3.5 Sonnet
Generation timestamp: 2025-01-27
Context: Automated Gitea repository management for GitHub mirroring

Technical details:
- LLM: Claude 3.5 Sonnet (2024-10-22)
- IDE: Cursor (cursor.sh)
- Generation method: AI-assisted pair programming
- Code style: Python with full mypy typing and numpy docstring style
- Dependencies: requests, pygit2, GitPython
"""

import os
import sys
import json
import logging
import argparse
from pathlib import Path
from typing import Dict, List, Optional, Tuple
from urllib.parse import urlparse

import requests
import git
from git import Repo, Remote


class GiteaManager:
    """
    Manager class for Gitea repository operations.
    
    This class handles creating repositories on Gitea server and managing
    git remotes for local repositories.
    """
    
    def __init__(self, gitea_url: str, token: str) -> None:
        """
        Initialize the Gitea manager.
        
        Parameters
        ----------
        gitea_url : str
            The base URL of the Gitea server (e.g., 'http://nas.local:3000')
        token : str
            The authentication token for Gitea API
        """
        self.gitea_url = gitea_url.rstrip('/')
        self.api_url = f"{self.gitea_url}/api/v1"
        self.token = token
        self.session = requests.Session()
        self.session.headers.update({
            'Authorization': f'token {token}',
            'Content-Type': 'application/json'
        })
        
        # Setup logging
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s'
        )
        self.logger = logging.getLogger(__name__)
    
    def test_connection(self) -> bool:
        """
        Test the connection to the Gitea server.
        
        Returns
        -------
        bool
            True if connection is successful, False otherwise
        """
        try:
            response = self.session.get(f"{self.api_url}/user")
            if response.status_code == 200:
                user_info = response.json()
                self.logger.info(f"Connected to Gitea as user: {user_info.get('login', 'Unknown')}")
                return True
            else:
                self.logger.error(f"Failed to connect to Gitea: {response.status_code}")
                return False
        except Exception as e:
            self.logger.error(f"Error testing Gitea connection: {e}")
            return False
    
    def get_user_repos(self) -> List[Dict]:
        """
        Get list of repositories for the authenticated user.
        
        Returns
        -------
        List[Dict]
            List of repository information dictionaries
        """
        try:
            response = self.session.get(f"{self.api_url}/user/repos")
            if response.status_code == 200:
                return response.json()
            else:
                self.logger.error(f"Failed to get user repos: {response.status_code}")
                return []
        except Exception as e:
            self.logger.error(f"Error getting user repos: {e}")
            return []
    
    def create_repo(self, name: str, description: str = "", private: bool = False) -> Optional[Dict]:
        """
        Create a new repository on Gitea.
        
        Parameters
        ----------
        name : str
            Name of the repository
        description : str, optional
            Description of the repository
        private : bool, optional
            Whether the repository should be private
        
        Returns
        -------
        Optional[Dict]
            Repository information if successful, None otherwise
        """
        data = {
            'name': name,
            'description': description,
            'private': private,
            'auto_init': False
        }
        
        try:
            response = self.session.post(f"{self.api_url}/user/repos", json=data)
            if response.status_code == 201:
                repo_info = response.json()
                self.logger.info(f"Created repository: {name}")
                return repo_info
            else:
                self.logger.error(f"Failed to create repository {name}: {response.status_code}")
                if response.text:
                    self.logger.error(f"Error details: {response.text}")
                return None
        except Exception as e:
            self.logger.error(f"Error creating repository {name}: {e}")
            return None
    
    def repo_exists(self, name: str) -> bool:
        """
        Check if a repository exists for the current user.
        
        Parameters
        ----------
        name : str
            Name of the repository to check
        
        Returns
        -------
        bool
            True if repository exists, False otherwise
        """
        repos = self.get_user_repos()
        return any(repo['name'] == name for repo in repos)
    
    def get_repo_url(self, name: str, use_ssh: bool = True) -> str:
        """
        Get the repository URL for cloning.
        
        Parameters
        ----------
        name : str
            Name of the repository
        use_ssh : bool, optional
            Whether to use SSH URL (default: True)
        
        Returns
        -------
        str
            The repository URL
        """
        if use_ssh:
            # Extract hostname from gitea_url and use SSH port 2222
            hostname = self.gitea_url.replace('http://', '').replace('https://', '')
            # Remove any existing port from hostname
            if ':' in hostname:
                hostname = hostname.split(':')[0]
            return f"ssh://git@{hostname}:2222/{self.get_username()}/{name}.git"
        else:
            return f"{self.gitea_url}/{self.get_username()}/{name}.git"
    
    def get_username(self) -> str:
        """
        Get the username of the authenticated user.
        
        Returns
        -------
        str
            The username
        """
        try:
            response = self.session.get(f"{self.api_url}/user")
            if response.status_code == 200:
                return response.json().get('login', 'unknown')
            return 'unknown'
        except Exception:
            return 'unknown'


class GitRepoManager:
    """
    Manager class for local git repository operations.
    """
    
    def __init__(self) -> None:
        """Initialize the git repository manager."""
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s'
        )
        self.logger = logging.getLogger(__name__)
    
    def get_github_remote_url(self, repo_path: str) -> Optional[str]:
        """
        Get the GitHub remote URL from a local repository.
        
        Parameters
        ----------
        repo_path : str
            Path to the local repository
        
        Returns
        -------
        Optional[str]
            GitHub remote URL if found, None otherwise
        """
        try:
            repo = Repo(repo_path)
            
            # Check for 'origin' remote first
            if 'origin' in repo.remotes:
                origin_url = repo.remotes.origin.url
                if 'github.com' in origin_url and 'dapperfu' in origin_url:
                    return origin_url
            
            # Check all remotes for GitHub URLs
            for remote in repo.remotes:
                if 'github.com' in remote.url and 'dapperfu' in remote.url:
                    return remote.url
            
            return None
        except Exception as e:
            self.logger.error(f"Error getting GitHub remote from {repo_path}: {e}")
            return None
    
    def add_gitea_remote(self, repo_path: str, gitea_url: str) -> bool:
        """
        Add or update the gitea remote for a repository.
        
        Parameters
        ----------
        repo_path : str
            Path to the local repository
        gitea_url : str
            URL of the Gitea repository
        
        Returns
        -------
        bool
            True if successful, False otherwise
        """
        try:
            repo = Repo(repo_path)
            
            # Remove existing gitea remote if it exists
            if 'gitea' in repo.remotes:
                repo.delete_remote('gitea')
            
            # Add new gitea remote
            repo.create_remote('gitea', gitea_url)
            self.logger.info(f"Added gitea remote to {repo_path}")
            return True
            
        except Exception as e:
            self.logger.error(f"Error adding gitea remote to {repo_path}: {e}")
            return False
    
    def push_to_gitea(self, repo_path: str, branch: str = 'main') -> bool:
        """
        Push the current branch to the gitea remote.
        
        Parameters
        ----------
        repo_path : str
            Path to the local repository
        branch : str, optional
            Branch to push (default: 'main')
        
        Returns
        -------
        bool
            True if successful, False otherwise
        """
        try:
            repo = Repo(repo_path)
            
            # Check if gitea remote exists
            if 'gitea' not in repo.remotes:
                self.logger.error(f"No gitea remote found in {repo_path}")
                return False
            
            # Check if repository has any commits
            try:
                repo.head.commit
            except Exception:
                self.logger.warning(f"Repository {repo_path} has no commits, skipping push")
                return True
            
            # Get the current branch
            try:
                current_branch = repo.active_branch.name
            except Exception:
                # If no active branch (detached HEAD), try to use the specified branch
                current_branch = branch
            
            # Check if branch exists
            try:
                repo.branches[current_branch]
            except Exception:
                self.logger.warning(f"Branch {current_branch} does not exist in {repo_path}, skipping push")
                return True
            
            # Push to gitea remote
            gitea_remote = repo.remotes.gitea
            try:
                push_info = gitea_remote.push(f"{current_branch}:{current_branch}")
                
                # Check if push was successful
                for info in push_info:
                    if info.flags & info.ERROR:
                        self.logger.error(f"Error pushing {current_branch} to gitea: {info.summary}")
                        return False
                    elif info.flags & info.UP_TO_DATE:
                        self.logger.info(f"Branch {current_branch} already up to date on gitea")
                    elif info.flags & info.FAST_FORWARD:
                        self.logger.info(f"Fast-forwarded {current_branch} on gitea")
                    elif info.flags & info.FORCED_UPDATE:
                        self.logger.info(f"Force-updated {current_branch} on gitea")
                    else:
                        self.logger.info(f"Pushed {current_branch} to gitea")
                
                return True
                
            except Exception as push_error:
                self.logger.error(f"Push operation failed for {repo_path}: {push_error}")
                return False
            
        except Exception as e:
            self.logger.error(f"Error pushing to gitea from {repo_path}: {e}")
            return False
    
    def get_repo_name_from_github_url(self, github_url: str) -> Optional[str]:
        """
        Extract repository name from GitHub URL.
        
        Parameters
        ----------
        github_url : str
            GitHub repository URL
        
        Returns
        -------
        Optional[str]
            Repository name if successful, None otherwise
        """
        try:
            # Handle both https and git URLs
            if github_url.startswith('git@'):
                # git@github.com:user/repo.git
                parts = github_url.split(':')[-1].rstrip('.git')
                return parts.split('/')[-1]
            else:
                # https://github.com/user/repo.git
                parsed = urlparse(github_url)
                path_parts = parsed.path.strip('/').split('/')
                if len(path_parts) >= 2:
                    return path_parts[-1].rstrip('.git')
            return None
        except Exception as e:
            self.logger.error(f"Error extracting repo name from {github_url}: {e}")
            return None


def find_git_repositories(base_path: str) -> List[str]:
    """
    Find all git repositories in the given path.
    
    Parameters
    ----------
    base_path : str
        Base path to search for git repositories
    
    Returns
    -------
    List[str]
        List of paths to git repositories
    """
    git_repos = []
    
    for root, dirs, files in os.walk(base_path):
        if '.git' in dirs:
            git_repos.append(root)
            # Don't search inside .git directories
            dirs.remove('.git')
    
    return git_repos


def main() -> None:
    """Main function to orchestrate the Gitea repository management."""
    parser = argparse.ArgumentParser(description='Manage Gitea repositories for GitHub mirroring')
    parser.add_argument('--push', action='store_true', 
                       help='Push all repositories to Gitea after setup')
    parser.add_argument('--sync-only', action='store_true',
                       help='Only sync existing repositories (skip creation)')
    parser.add_argument('--base-path', default='/projects',
                       help='Base path to search for git repositories (default: /projects)')
    parser.add_argument('--gitea-url', default='http://nas.local:3000',
                       help='Gitea server URL (default: http://nas.local:3000)')
    parser.add_argument('--token-file', default='/projects/gitea.token',
                       help='Path to Gitea token file (default: /projects/gitea.token)')
    
    args = parser.parse_args()
    
    # Configuration
    gitea_url = args.gitea_url
    token_file = args.token_file
    base_path = args.base_path
    
    # Read Gitea token
    try:
        with open(token_file, 'r') as f:
            token = f.read().strip()
    except FileNotFoundError:
        print(f"Error: Token file {token_file} not found")
        sys.exit(1)
    except Exception as e:
        print(f"Error reading token file: {e}")
        sys.exit(1)
    
    # Initialize managers
    gitea_manager = GiteaManager(gitea_url, token)
    git_manager = GitRepoManager()
    
    # Test connection
    if not gitea_manager.test_connection():
        print("Failed to connect to Gitea server")
        sys.exit(1)
    
    # Find all git repositories
    print(f"Searching for git repositories in {base_path}...")
    git_repos = find_git_repositories(base_path)
    print(f"Found {len(git_repos)} git repositories")
    
    # Process each repository
    processed_count = 0
    created_count = 0
    pushed_count = 0
    
    for repo_path in git_repos:
        print(f"\nProcessing: {repo_path}")
        
        # Get GitHub remote URL
        github_url = git_manager.get_github_remote_url(repo_path)
        if not github_url:
            print(f"  No GitHub remote found, skipping")
            continue
        
        print(f"  GitHub URL: {github_url}")
        
        # Extract repository name
        repo_name = git_manager.get_repo_name_from_github_url(github_url)
        if not repo_name:
            print(f"  Could not extract repository name, skipping")
            continue
        
        print(f"  Repository name: {repo_name}")
        
        # Check if repository exists on Gitea
        repo_exists = gitea_manager.repo_exists(repo_name)
        
        if not args.sync_only and not repo_exists:
            print(f"  Creating repository on Gitea...")
            repo_info = gitea_manager.create_repo(
                repo_name,
                description=f"Mirror of {github_url}",
                private=False
            )
            if repo_info:
                created_count += 1
                print(f"  ✓ Repository created successfully")
            else:
                print(f"  ✗ Failed to create repository")
                continue
        elif repo_exists:
            print(f"  Repository already exists on Gitea")
        else:
            print(f"  Repository does not exist on Gitea (sync-only mode)")
            continue
        
        # Add gitea remote (use SSH for pushing)
        gitea_repo_url = gitea_manager.get_repo_url(repo_name, use_ssh=True)
        if git_manager.add_gitea_remote(repo_path, gitea_repo_url):
            print(f"  ✓ Added gitea remote: {gitea_repo_url}")
            processed_count += 1
            
            # Push to Gitea if requested
            if args.push:
                print(f"  Pushing to Gitea...")
                if git_manager.push_to_gitea(repo_path):
                    pushed_count += 1
                    print(f"  ✓ Pushed successfully")
                else:
                    print(f"  ✗ Failed to push")
        else:
            print(f"  ✗ Failed to add gitea remote")
    
    print(f"\n=== Summary ===")
    print(f"Total repositories processed: {processed_count}")
    print(f"New repositories created: {created_count}")
    if args.push:
        print(f"Repositories pushed: {pushed_count}")
    print(f"Gitea server: {gitea_url}")


if __name__ == "__main__":
    main()
