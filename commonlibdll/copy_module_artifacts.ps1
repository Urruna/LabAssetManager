param(
    [Parameter(Mandatory = $true)][string]$IncludeSource,
    [Parameter(Mandatory = $true)][string]$IncludeDest,
    [Parameter(Mandatory = $true)][string]$LibSource,
    [Parameter(Mandatory = $true)][string]$LibDest
)

$ErrorActionPreference = "Stop"

New-Item -ItemType Directory -Path $IncludeDest -Force | Out-Null
New-Item -ItemType Directory -Path $LibDest -Force | Out-Null

Get-ChildItem -LiteralPath $IncludeSource -Filter "*.h" -File |
    Where-Object {
        $_.Name -notlike "moc_*" -and
        $_.Name -notlike "ui_*"
    } |
    Copy-Item -Destination $IncludeDest -Force

if (Test-Path -LiteralPath $LibSource) {
    Copy-Item -LiteralPath $LibSource -Destination $LibDest -Force
}
