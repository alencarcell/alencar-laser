#requires -Version 5.1
<#
ALENCAR LASER 19.9 - registro local de interacao (sem telemetria online).
Nao abre o aplicativo, nao controla a maquina e nao envia dados ao GitHub.
Os cinco resultados sao observacoes informadas pelo operador, nao testes
automaticos. O CSV opcional deve ser do medidor passivo de RAM/CPU.
#>
[CmdletBinding()]
param(
 [ValidateSet('PASS','FAIL','NOT_TESTED')][string]$Selecao = 'NOT_TESTED',
 [ValidateSet('PASS','FAIL','NOT_TESTED')][string]$Arraste = 'NOT_TESTED',
 [ValidateSet('PASS','FAIL','NOT_TESTED')][string]$ZoomPan = 'NOT_TESTED',
 [ValidateSet('PASS','FAIL','NOT_TESTED')][string]$Tesoura = 'NOT_TESTED',
 [ValidateSet('PASS','FAIL','NOT_TESTED')][string]$DesfazerRefazer = 'NOT_TESTED',
 [string]$CsvMemoria = '',
 [string]$ProjetoCopia = ''
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$raiz = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$destino = Join-Path $raiz 'RELATORIOS\AUDITORIAS\GITHUB_19_8\INTERACAO_19_9'
$mapa = [ordered]@{
 selection = $Selecao
 drag = $Arraste
 zoom_pan = $ZoomPan
 scissors = $Tesoura
 undo_redo = $DesfazerRefazer
}
$testados = @($mapa.Values | Where-Object { $_ -ne 'NOT_TESTED' }).Count
$falhas = @($mapa.Values | Where-Object { $_ -eq 'FAIL' }).Count
$memoria = [ordered]@{
 sample_count = 0
 rss_p95_mb = $null
 rss_max_mb = $null
 cpu_p95_percent = $null
}
function Numero([string]$texto) {
 $value = 0.0
 $ok = [double]::TryParse(
    $texto,
    [System.Globalization.NumberStyles]::Float,
    [System.Globalization.CultureInfo]::InvariantCulture,
    [ref]$value
 )
 if (-not $ok -or [double]::IsNaN($value) -or
     [double]::IsInfinity($value) -or $value -lt 0) {
  throw 'CSV possui valor numerico invalido.'
 }
 return [double]$value
}
function Percentil95([double[]]$numeros) {
 if ($numeros.Count -eq 0) { return $null }
 $sort = @($numeros | Sort-Object)
 $indice = [math]::Max(0,[math]::Ceiling($sort.Count*0.95)-1)
 return [math]::Round([double]$sort[$indice],4)
}
if ($CsvMemoria) {
 $csv = (Resolve-Path -LiteralPath $CsvMemoria).Path
 if ([IO.Path]::GetExtension($csv) -ine '.csv') { throw 'Memoria deve ser CSV.' }
 if ((Get-Item -LiteralPath $csv).Length -gt 16MB) { throw 'CSV maior que 16 MiB.' }
 $rows = @(Import-Csv -LiteralPath $csv -Delimiter ',')
 if ($rows.Count -lt 3 -or $rows.Count -gt 200000) {
  throw 'CSV precisa conter de 3 a 200000 amostras.'
 }
 $obrigatorios = @('timestamp_utc','pid','working_set_mb','private_bytes_mb',
                  'cpu_percent_total','threads','handles')
 foreach ($col in $obrigatorios) {
  if ($rows[0].PSObject.Properties.Name -notcontains $col) {
   throw "CSV incompleto: $col"
  }
 }
 $pidInicial = [string]$rows[0].pid
 if ($pidInicial -notmatch '^\d{1,10}$') { throw 'PID invalido no CSV.' }
 $rss = New-Object 'System.Collections.Generic.List[double]'
 $cpu = New-Object 'System.Collections.Generic.List[double]'
 foreach ($linha in $rows) {
  if ([string]$linha.pid -ne $pidInicial) { throw 'CSV mistura processos diferentes.' }
  $vRss = Numero ([string]$linha.working_set_mb)
  $vCpu = Numero ([string]$linha.cpu_percent_total)
  if ($vRss -gt 1048576 -or $vCpu -gt 10000) {
   throw 'CSV contem medidas fisicamente suspeitas.'
  }
  $rss.Add($vRss)
  $cpu.Add($vCpu)
 }
 $memoria['sample_count'] = [int]$rows.Count
 $memoria['rss_p95_mb'] = Percentil95 ($rss.ToArray())
 $memoria['rss_max_mb'] = [math]::Round([double](($rss.ToArray() | Measure-Object -Maximum).Maximum),4)
 $memoria['cpu_p95_percent'] = Percentil95 ($cpu.ToArray())
}
if ($testados -eq 0 -and [int]$memoria['sample_count'] -eq 0) {
 throw 'Informe ao menos um resultado manual ou CSV com amostras.'
}
$sha = $null
if ($ProjetoCopia) {
 $projeto = (Resolve-Path -LiteralPath $ProjetoCopia).Path
 if ([IO.Path]::GetExtension($projeto) -notin @('.lbrn2','.alaser')) {
  throw 'ProjetoCopia deve ser .lbrn2 ou .alaser.'
 }
 if ((Get-Item -LiteralPath $projeto).Length -gt 64MB) {
  throw 'ProjetoCopia maior que 64 MiB.'
 }
 $sha = (Get-FileHash -LiteralPath $projeto -Algorithm SHA256).Hash
}
$registro = [ordered]@{
 experiment = 'manual-interaction-19.9'
 status = 'MANUAL_REVIEW_PENDING'
 captured_utc = [DateTime]::UtcNow.ToString('yyyy-MM-ddTHH:mm:ssZ')
 source_sha256 = $sha
 machine_commands_sent = $null
 production_homologated = $false
 manual_validation_pending = $true
 manual_results = $mapa
 tested_cases = [int]$testados
 failed_cases = [int]$falhas
 memory = $memoria
}
[void](New-Item -ItemType Directory -Path $destino -Force)
$arquivo = Join-Path $destino ('SESSAO_INTERACAO_19_9_' +
    (Get-Date -Format 'yyyyMMdd_HHmmss') + '.json')
$json = $registro | ConvertTo-Json -Depth 7
[IO.File]::WriteAllText($arquivo,($json + [Environment]::NewLine),
   [System.Text.UTF8Encoding]::new($false))
Write-Host ("Sessao local registrada: {0} casos testados, {1} falhas, {2} amostras de memoria." -f
    $testados,$falhas,$memoria['sample_count']) -ForegroundColor Green
Write-Host "JSON local: $arquivo"
Write-Host 'SEM ENVIO. Este registro e observacional e nao homologa a maquina.' -ForegroundColor Yellow
