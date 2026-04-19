#!/usr/bin/env bash
set -euo pipefail

# Update the repo + sync submodules to the commit pinned by this repo.
# (We intentionally do NOT `git pull` inside submodules, because that leaves the superproject dirty.)

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
