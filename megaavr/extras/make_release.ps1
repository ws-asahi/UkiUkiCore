# ============================================================================
#  make_release.ps1 - UkiUkiCore Boards Manager release packager (PowerShell)
#
#  PowerShell twin of make_release.sh (that one is kept for Linux/CI; it must
#  be checked out with LF endings - enforced via .gitattributes - or bash
#  fails with "env: 'bash\r': No such file or directory").
#
#  Builds dist\UkiUkiCore-<ver>.tar.bz2 from megaavr\ (same layout as
#  WazamonoCore releases: single root folder "UkiUkiCore-<ver>/" holding
#  boards.txt / platform.txt / cores/ ... directly), computes SHA-256 + size,
#  and writes version/url/archiveFileName/checksum/size into
#  docs\package_ukiuki_index.json IN PLACE, touching only those lines inside
#  the platforms[0] block (format-preserving; tool entries with their own
#  "version" fields are never modified).
#
#  Usage (Windows PowerShell 5.1 or pwsh 7, from anywhere):
#      .\megaavr\extras\make_release.ps1            # version from platform.txt
#      .\megaavr\extras\make_release.ps1 0.1.0      # explicit version
#
#  Requirements: tar.exe (bundled with Windows 10 1803+; libarchive build
#  with bzip2 support) - checked at startup.
#
#  After running:
#    1. Review + commit docs\package_ukiuki_index.json
#    2. git tag v<ver>; git push --tags
#    3. Create GitHub Release v<ver>, upload dist\UkiUkiCore-<ver>.tar.bz2
#    4. Push main; GitHub Pages (docs/) serves
#       https://ws-asahi.github.io/UkiUkiCore/package_ukiuki_index.json
# ============================================================================
[CmdletBinding()]
param(
    [Parameter(Position = 0)]
    [string]$Version
)

$ErrorActionPreference = 'Stop'

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$MegaAvr  = Join-Path $RepoRoot 'megaavr'
$Index    = Join-Path $RepoRoot 'docs\package_ukiuki_index.json'
$Dist     = Join-Path $RepoRoot 'dist'

# --- Preconditions -----------------------------------------------------------
$tarCmd = Get-Command tar.exe -ErrorAction SilentlyContinue
if (-not $tarCmd) { throw "tar.exe not found (Windows 10 1803+ ships it in System32)." }
if (-not (Test-Path $Index)) { throw "Index not found: $Index" }

# --- Version: default = LAST '^version=' line of platform.txt ---------------
if (-not $Version) {
    $Version = (Select-String -Path (Join-Path $MegaAvr 'platform.txt') `
                              -Pattern '^version=(.+)$' |
                Select-Object -Last 1).Matches[0].Groups[1].Value.Trim()
}
if ($Version -notmatch '^[0-9]+(\.[0-9]+)*$') { throw "Bad version: '$Version'" }
Write-Host "Packaging UkiUkiCore version: $Version"

$Name    = "UkiUkiCore-$Version"
$Stage   = Join-Path ([System.IO.Path]::GetTempPath()) ("ukiuki_rel_" + [guid]::NewGuid().ToString('N'))
$StageIn = Join-Path $Stage $Name
$null    = New-Item -ItemType Directory -Path $StageIn -Force
$null    = New-Item -ItemType Directory -Path $Dist    -Force

try {
    # --- Stage megaavr\ minus machine-local / junk files (robocopy /E) ------
    # Boards Manager installs resolve {runtime.tools.*} from toolsDependencies,
    # so the local-toolchain shim files are excluded from the archive.
    $null = robocopy $MegaAvr $StageIn /E /NFL /NDL /NJH /NJS /NP `
        /XF platform.local.txt make_platform_local.bat .DS_Store *.pyc `
        /XD __pycache__
    if ($LASTEXITCODE -ge 8) { throw "robocopy failed (exit $LASTEXITCODE)" }
    $global:LASTEXITCODE = 0

    # --- Create tar.bz2 with single root folder ----------------------------
    $Tarball = Join-Path $Dist "$Name.tar.bz2"
    if (Test-Path $Tarball) { Remove-Item $Tarball -Force }
    & tar.exe -C $Stage -cjf $Tarball $Name
    if ($LASTEXITCODE -ne 0) { throw "tar.exe failed (exit $LASTEXITCODE)" }

    $Sha  = (Get-FileHash -Path $Tarball -Algorithm SHA256).Hash.ToLower()
    $Size = (Get-Item $Tarball).Length
    Write-Host "  archive : $Tarball"
    Write-Host "  sha256  : $Sha"
    Write-Host "  size    : $Size"

    # --- Update index in place (format-preserving line pass) ----------------
    # Only lines between '"platforms": [' and its '"boards"' key are eligible,
    # so the "version" fields of toolsDependencies / tools stay untouched.
    $url      = "https://github.com/ws-asahi/UkiUkiCore/releases/download/v$Version/UkiUkiCore-$Version.tar.bz2"
    $inPlat   = $false
    $done     = $false
    $lines    = Get-Content -Path $Index -Encoding UTF8
    $outLines = foreach ($line in $lines) {
        if (-not $done) {
            if ($line -match '"platforms"\s*:\s*\[') { $inPlat = $true }
            elseif ($inPlat -and $line -match '"boards"\s*:') { $inPlat = $false; $done = $true }
            elseif ($inPlat) {
                if     ($line -match '^(\s*)"version"\s*:')         { $line = $Matches[1] + "`"version`": `"$Version`"," }
                elseif ($line -match '^(\s*)"url"\s*:')             { $line = $Matches[1] + "`"url`": `"$url`"," }
                elseif ($line -match '^(\s*)"archiveFileName"\s*:') { $line = $Matches[1] + "`"archiveFileName`": `"$Name.tar.bz2`"," }
                elseif ($line -match '^(\s*)"checksum"\s*:')        { $line = $Matches[1] + "`"checksum`": `"SHA-256:$Sha`"," }
                elseif ($line -match '^(\s*)"size"\s*:')            { $line = $Matches[1] + "`"size`": `"$Size`"," }
            }
        }
        $line
    }
    if (-not $done) { throw "platforms[0] block not found in $Index - index not modified as expected." }

    # Write back as UTF-8 (no BOM) with LF endings, matching the repo file.
    $text = ($outLines -join "`n") + "`n"
    [System.IO.File]::WriteAllText($Index, $text, [System.Text.UTF8Encoding]::new($false))
    Write-Host "Updated $Index -> platform $Version"

    # Sanity check: file must still parse as JSON.
    $null = Get-Content -Raw -Path $Index | ConvertFrom-Json
    Write-Host "Done. Next: commit index, tag v$Version, upload $Name.tar.bz2 to the GitHub Release."
}
finally {
    if (Test-Path $Stage) { Remove-Item $Stage -Recurse -Force }
}
