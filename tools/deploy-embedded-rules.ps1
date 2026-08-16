# deploy-embedded-rules.ps1 — deploy the embedded-coding rules package.
# Deploys:
#   1. skills/embedded-coding/SKILL.md  -> ~/.agents/skills/embedded-coding/ (global layer)
#   2. skills/embedded-coding/SKILL.md  -> <project>/.agents/skills/embedded-coding/ (per -Projects; project layer overrides global)
#   3. rules/embedded-coding/AGENTS.inject.md -> <project>/AGENTS.md (managed block, idempotent)
#   4. rules/embedded-coding/AGENTS.inject.md -> ~/.dsh/AGENTS.md (global, appended once)
#
# Usage:
#   .\deploy-embedded-rules.ps1                               # global only
#   .\deploy-embedded-rules.ps1 -Projects E:\Project\modus_template,E:\Project\modus
#   .\deploy-embedded-rules.ps1 -Projects @("E:\Project\modus_template","E:\Project\modus")
#   .\deploy-embedded-rules.ps1 -Remove                       # uninstall all
#
# Idempotent: re-running replaces managed blocks in place. ASCII-only output.
param(
    [string[]]$Projects = @(),
    [switch]$Remove
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path      # modus/tools
$modusRoot = Split-Path -Parent $scriptDir                       # modus/
$skillSrc = Join-Path $modusRoot "rules\embedded-coding\SKILL.md"
$injectSrc = Join-Path $modusRoot "rules\embedded-coding\AGENTS.inject.md"

if (-not (Test-Path $skillSrc)) { Write-Host "[ERROR] SKILL.md not found: $skillSrc" -ForegroundColor Red; exit 1 }
if (-not (Test-Path $injectSrc)) { Write-Host "[ERROR] AGENTS.inject.md not found: $injectSrc" -ForegroundColor Red; exit 1 }

$utf8Bom = New-Object System.Text.UTF8Encoding($true)
$blockBegin = "# >>> embedded-coding managed block >>>"
$blockEnd = "# <<< embedded-coding managed block <<<"
$injectText = [System.IO.File]::ReadAllText($injectSrc, $utf8Bom).TrimEnd()
$managedBlock = $blockBegin + "`r`n" + $injectText + "`r`n" + $blockEnd

function Set-ManagedBlock { param([string]$file)
    $dir = Split-Path -Parent $file
    New-Item -ItemType Directory -Path $dir -Force | Out-Null
    if (Test-Path $file) {
        $content = [System.IO.File]::ReadAllText($file, $utf8Bom)
        if ($content.Contains($blockBegin)) {
            $pattern = "(?s)$([regex]::Escape($blockBegin)).*?$([regex]::Escape($blockEnd))"
            $content = [regex]::Replace($content, $pattern, $managedBlock)
        } else {
            $content = $content.TrimEnd() + "`r`n`r`n" + $managedBlock
        }
        [System.IO.File]::WriteAllText($file, $content, $utf8Bom)
    } else {
        [System.IO.File]::WriteAllText($file, $managedBlock, $utf8Bom)
    }
}

function Remove-ManagedBlock { param([string]$file)
    if (-not (Test-Path $file)) { return }
    $content = [System.IO.File]::ReadAllText($file, $utf8Bom)
    if ($content.Contains($blockBegin)) {
        $pattern = "(?s)$([regex]::Escape($blockBegin)).*?$([regex]::Escape($blockEnd))"
        $content = [regex]::Replace($content, $pattern, "").TrimEnd()
        [System.IO.File]::WriteAllText($file, $content, $utf8Bom)
    }
}

# ── 1. deploy skill ──────────────────────────────────────────────────────────
$globalSkillDir = Join-Path $env:USERPROFILE ".agents\skills\embedded-coding"
if ($Remove) {
    Remove-Item $globalSkillDir -Recurse -Force -ErrorAction SilentlyContinue
    Write-Host "[OK] removed global skill: $globalSkillDir"
} else {
    New-Item -ItemType Directory -Path $globalSkillDir -Force | Out-Null
    Copy-Item $skillSrc -Destination (Join-Path $globalSkillDir "SKILL.md") -Force
    Write-Host "[OK] skill -> $globalSkillDir"
}

# ── 2. project skills (project layer overrides global) ───────────────────────
foreach ($proj in $Projects) {
    $projSkillDir = Join-Path $proj ".agents\skills\embedded-coding"
    if ($Remove) {
        Remove-Item $projSkillDir -Recurse -Force -ErrorAction SilentlyContinue
        Write-Host "[OK] removed project skill: $projSkillDir"
    } else {
        New-Item -ItemType Directory -Path $projSkillDir -Force | Out-Null
        Copy-Item $skillSrc -Destination (Join-Path $projSkillDir "SKILL.md") -Force
        Write-Host "[OK] skill -> $projSkillDir"
    }
}

# ── 3. project AGENTS.md injection ───────────────────────────────────────────
foreach ($proj in $Projects) {
    $agentsFile = Join-Path $proj "AGENTS.md"
    if ($Remove) { Remove-ManagedBlock $agentsFile; Write-Host "[OK] removed block from $agentsFile" }
    else { Set-ManagedBlock $agentsFile; Write-Host "[OK] injected -> $agentsFile" }
}

# ── 4. global AGENTS.md (~/.dsh) ─────────────────────────────────────────────
$globalAgents = Join-Path $env:USERPROFILE ".dsh\AGENTS.md"
if ($Remove) {
    Remove-ManagedBlock $globalAgents
    Write-Host "[OK] removed block from $globalAgents"
} else {
    Set-ManagedBlock $globalAgents
    Write-Host "[OK] injected -> $globalAgents"
}

# ── 5. verify ────────────────────────────────────────────────────────────────
Write-Host ""
Write-Host "===== verify =====" -ForegroundColor Cyan
$fail = $false
if (-not $Remove) {
    if (Test-Path (Join-Path $globalSkillDir "SKILL.md")) { Write-Host "[OK] global skill in place" } else { Write-Host "[!!] global skill MISSING"; $fail = $true }
    foreach ($proj in $Projects) {
        if (Test-Path (Join-Path $proj ".agents\skills\embedded-coding\SKILL.md")) { Write-Host "[OK] project skill: $proj" } else { Write-Host "[!!] project skill MISSING: $proj"; $fail = $true }
        $agentsFile = Join-Path $proj "AGENTS.md"
        $agentsText = if (Test-Path $agentsFile) { [System.IO.File]::ReadAllText($agentsFile, $utf8Bom) } else { "" }
        if ($agentsText.Contains($blockBegin)) { Write-Host "[OK] AGENTS.md block: $proj" } else { Write-Host "[!!] AGENTS.md block MISSING: $proj"; $fail = $true }
    }
    $globalAgentsText = if (Test-Path $globalAgents) { [System.IO.File]::ReadAllText($globalAgents, $utf8Bom) } else { "" }
    if ($globalAgentsText.Contains($blockBegin)) { Write-Host "[OK] global AGENTS.md block" } else { Write-Host "[!!] global AGENTS.md block MISSING"; $fail = $true }
}
if ($fail) { Write-Host "[RESULT] FAILED - check paths above" -ForegroundColor Red; exit 1 }
Write-Host "[RESULT] OK" -ForegroundColor Green
Write-Host "Note: new sessions pick up skills and AGENTS.md automatically; restart DSH web if a session is already open."
