# Script PowerShell pour compiler et executer les tests unitaires
# Usage: .\build_and_test.ps1

Write-Host "========================================================" -ForegroundColor Cyan
Write-Host "     Compilation des Tests Unitaires - Basic80         " -ForegroundColor Cyan
Write-Host "========================================================" -ForegroundColor Cyan
Write-Host ""

# Creer le repertoire obj s'il n'existe pas
if (!(Test-Path "obj")) {
    New-Item -ItemType Directory -Path "obj" | Out-Null
    Write-Host "[OK] Repertoire obj cree" -ForegroundColor Green
}

# Compiler tous les fichiers
Write-Host "Compilation en cours..." -ForegroundColor Yellow

$srcFiles = @(
    "..\..\src\lexer.c",
    "..\..\src\vars.c", 
    "..\..\src\expr.c",
    "..\..\src\interp.c",
    "..\..\src\commands.c",
    "..\..\src\ctrlflow.c",
    "tstfrmwk.c",
    "tstlexer.c",
    "tst_vars.c",
    "tst_expr.c",
    "tstintrp.c",
    "runtests.c"
)

$compileCmd = "clang -std=c89 -pedantic -Wall -g -I..\..\include " + ($srcFiles -join " ") + " -o runtests.exe"

Write-Host "Commande: $compileCmd" -ForegroundColor Gray
$output = Invoke-Expression $compileCmd 2>&1

if ($LASTEXITCODE -eq 0) {
    Write-Host "[OK] Compilation reussie!" -ForegroundColor Green
    Write-Host ""
    Write-Host "========================================================" -ForegroundColor Cyan
    Write-Host "         Execution des Tests Unitaires                 " -ForegroundColor Cyan
    Write-Host "========================================================" -ForegroundColor Cyan
    Write-Host ""
    
    .\runtests.exe
    
    Write-Host ""
    if ($LASTEXITCODE -eq 0) {
        Write-Host "[OK] Tests termines avec succes!" -ForegroundColor Green
    } else {
        Write-Host "[ERREUR] Certains tests ont echoue" -ForegroundColor Red
    }
} else {
    Write-Host "[ERREUR] Erreur de compilation:" -ForegroundColor Red
    Write-Host $output
    exit 1
}
