#requires -Version 5.1
<#
Autoteste OFFLINE da Central de Auditoria.
Cria repositorio Git temporario, gera fixture sem dados reais, valida
sanitizacao e rejeicao de campos extras. NUNCA faz push.
#>
[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$collector = Join-Path $PSScriptRoot 'REGISTRAR_AUDITORIA_GITHUB_19_9.ps1'
$tokens = $null
$errors = $null
[void][System.Management.Automation.Language.Parser]::ParseFile($collector,[ref]$tokens,[ref]$errors)
if (@($errors).Count -gt 0) {
 foreach ($e in $errors) { Write-Host $e.Message -ForegroundColor Red }
 throw 'Parser PowerShell falhou na Central de Auditoria.'
}
if (-not (Get-Command git -ErrorAction SilentlyContinue)) { throw 'git nao encontrado.' }
$temp = Join-Path ([IO.Path]::GetTempPath()) ('ALASER_AUDIT_TEST_' + [guid]::NewGuid().ToString('N'))
try {
 $scripts = Join-Path $temp 'SCRIPTS'
 $reports = Join-Path $temp 'RELATORIOS\AUDITORIAS\GITHUB_19_8\BEZIER_PERFORMANCE_19_9'
 New-Item -ItemType Directory -Path $scripts,$reports -Force | Out-Null
 Copy-Item -LiteralPath $collector -Destination (Join-Path $scripts 'REGISTRAR_AUDITORIA_GITHUB_19_9.ps1')
 & git -C $temp init -q
 if ($LASTEXITCODE -ne 0) { throw 'git init falhou no ambiente isolado.' }
 & git -C $temp checkout -q -b 'fix/curvas-tesoura-preflight-19-8-20261030'
 if ($LASTEXITCODE -ne 0) { throw 'git checkout falhou no ambiente isolado.' }
 $fixture = [ordered]@{
  audit = 'alencar-laser-cache-zoom-review-19.9'
  status = 'PASS_AUTOMATED_MANUAL_PENDING'
  source_sha256 = ('A' * 64)
  checks_failed = @()
  hardware_permitted = $false
  production_homologated = $false
  input_json = 'C:\Users\CLIENTE_PRIVADO_XYZ\projeto_cliente.lbrn2'
  cliente_nome = 'CLIENTE_PRIVADO_XYZ'
 }
 $json = Join-Path $reports 'AUDITORIA_CACHE_ZOOM_19_9_TESTE.json'
 [IO.File]::WriteAllText($json,($fixture | ConvertTo-Json -Depth 5),
   [Text.UTF8Encoding]::new($false))
 $exe = Join-Path $scripts 'REGISTRAR_AUDITORIA_GITHUB_19_9.ps1'
 & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $exe
 if ($LASTEXITCODE -ne 0) { throw 'Fixture valida nao foi aceita.' }
 $idx = Join-Path $temp 'AUDITORIA_GITHUB\INDICE.json'
 if (-not (Test-Path -LiteralPath $idx)) { throw 'Indice nao gerado.' }
 $index = Get-Content -LiteralPath $idx -Raw | ConvertFrom-Json
 if ([int]$index.records_count -ne 1) { throw 'Quantidade de registros incorreta.' }
 $recordPath = Join-Path $temp (($index.records[0].path) -replace '/','\')
 $data = Get-Content -LiteralPath $recordPath -Raw
 if ($data.Contains('CLIENTE_PRIVADO_XYZ') -or $data.Contains('C:\Users\')) {
  throw 'Dado sensivel vazou para registro filtrado.'
 }
 $rec = $data | ConvertFrom-Json
 if ($rec.status -ne 'PASS_AUTOMATED_MANUAL_PENDING' -or
     $rec.production_homologated -ne $false) {
  throw 'Estado de homologacao incorreto.'
 }
 # Injetar campo nao autorizado em uma copia temporaria do registro.
 $rec | Add-Member -NotePropertyName 'cliente_nome' -NotePropertyValue 'CLIENTE_PRIVADO_XYZ'
 [IO.File]::WriteAllText($recordPath,($rec | ConvertTo-Json -Depth 6),
   [Text.UTF8Encoding]::new($false))
 # O processo filho DEVE falhar por seguranca. Em Windows PowerShell 5.1,
 # a saida stderr redirecionada via 2>&1 pode virar NativeCommandError
 # com ErrorActionPreference=Stop e interromper o proprio autoteste.
 # Start-Process captura os dois fluxos sem converter stderr em excecao.
 $negativeOut = Join-Path $temp 'NEGATIVE_STDOUT.txt'
 $negativeErr = Join-Path $temp 'NEGATIVE_STDERR.txt'
 $child = Start-Process -FilePath 'powershell.exe' -ArgumentList @(
   '-NoProfile','-ExecutionPolicy','Bypass','-File',('"' + $exe + '"')
 ) -Wait -PassThru -NoNewWindow -RedirectStandardOutput $negativeOut -RedirectStandardError $negativeErr
 if ($child.ExitCode -eq 0) {
  throw 'Falha de seguranca: campo inesperado nao foi rejeitado.'
 }
 $negativeMessage = (Get-Content -LiteralPath $negativeErr -Raw -ErrorAction SilentlyContinue) +
   (Get-Content -LiteralPath $negativeOut -Raw -ErrorAction SilentlyContinue)
 if ($negativeMessage -notmatch 'Campo nao permitido' -or
     $negativeMessage -notmatch 'cliente_nome') {
  throw 'Autoteste negativo falhou por motivo diferente do bloqueio de privacidade.'
 }
 Write-Host 'PASS: campo proibido foi rejeitado corretamente no teste negativo.' -ForegroundColor Green
 Write-Host 'PASS: parser, fixture, indice, privacidade e rejeicao de campo extra.' -ForegroundColor Green
 Write-Host 'Autoteste isolado: nenhum envio ao GitHub ou uso de maquina laser.' -ForegroundColor Green
} finally {
 if ($temp -and (Test-Path -LiteralPath $temp) -and
     ([IO.Path]::GetFileName($temp) -like 'ALASER_AUDIT_TEST_*')) {
  Remove-Item -LiteralPath $temp -Recurse -Force -ErrorAction SilentlyContinue
 }
}
