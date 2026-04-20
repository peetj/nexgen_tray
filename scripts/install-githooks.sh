#!/usr/bin/env bash
set -euo pipefail

# Install repo versioned githooks into .git/hooks by copying.
# Usage: ./scripts/install-githooks.sh

repo_root=$(git rev-parse --show-toplevel)
mkdir -p "$repo_root/.git/hooks"

src="$repo_root/githooks/post-commit"
dst="$repo_root/.git/hooks/post-commit"

if [[ ! -f "$src" ]]; then
  echo "Missing $src" >&2
  exit 1
fi

cp "$src" "$dst"
chmod +x "$dst"

echo "Installed post-commit hook -> .git/hooks/post-commit"
