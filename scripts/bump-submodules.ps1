Param(
  [switch]$Commit = $true,
  [string]$Message = "Bump submodules"
)

$ErrorActionPreference = 'Stop'

$repoRoot = (git rev-parse --show-toplevel).Trim()
Set-Location $repoRoot

function Bump-One([string]$path) {
  if (-not (Test-Path $path)) { throw "Missing submodule at $path" }

  Write-Host "[bump] $path: fetch + fast-forward to origin/main"
  Push-Location $path
  try {
    git fetch origin main | Out-Null
    $hasMain = (git show-ref --verify --quiet refs/heads/main; $LASTEXITCODE) -eq 0
    if ($hasMain) {
      git checkout main | Out-Null
    } else {
      git checkout -b main --track origin/main | Out-Null
    }
    git pull --ff-only origin main | Out-Null
  } finally {
    Pop-Location
  }

  git add $path | Out-Null
}

Bump-One "external/nexgen_qt_sys"
Bump-One "external/nexgen_qt_themeset"

if ($Commit) {
  git diff --cached --quiet
  if ($LASTEXITCODE -eq 0) {
    Write-Host "[bump] no submodule updates to commit"
    exit 0
  }
  git commit -m $Message | Out-Null
  Write-Host "[bump] committed: $Message"
} else {
  Write-Host "[bump] staged submodule pointer updates (no commit)"
  Write-Host "       review with: git diff --cached"
}
