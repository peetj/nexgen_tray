#!/usr/bin/env bash
set -euo pipefail

# Safe-ish updater: only pulls when working tree is clean.

is_clean() {
  git diff --quiet && git diff --cached --quiet
}

if ! is_clean; then
  echo "[update] Working tree not clean in $(pwd). Skipping git pull." >&2
  exit 0
fi

echo "[update] git pull"
git pull --rebase --autostash

echo "[update] submodule update --init --recursive"
git submodule update --init --recursive

# Pull submodule remotes too (only if clean)
echo "[update] submodule foreach pull"
git submodule foreach --recursive '
  if git diff --quiet && git diff --cached --quiet; then
    git pull --rebase --autostash || true
  else
    echo "[update] submodule $name dirty; skipping pull" >&2
  fi
'
