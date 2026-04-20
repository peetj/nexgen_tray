#!/usr/bin/env bash
set -euo pipefail

# Bump external submodules to latest origin/main and stage the gitlink updates.
# Usage:
#   ./scripts/bump-submodules.sh           # stages changes (no commit)
#   COMMIT=1 ./scripts/bump-submodules.sh  # also creates a commit
#   MSG="Bump qt_sys + themeset" COMMIT=1 ./scripts/bump-submodules.sh
#
# Notes:
# - Requires the submodules to have an 'origin' remote and a 'main' branch.
# - This script does NOT push.

COMMIT="${COMMIT:-1}"
MSG="${MSG:-Bump submodules}"

repo_root=$(git rev-parse --show-toplevel)
cd "$repo_root"

bump_one() {
  local path="$1"
  if [[ ! -d "$path/.git" && ! -f "$path/.git" ]]; then
    echo "[bump] missing submodule at $path" >&2
    exit 1
  fi

  echo "[bump] $path: fetch + fast-forward to origin/main"
  (
    cd "$path"
    git fetch origin main
    if git show-ref --verify --quiet refs/heads/main; then
      git checkout main
    else
      git checkout -b main --track origin/main
    fi
    git pull --ff-only origin main
  )

  git add "$path"
}

bump_one external/nexgen_qt_sys
bump_one external/nexgen_qt_themeset

if [[ "$COMMIT" == "1" ]]; then
  if git diff --cached --quiet; then
    echo "[bump] no submodule updates to commit"
    exit 0
  fi
  git commit -m "$MSG"
  echo "[bump] committed: $MSG"
else
  echo "[bump] staged submodule pointer updates (no commit)"
  echo "       review with: git diff --cached"
fi
