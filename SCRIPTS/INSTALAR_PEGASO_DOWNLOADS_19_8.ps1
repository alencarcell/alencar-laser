# Alencar Laser - instalador simples da identidade PEGASO_V1 (somente DEV).
# Coloque Alencar_Laser_Pegaso_Kit_v1.zip na pasta Downloads.
# No terminal VS Code na raiz: powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\INSTALAR_PEGASO_DOWNLOADS_19_8.ps1"
# Opcional: -Simular (confere tudo, sem copiar), -ArquivoZip "D:\...\kit.zip"
param(
    [string]$ArquivoZip = "",
    [switch]$Simular
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$nomeBranch = 'fix/validacao-real-19-8-20261029'
$nomeKit = 'Alencar_Laser_Pegaso_Kit_v1.zip'
$shaOficial = 'f1db7c250d72b9d702af3a1d493079d32e0fc3744465affee54edfa5bdaaecee'
$raiz = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

if (-not (Test-Path -LiteralPath (Join-Path $raiz 'DESENVOLVIMENTO\FONTE\qt\branding_pegaso_19_8.hpp') -PathType Leaf)) {
    throw "A candidata nao possui o carregador de marca. Atualize a branch experimental do GitHub antes de instalar."
}
if (-not (Test-Path -LiteralPath (Join-Path $raiz '.git'))) {
    throw "Este caminho nao e o repositorio: $raiz"
}

Push-Location -LiteralPath $raiz
try {
    $branchAtual = (& git branch --show-current)
    if ($LASTEXITCODE -ne 0 -or ($branchAtual -join '').Trim() -ne $nomeBranch) {
        throw "Branch inesperada. Use apenas '$nomeBranch'; branch atual: $branchAtual"
    }
} finally { Pop-Location }

if ($ArquivoZip.Trim()) {
    if (-not (Test-Path -LiteralPath $ArquivoZip -PathType Leaf)) { throw "ZIP nao encontrado: $ArquivoZip" }
    $candidatos = @(Get-Item -LiteralPath $ArquivoZip)
} else {
    $pastas = New-Object System.Collections.Generic.List[string]
    $pastas.Add((Join-Path ([Environment]::GetFolderPath('UserProfile')) 'Downloads'))
    try {
        $reg = Get-ItemProperty -Path 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\User Shell Folders'
        $registroDownloads = $reg.'{374DE290-123F-4565-9164-39C4925E467B}'
        if ($registroDownloads) { $pastas.Add([Environment]::ExpandEnvironmentVariables([string]$registroDownloads)) }
    } catch { }
    $candidatos = @(
        foreach ($pasta in @($pastas | Select-Object -Unique)) {
            if (Test-Path -LiteralPath $pasta -PathType Container) {
                Get-ChildItem -LiteralPath $pasta -File -Filter 'Alencar_Laser_Pegaso_Kit_v1*.zip'
            }
        }
    ) | Sort-Object LastWriteTime -Descending
}
if (@($candidatos).Count -eq 0) {
    throw "ZIP nao localizado em Downloads. Baixe '$nomeKit' e deixe-o la, sem extrair."
}

$zip = $null
foreach ($file in @($candidatos)) {
    $hash = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($hash -eq $shaOficial) { $zip = $file; break }
}
if ($null -eq $zip) {
    throw "ZIP encontrado, mas o SHA-256 nao corresponde ao kit aprovado. Nao foi instalado. Baixe novamente a versao V1."
}

$extraido = Join-Path ([System.IO.Path]::GetTempPath()) ('ALENCAR_PEGASO_V1_' + [guid]::NewGuid().ToString('N'))
$nomePasta = 'PEGASO_V1'
$raizLogos = Join-Path $raiz 'DADOS\LOGO'
$destino = Join-Path $raizLogos $nomePasta
$backupBase = Join-Path $raiz 'BACKUP\IDENTIDADE_PEGASO'
$stamp = (Get-Date -Format 'yyyyMMdd_HHmmss') + '_' + [guid]::NewGuid().ToString('N').Substring(0,8)
$backupPasta = Join-Path (Join-Path $backupBase $stamp) $nomePasta
$temporaria = Join-Path $raizLogos ('PEGASO_V1_INSTALANDO_' + [guid]::NewGuid().ToString('N'))
$movidoAnterior = $false
$instalado = $false

function Conferir-Recursos([string]$pasta) {
    $manifestPath = Join-Path $pasta 'manifesto.json'
    if (-not (Test-Path -LiteralPath $manifestPath -PathType Leaf)) { throw "Manifesto ausente: $manifestPath" }
    $manifesto = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
    $arquivos = @($manifesto.arquivos)
    if ($arquivos.Count -ne 13) { throw "Quantidade inesperada de itens no manifesto ($($arquivos.Count))." }
    foreach ($arq in $arquivos) {
        $nome = [string]$arq.nome
        if ($nome -ne [IO.Path]::GetFileName($nome) -or $nome -eq '.' -or $nome -eq '..') {
            throw "Nome inseguro no manifesto: $nome"
        }
        $caminho = Join-Path $pasta $nome
        if (-not (Test-Path -LiteralPath $caminho -PathType Leaf)) { throw "Recurso ausente: $nome" }
        if ((Get-Item -LiteralPath $caminho).Length -ne [long]$arq.bytes) { throw "Tamanho inesperado: $nome" }
        $hashAtual = (Get-FileHash -LiteralPath $caminho -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($hashAtual -ne ([string]$arq.sha256).ToLowerInvariant()) { throw "Hash incorreto: $nome" }
    }
    foreach ($necessario in @('marca_cabecalho_compacta.png','pegaso_icone_256.png','abertura_pegaso.png','pegaso_aprovado_original.png','alencar_laser_pegaso.ico')) {
        if (-not (Test-Path -LiteralPath (Join-Path $pasta $necessario) -PathType Leaf)) {
            throw "Falta recurso obrigatorio: $necessario"
        }
    }
}

try {
    Write-Host "ALENCAR LASER - PEGASO V1 / CANDIDATA OFFLINE" -ForegroundColor Cyan
    Write-Host "ZIP verificado: $($zip.FullName)"
    Write-Host "Destino: $destino"
    Write-Host "Modo: $(if ($Simular) { 'SIMULACAO' } else { 'INSTALAR' })"
    Expand-Archive -LiteralPath $zip.FullName -DestinationPath $extraido -Force
    $origem = Join-Path $extraido 'DADOS\LOGO\PEGASO_V1'
    if (-not (Test-Path -LiteralPath $origem -PathType Container)) {
        throw "Estrutura do kit invalida: DADOS/LOGO/PEGASO_V1 ausente."
    }
    Conferir-Recursos $origem

    $mesmoConteudo = $false
    if (Test-Path -LiteralPath $destino -PathType Container) {
        try {
            Conferir-Recursos $destino
            $mesmoConteudo = ((Get-FileHash -LiteralPath (Join-Path $destino 'manifesto.json') -Algorithm SHA256).Hash -eq
                (Get-FileHash -LiteralPath (Join-Path $origem 'manifesto.json') -Algorithm SHA256).Hash)
        } catch { $mesmoConteudo = $false }
    }
    if ($mesmoConteudo) {
        Write-Host "O kit aprovado ja esta instalado, sem necessidade de copiar." -ForegroundColor Green
        return
    }
    if ($Simular) {
        Write-Host "SIMULACAO PASS: os arquivos foram verificados. Nenhum arquivo foi instalado." -ForegroundColor Green
        if (Test-Path -LiteralPath $destino) { Write-Host "A pasta atual seria movida para BACKUP antes da substituicao." }
        return
    }

    New-Item -ItemType Directory -Path $raizLogos -Force | Out-Null
    Copy-Item -LiteralPath $origem -Destination $temporaria -Recurse -Force
    Conferir-Recursos $temporaria

    if (Test-Path -LiteralPath $destino) {
        New-Item -ItemType Directory -Path (Split-Path -Parent $backupPasta) -Force | Out-Null
        Move-Item -LiteralPath $destino -Destination $backupPasta
        $movidoAnterior = $true
        Write-Host "Identidade anterior salva em BACKUP: $backupPasta"
    }
    try {
        Move-Item -LiteralPath $temporaria -Destination $destino
        Conferir-Recursos $destino
        $instalado = $true
    } catch {
        if (Test-Path -LiteralPath $destino) {
            $falha = Join-Path $backupBase ('INSTALACAO_INCOMPLETA_' + $stamp)
            New-Item -ItemType Directory -Path $backupBase -Force | Out-Null
            Move-Item -LiteralPath $destino -Destination $falha
        }
        if ($movidoAnterior -and (Test-Path -LiteralPath $backupPasta)) {
            Move-Item -LiteralPath $backupPasta -Destination $destino
        }
        throw
    }
    Write-Host "INSTALACAO CONCLUIDA: $destino" -ForegroundColor Green
    Write-Host "Nao foram alterados APLICATIVO, logo antiga, executavel, arquivos .alaser ou hardware." -ForegroundColor Green
    Write-Host "O aplicativo 19.8 experimental deve ser aberto apos os testes para ver a marca."
} finally {
    if (Test-Path -LiteralPath $extraido) {
        Remove-Item -LiteralPath $extraido -Recurse -Force -ErrorAction SilentlyContinue
    }
    if (Test-Path -LiteralPath $temporaria) {
        # So elimina a pasta temporaria criada por este instalador.
        Remove-Item -LiteralPath $temporaria -Recurse -Force -ErrorAction SilentlyContinue
    }
}
