Param()
$ErrorActionPreference = 'Stop'

$repoRoot = (git rev-parse --show-toplevel).Trim()
$hooksDir = Join-Path $repoRoot ".git\hooks"
New-Item -ItemType Directory -Force -Path $hooksDir | Out-Null

$src = Join-Path $repoRoot "githooks\post-commit"
$dst = Join-Path $hooksDir "post-commit"

if (-not (Test-Path $src)) {
  throw "Missing $src"
}

Copy-Item -Force $src $dst

Write-Host "Installed post-commit hook -> .git\hooks\post-commit"
