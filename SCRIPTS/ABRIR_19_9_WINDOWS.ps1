#requires -Version 5.1
<#
Alencar Laser 19.9 - Development Windows launcher.
Opens the already built Qt editor using MSYS2 UCRT64 runtime DLLs.
No rebuild, no machine connection, no persistence of PATH changes.
Usage (from repo root):
 powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\ABRIR_19_9_WINDOWS.ps1"
 powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\ABRIR_19_9_WINDOWS.ps1" -VerificarSomente
 powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\ABRIR_19_9_WINDOWS.ps1" -ModoCompatibilidade
 powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\ABRIR_19_9_WINDOWS.ps1" -UsarBuildDragAB
 powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\ABRIR_19_9_WINDOWS.ps1" -UsarBuildDragAB -TestarRepinturaLocal
#>
[CmdletBinding()]
param([switch]$VerificarSomente, [switch]$ModoCompatibilidade, [switch]$TestarRepinturaLocal, [switch]$UsarBuildDragAB)

$ErrorActionPreference = 'Stop'
$originalPath = $env:PATH
$originalGroupedDrag = [Environment]::GetEnvironmentVariable('ALENCAR_DISABLE_GROUPED_DRAG','Process')
$originalOverlay = [Environment]::GetEnvironmentVariable('ALENCAR_LOCAL_DRAG_OVERLAY_19_9','Process')
$didPush = $false
try {
    $root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
    $ucrtBin = 'C:\msys64\ucrt64\bin'
    $requiredDll = Join-Path $ucrtBin 'libminizip-1.dll'
    if ($TestarRepinturaLocal -and -not $UsarBuildDragAB) {
        throw 'Teste real de repintura 19.9 exige -UsarBuildDragAB para usar a mesma compilacao isolada nas sessoes OFF/ON.'
    }
    $exeBuild = if ($UsarBuildDragAB) { 'build_WINDOWS_DRAG_AB_19_9' } else { 'build_WINDOWS_GIT_19_8' }
    $exe = Join-Path $root ("DESENVOLVIMENTO\FONTE\$exeBuild\alencar_test7_qt6.exe")
    $fixture = Join-Path $root 'DESENVOLVIMENTO\FONTE\fixtures\web_nodes_normalized.csv'
    $sessionRoot = Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\sessao'
    $sessionDir = $sessionRoot
    if ($UsarBuildDragAB) {
        # An isolated, unique folder prevents the ON run from overwriting
        # the OFF session's SESSAO_INTERATIVA_TESTE_6.json.
        $modeLabel = if ($TestarRepinturaLocal) { 'ON' } else { 'OFF' }
        $stamp = Get-Date -Format 'yyyyMMdd_HHmmss'
        $nonce = [Guid]::NewGuid().ToString('N').Substring(0,8)
        $sessionDir = Join-Path $sessionRoot ("DRAG_AB_19_9_" + $modeLabel + "_" + $stamp + "_" + $nonce)
    }
    foreach ($file in @($exe, $fixture)) {
        if (-not (Test-Path -LiteralPath $file -PathType Leaf)) {
            throw "Arquivo ausente: $file. Compile a candidata com TESTAR_19_8_WINDOWS.ps1 primeiro."
        }
    }
    if (-not (Test-Path -LiteralPath $requiredDll -PathType Leaf)) {
        throw "Biblioteca ausente: $requiredDll. Instale pelo MSYS2 UCRT64: pacman -S mingw-w64-ucrt-x86_64-minizip. Nunca baixe DLLs isoladas de sites de terceiros."
    }
    Write-Host 'Alencar Laser - runtime UCRT64 encontrado.' -ForegroundColor Green
    Write-Host "Executavel: $exe"
    if ($UsarBuildDragAB) {
        Write-Host 'COMPARACAO REAL: mesmo executavel isolado para modo OFF e ON.' -ForegroundColor Cyan
    }
    Write-Host "MiniZip: $requiredDll"
    if ($VerificarSomente) {
        Write-Host 'Preflight de arquivos: PASS. O editor nao foi iniciado.' -ForegroundColor Green
        return
    }
    [void](New-Item -ItemType Directory -Path $sessionDir -Force)
    if ($UsarBuildDragAB) { Write-Host "Pasta exclusiva desta sessao: $sessionDir" -ForegroundColor Cyan }
    $env:PATH = $ucrtBin + ';' + $originalPath
    if ($ModoCompatibilidade) {
        # Disable only transient grouped-drag optimization. Files, history
        # and canonical shapes remain untouched.
        $env:ALENCAR_DISABLE_GROUPED_DRAG = '1'
        Write-Warning 'MODO COMPATIBILIDADE: arraste agrupado experimental desativado nesta sessao.'
    }
    if ($TestarRepinturaLocal) {
        $env:ALENCAR_LOCAL_DRAG_OVERLAY_19_9 = '1'
        Write-Warning 'EXPERIMENTO: invalidação local de overlay no arraste ativada somente nesta sessao. Sem homologacao.'
    }
    Push-Location -LiteralPath $root
    $didPush = $true
    Write-Host 'Abrindo editor experimental. Mantenha o laser desconectado.' -ForegroundColor Yellow
    $inicio = Get-Date
    & $exe --interactive $fixture $sessionDir
    $codigoEditor = $LASTEXITCODE
    $tempo = [math]::Round(((Get-Date)-$inicio).TotalSeconds,1)
    Write-Host "Editor encerrou; duracao: $tempo segundos; codigo: $codigoEditor" -ForegroundColor Cyan
    if ($UsarBuildDragAB) {
        $telemetria = Join-Path $sessionDir 'SESSAO_INTERATIVA_TESTE_6.json'
        if (Test-Path -LiteralPath $telemetria -PathType Leaf) {
            Write-Host "Relatorio automatico desta sessao: $telemetria" -ForegroundColor Green
        } else {
            Write-Warning "Relatorio automatico nao encontrado: $telemetria"
        }
    }
    if ($codigoEditor -ne 0) {
        throw "Editor encerrou com codigo $codigoEditor. Confira mensagens acima."
    }
}
catch {
    Write-Error $_.Exception.Message
    exit 1
}
finally {
    $env:PATH = $originalPath
    if ($null -eq $originalGroupedDrag) {
        [Environment]::SetEnvironmentVariable('ALENCAR_DISABLE_GROUPED_DRAG',$null,'Process')
    } else {
        [Environment]::SetEnvironmentVariable('ALENCAR_DISABLE_GROUPED_DRAG',$originalGroupedDrag,'Process')
    }
    [Environment]::SetEnvironmentVariable('ALENCAR_LOCAL_DRAG_OVERLAY_19_9',$originalOverlay,'Process')
    if ($didPush) { Pop-Location }
}
