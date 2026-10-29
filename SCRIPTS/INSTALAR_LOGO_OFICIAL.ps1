param(
    [string]$Origem = (Join-Path $env:USERPROFILE 'Downloads\logotipo_neon_alencar_laser.png'),
    [string]$Raiz = 'C:\ProjetoAlencarLaser'
)
$ErrorActionPreference = 'Stop'
if (-not (Test-Path -LiteralPath $Origem -PathType Leaf)) {
    throw "Logo não encontrada em '$Origem'. Baixe a imagem original e salve-a em Downloads com o nome logotipo_neon_alencar_laser.png."
}
$destinoPasta = Join-Path $Raiz 'DADOS\LOGO'
New-Item -ItemType Directory -Path $destinoPasta -Force | Out-Null
$destino = Join-Path $destinoPasta 'logo_alencar_laser.png'
$novaHash = (Get-FileHash -LiteralPath $Origem -Algorithm SHA256).Hash
if (Test-Path -LiteralPath $destino -PathType Leaf) {
    $atualHash = (Get-FileHash -LiteralPath $destino -Algorithm SHA256).Hash
    if ($atualHash -eq $novaHash) {
        Write-Host "Logo oficial já instalada e idêntica: $destino" -ForegroundColor Green
        exit 0
    }
    $backupDir = Join-Path $Raiz 'BACKUP\LOGOS'
    New-Item -ItemType Directory -Path $backupDir -Force | Out-Null
    $backup = Join-Path $backupDir ("logo_alencar_laser_" + (Get-Date -Format 'yyyyMMdd_HHmmss_fff') + ".png")
    Copy-Item -LiteralPath $destino -Destination $backup -ErrorAction Stop
    Write-Host "Backup da logo anterior: $backup" -ForegroundColor Yellow
}
Copy-Item -LiteralPath $Origem -Destination $destino -Force -ErrorAction Stop
if ((Get-FileHash -LiteralPath $destino -Algorithm SHA256).Hash -ne $novaHash) {
    throw "O SHA-256 da logo copiada não confere com a origem"
}
Write-Host "Logo oficial instalada com SHA-256 conferido: $destino" -ForegroundColor Green
Write-Host "Reabra o Alencar Laser para atualizar o cabeçalho." -ForegroundColor Cyan
