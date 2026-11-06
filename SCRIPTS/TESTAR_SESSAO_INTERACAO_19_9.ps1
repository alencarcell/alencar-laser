#requires -Version 5.1
<#
Teste isolado do gerador de sessao 19.9. Fixtures sinteticas em %TEMP%.
Sem projeto real, rede, Git push ou conexao com laser.
#>
[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$fonte = Join-Path $PSScriptRoot 'GERAR_SESSAO_INTERACAO_19_9.ps1'
$tokens = $null
$erros = $null
[void][System.Management.Automation.Language.Parser]::ParseFile($fonte,[ref]$tokens,[ref]$erros)
if (@($erros).Count -gt 0) {
 foreach ($e in $erros) { Write-Host $e.Message -ForegroundColor Red }
 throw 'Parser do gerador de sessao falhou.'
}
$temp = Join-Path ([IO.Path]::GetTempPath()) ('ALASER_INTERACAO_TEST_' + [guid]::NewGuid().ToString('N'))
try {
 $scriptDir = Join-Path $temp 'SCRIPTS'
 [void](New-Item -ItemType Directory -Path $scriptDir -Force)
 $runner = Join-Path $scriptDir 'GERAR_SESSAO_INTERACAO_19_9.ps1'
 Copy-Item -LiteralPath $fonte -Destination $runner
 $project = Join-Path $temp 'CLIENTE_PRIVADO_XYZ.lbrn2'
 [IO.File]::WriteAllText($project,'FIXTURE_DE_TESTE_SEM_DADOS_REAIS',
     [Text.UTF8Encoding]::new($false))
 $csv = Join-Path $temp 'CLIENTE_PRIVADO_XYZ_MEMORIA.csv'
 $sample = @(
  'timestamp_utc,pid,working_set_mb,private_bytes_mb,cpu_percent_total,threads,handles',
  '2026-11-05T12:00:00Z,123,50.000,40.000,10.00,2,30',
  '2026-11-05T12:00:01Z,123,75.000,50.000,20.00,2,31',
  '2026-11-05T12:00:02Z,123,80.000,55.000,30.00,3,32'
 )
 [IO.File]::WriteAllLines($csv,$sample,[Text.UTF8Encoding]::new($false))
 $argsPs = @('-NoProfile','-ExecutionPolicy','Bypass','-File',
   $runner,'-Selecao','PASS','-Arraste','FAIL',
   '-CsvMemoria',$csv,'-ProjetoCopia',$project)
 & powershell.exe @argsPs
 if ($LASTEXITCODE -ne 0) { throw 'Geracao de sessao sintetica falhou.' }
 $reports = Join-Path $temp 'RELATORIOS\AUDITORIAS\GITHUB_19_8\INTERACAO_19_9'
 $found = @(Get-ChildItem -LiteralPath $reports -Filter 'SESSAO_INTERACAO_19_9_*.json' -File)
 if ($found.Count -ne 1) { throw 'Gerador nao produziu exatamente um relatorio.' }
 $raw = Get-Content -LiteralPath $found[0].FullName -Raw
 if ($raw.Contains('CLIENTE_PRIVADO_XYZ') -or $raw.Contains('C:\Users\')) {
  throw 'Caminho ou nome privado incluido no relatorio local.'
 }
 $j = $raw | ConvertFrom-Json
 if ($j.experiment -ne 'manual-interaction-19.9' -or
     $j.status -ne 'MANUAL_REVIEW_PENDING' -or
     $j.tested_cases -ne 2 -or $j.failed_cases -ne 1 -or
     $j.manual_results.selection -ne 'PASS' -or
     $j.manual_results.drag -ne 'FAIL' -or
     $j.manual_results.scissors -ne 'NOT_TESTED' -or
     $j.memory.sample_count -ne 3 -or
     $j.memory.rss_p95_mb -ne 80.0 -or
     $j.memory.rss_max_mb -ne 80.0 -or
     $j.memory.cpu_p95_percent -ne 30.0 -or
     $j.production_homologated -ne $false -or
     $null -ne $j.machine_commands_sent -or
     [string]$j.source_sha256 -notmatch '^[A-F0-9]{64}$') {
  throw 'Sessao gerada nao corresponde aos dados sinteticos.'
 }
 Write-Host 'PASS: geracao offline, memoria, hash, privacidade e homologacao pendente.' -ForegroundColor Green
} finally {
 if (Test-Path -LiteralPath $temp) {
  Remove-Item -LiteralPath $temp -Recurse -Force -ErrorAction SilentlyContinue
 }
}
