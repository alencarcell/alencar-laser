#requires -Version 5.1
<#
Valida um projeto LightBurn real no parser nativo 19.9 SEM abrir o editor.
Arquivo original: somente leitura. Nao ativa conexao nem emissao laser.
Por padrao abre seletor de arquivos. Opcional: -ArquivoLbrn2 <caminho>.
#>
[CmdletBinding()]
param([string]$ArquivoLbrn2 = '')

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$tester = Join-Path $PSScriptRoot 'TESTAR_19_8_WINDOWS.ps1'
if (-not (Test-Path -LiteralPath $tester -PathType Leaf)) {
    throw "Validador indisponivel: $tester"
}
if ([string]::IsNullOrWhiteSpace($ArquivoLbrn2)) {
    Add-Type -AssemblyName System.Windows.Forms
    $dialog = New-Object System.Windows.Forms.OpenFileDialog
    $dialog.Title = 'Escolha uma COPIA do arquivo LightBurn para auditoria sem alteracoes'
    $dialog.Filter = 'Projetos LightBurn (*.lbrn2)|*.lbrn2'
    $dialog.CheckFileExists = $true
    $dialog.Multiselect = $false
    $downloads = Join-Path $env:USERPROFILE 'Downloads'
    if (Test-Path -LiteralPath $downloads -PathType Container) {
        $dialog.InitialDirectory = $downloads
    }
    try {
        if ($dialog.ShowDialog() -ne [System.Windows.Forms.DialogResult]::OK) {
            Write-Host 'Cancelado. Nenhum arquivo foi analisado ou alterado.'
            return
        }
        $ArquivoLbrn2 = $dialog.FileName
    }
    finally {
        $dialog.Dispose()
    }
}
$selected = (Resolve-Path -LiteralPath $ArquivoLbrn2 -ErrorAction Stop).Path
if (-not (Test-Path -LiteralPath $selected -PathType Leaf) -or
    [System.IO.Path]::GetExtension($selected) -ine '.lbrn2') {
    throw 'Selecione um arquivo .lbrn2 existente.'
}
$before = Get-FileHash -LiteralPath $selected -Algorithm SHA256
Write-Host 'Arquivo lido (sem importacao GUI):' $selected -ForegroundColor Cyan
Write-Host 'SHA256 original:' $before.Hash
Write-Host 'Iniciando verificacao offline. Nunca envia dados para a maquina laser.' -ForegroundColor Yellow
$exitCode = 1
try {
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $tester -SomenteBezierNativo -ArquivoLbrn2 $selected
    $exitCode = $LASTEXITCODE
}
finally {
    $after = Get-FileHash -LiteralPath $selected -Algorithm SHA256
    if ($before.Hash -ne $after.Hash) {
        throw 'ALERTA: hash do arquivo .lbrn2 mudou durante a auditoria. Pare os testes.'
    }
    Write-Host 'Integridade do original: SHA256 preservado.' -ForegroundColor Green
}
if ($exitCode -ne 0) {
    throw "Validacao do arquivo real nao passou (codigo $exitCode). Envie o erro do terminal."
}
Write-Host 'AUDITORIA LBRN2 REAL 19.9: PASS offline. A abertura no editor e a fluidez ainda nao foram homologadas.' -ForegroundColor Green
