#requires -Version 5.1
<#
ALENCAR LASER 19.9 - Central de Auditoria GitHub (somente metadados).
Padrao: prepara registros filtrados localmente. -Publicar faz commit/push
apenas dos registros filtrados, mediante acao explicita do operador.
Nunca envia JSON bruto, arquivos .lbrn2/.alaser, logs, imagens ou caminhos locais.
#>
[CmdletBinding()]
param(
 [string]$ArquivoJson = '',
 [switch]$Todos,
 [switch]$Publicar
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$branchEsperada = 'fix/curvas-tesoura-preflight-19-8-20261030'
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$origem = Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\BEZIER_PERFORMANCE_19_9'
$interacao = Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\INTERACAO_19_9'
$baseRel = 'AUDITORIA_GITHUB'
$base = Join-Path $root $baseRel
$registros = Join-Path $base 'REGISTROS'
function Campo($obj, [string]$nome) {
 if ($null -eq $obj) { return $null }
 $p = $obj.PSObject.Properties[$nome]
 if ($null -eq $p) { return $null }
 return $p.Value
}
function Num($obj, [string]$nome) {
 $v = Campo $obj $nome
 if ($null -eq $v) { return $null }
 try {
  $n = [double]$v
  if ([double]::IsNaN($n) -or [double]::IsInfinity($n)) { return $null }
  return [math]::Round($n, 5)
 } catch { return $null }
}
function Estado($v) {
 switch ([string]$v) {
  'PASS' { return 'PASS' }
  'FAIL' { return 'FAIL' }
  'PASS_AUTOMATED_MANUAL_PENDING' { return 'PASS_AUTOMATED_MANUAL_PENDING' }
  'FAIL_AUTOMATED' { return 'FAIL_AUTOMATED' }
  'INCONCLUSIVE' { return 'INCONCLUSIVE' }
  'MANUAL_REVIEW_PENDING' { return 'MANUAL_REVIEW_PENDING' }
  default { return 'UNVERIFIED' }
 }
}
function JsonSeguro([object]$value, [string]$file) {
 $content = $value | ConvertTo-Json -Depth 9
 [IO.File]::WriteAllText($file,($content + [Environment]::NewLine),
     [System.Text.UTF8Encoding]::new($false))
}
# Conferir repositorio e branch, sem alterar nada.
$top = (& git -C $root rev-parse --show-toplevel) -join ''
if ($LASTEXITCODE -ne 0 -or -not $top) { throw 'Pasta nao e um repositorio Git valido.' }
$branch = (& git -C $root branch --show-current) -join ''
if ($LASTEXITCODE -ne 0 -or $branch -ne $branchEsperada) {
 throw "Branch incorreta: $branch. Esperada: $branchEsperada. Nenhuma alteracao."
}
$fontes = @()
if ($ArquivoJson) {
 $fontes = @(Get-Item -LiteralPath (Resolve-Path -LiteralPath $ArquivoJson).Path -ErrorAction Stop)
} else {
 $targets = @(
  [pscustomobject]@{ Folder=$origem; Pattern='AUDITORIA_CACHE_ZOOM_19_9_*.json' },
  [pscustomobject]@{ Folder=$origem; Pattern='COMPARACAO_CACHE_19_9_*.json' },
  [pscustomobject]@{ Folder=$interacao; Pattern='SESSAO_INTERACAO_19_9_*.json' }
 )
 foreach ($target in $targets) {
  if (-not (Test-Path -LiteralPath $target.Folder -PathType Container)) { continue }
  $found = @(Get-ChildItem -LiteralPath $target.Folder -Filter $target.Pattern -File |
     Sort-Object LastWriteTimeUtc -Descending)
  if ($Todos) { $fontes += $found }
  elseif ($found.Count -gt 0) { $fontes += $found[0] }
 }
}
if ($fontes.Count -eq 0) { throw 'Nenhum JSON compativel para coletar.' }
New-Item -ItemType Directory -Path $registros -Force | Out-Null
foreach ($f in $fontes) {
 if ($f.Extension -ine '.json' -or $f.Length -le 0 -or $f.Length -gt 4MB) {
  throw "Arquivo rejeitado por formato ou tamanho: $($f.Name)"
 }
 $digest = (Get-FileHash -LiteralPath $f.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
 $raw = Get-Content -LiteralPath $f.FullName -Raw | ConvertFrom-Json
 $type = ''
 if ((Campo $raw 'audit') -eq 'alencar-laser-cache-zoom-review-19.9') {
  $type = 'cache_zoom_validation'
 } elseif ((Campo $raw 'experiment') -eq 'native-device-coordinate-cache-19.9') {
  $type = 'cache_ab'
 } elseif ((Campo $raw 'experiment') -eq 'manual-interaction-19.9') {
  $type = 'interaction_session'
 } else {
  throw "Esquema nao autorizado para exportacao: $($f.Name)"
 }
 $shaOriginal = [string](Campo $raw 'source_sha256')
 if ($type -ne 'interaction_session' -and $shaOriginal -notmatch '^[0-9a-fA-F]{64}$') {
  throw "SHA da origem ausente/invalido: $($f.Name)"
 }
 if ($type -eq 'interaction_session' -and $shaOriginal -ne '' -and
     $shaOriginal -notmatch '^[0-9a-fA-F]{64}$') {
  throw "SHA opcional da sessao invalido: $($f.Name)"
 }
 $prefixoProjeto = $null
 if ($shaOriginal) { $prefixoProjeto = $shaOriginal.Substring(0,12).ToUpperInvariant() }
 # Identificadores de arquivo e metricas numericas apenas: nenhum campo livre
 # do JSON original e transportado ao GitHub.
 $dataUtc = $f.LastWriteTimeUtc.ToString('yyyy-MM-ddTHH:mm:ssZ')
 $status = Estado (Campo $raw 'status')
 $record = [ordered]@{
  schema = 'alencar-laser-audit-sanitized-v1'
  event_id = $digest.Substring(0,16)
  recorded_utc = $dataUtc
  version = '19.9-experimental'
  type = $type
  status = $status
  source_report_sha256 = $digest
  source_project_sha256_prefix = $prefixoProjeto
  machine_commands_sent = $null
  production_homologated = $false
  manual_validation_pending = $true
 }
 if ($type -eq 'interaction_session') {
  if ((Campo $raw 'status') -ne 'MANUAL_REVIEW_PENDING' -or
      (Campo $raw 'production_homologated') -ne $false -or
      (Campo $raw 'manual_validation_pending') -ne $true -or
      (Campo $raw 'machine_commands_sent') -ne $null) {
   throw 'Sessao nao possui os estados observacionais seguros.'
  }
  $manual = Campo $raw 'manual_results'
  $memory = Campo $raw 'memory'
  if ($null -eq $manual -or $null -eq $memory) { throw 'Sessao incompleta.' }
  $keys = @('selection','drag','zoom_pan','scissors','undo_redo')
  $statuses = [ordered]@{}
  foreach ($key in $keys) {
   $val = [string](Campo $manual $key)
   if (@('PASS','FAIL','NOT_TESTED') -notcontains $val) {
    throw "Resultado manual invalido: $key"
   }
   $statuses[$key] = $val
  }
  $t = @($statuses.Values | Where-Object { $_ -ne 'NOT_TESTED' }).Count
  $n = @($statuses.Values | Where-Object { $_ -eq 'FAIL' }).Count
  if ([int](Campo $raw 'tested_cases') -ne $t -or
      [int](Campo $raw 'failed_cases') -ne $n) {
   throw 'Contagem manual divergente.'
  }
  $samples = Num $memory 'sample_count'
  if ($null -eq $samples -or $samples -lt 0 -or $samples -gt 200000) {
   throw 'Contagem de memoria invalida.'
  }
  if ($t -eq 0 -and $samples -eq 0) { throw 'Sessao vazia.' }
  $rss95 = Num $memory 'rss_p95_mb'
  $rssMax = Num $memory 'rss_max_mb'
  $cpu95 = Num $memory 'cpu_p95_percent'
  if ($samples -gt 0 -and ($samples -lt 3 -or $null -eq $rss95 -or
       $null -eq $rssMax -or $null -eq $cpu95 -or
       $rss95 -lt 0 -or $rss95 -gt $rssMax -or
       $rssMax -gt 1048576 -or $cpu95 -lt 0 -or $cpu95 -gt 10000)) {
   throw 'Metricas de memoria invalidas.'
  }
  if ($samples -eq 0 -and ($null -ne $rss95 -or $null -ne $rssMax -or
       $null -ne $cpu95)) { throw 'Metricas de memoria sem amostras.' }
  $record['metrics'] = [ordered]@{
   selection = $statuses['selection']
   drag = $statuses['drag']
   zoom_pan = $statuses['zoom_pan']
   scissors = $statuses['scissors']
   undo_redo = $statuses['undo_redo']
   tested_cases = [int]$t
   failed_cases = [int]$n
   memory_sample_count = [int]$samples
   rss_p95_mb = $rss95
   rss_max_mb = $rssMax
   cpu_p95_percent = $cpu95
  }
 } elseif ($type -eq 'cache_zoom_validation') {
  if ((Campo $raw 'hardware_permitted') -eq $true -or
      (Campo $raw 'production_homologated') -eq $true) {
   throw 'Relatorio de validacao contradiz as travas de seguranca.'
  }
  $failed = Campo $raw 'checks_failed'
  $countFailed = if ($null -eq $failed) { 0 } else { @($failed).Count }
  $record['metrics'] = [ordered]@{
   checks_failed_count = [int]$countFailed
  }
 } else {
  if ((Campo $raw 'machine_commands_sent') -ne $false -or
      (Campo $raw 'source_intact') -ne $true -or
      (Campo $raw 'matching_source') -ne $true) {
   throw 'Relatorio A/B sem integridade ou sem bloqueio fisico comprovado.'
  }
  $record['machine_commands_sent'] = $false
  $a = Campo $raw 'no_cache'
  $b = Campo $raw 'device_cache'
  if ($null -eq $a -or $null -eq $b) { throw 'Relatorio A/B incompleto.' }
  $zs = @()
  foreach ($z in @(Campo $b 'zoom_audit')) {
   if ($null -eq $z) { continue }
   $zs += [ordered]@{
    factor = Num $z 'relative_zoom_factor'
    visible_contours = Num $z 'visible_native_items'
    cache_items = Num $z 'cache_items'
    p95_uncached_ms = Num $z 'no_cache_paint_p95_ms'
    p95_cached_ms = Num $z 'cached_paint_p95_ms'
    visual_pixels_over8_percent = Num $z 'visual_pixels_over8_percent'
   }
  }
  $record['metrics'] = [ordered]@{
   contours = Num $a 'contours'
   p95_uncached_ms = Num $a 'paint_p95_ms'
   p95_cached_ms = Num $b 'paint_p95_ms'
   reduction_percent = Num $raw 'paint_p95_reduction_percent'
   cached_items = Num $b 'native_device_cache_items'
   cache_estimated_bytes = Num $b 'native_device_cache_estimated_bytes'
   process_rss_bytes = Num $b 'rss_bytes_after_benchmark'
   visual_pixels_over8_percent = Num $b 'cache_visual_pixels_over8_percent'
   zoom = $zs
  }
 }
 $dia = $f.LastWriteTimeUtc.ToString('yyyy-MM-dd')
 $subdir = Join-Path $registros $dia
 New-Item -ItemType Directory -Path $subdir -Force | Out-Null
 $name = "$($type)_$($digest.Substring(0,16)).json"
 $destino = Join-Path $subdir $name
 if (-not (Test-Path -LiteralPath $destino)) { JsonSeguro $record $destino }
 Write-Host "Registro filtrado: $dia/$name" -ForegroundColor Green
}
function Validar-Registro($rec, [string]$nome) {
 $campos = @('schema','event_id','recorded_utc','version','type','status',
  'source_report_sha256','source_project_sha256_prefix','machine_commands_sent',
  'production_homologated','manual_validation_pending','metrics')
 foreach ($p in $rec.PSObject.Properties.Name) {
  if ($campos -notcontains $p) { throw "Campo nao permitido em $nome : $p" }
 }
 $tip = [string](Campo $rec 'type')
 if ((Campo $rec 'schema') -ne 'alencar-laser-audit-sanitized-v1' -or
     @('cache_zoom_validation','cache_ab','interaction_session') -notcontains $tip) {
  throw "Esquema inesperado em $nome"
 }
 $sha = [string](Campo $rec 'source_report_sha256')
 if ($sha -notmatch '^[a-f0-9]{64}$' -or
     (Campo $rec 'event_id') -ne $sha.Substring(0,16)) {
  throw "Identidade SHA incorreta em $nome"
 }
 $prefixo = Campo $rec 'source_project_sha256_prefix'
 if (($tip -ne 'interaction_session' -or $null -ne $prefixo) -and
     [string]$prefixo -notmatch '^[A-F0-9]{12}$') {
  throw "Identidade do projeto incorreta em $nome"
 }
 if ((Campo $rec 'version') -ne '19.9-experimental' -or
     [string](Campo $rec 'recorded_utc') -notmatch '^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z$' -or
     @('PASS','FAIL','PASS_AUTOMATED_MANUAL_PENDING','FAIL_AUTOMATED',
       'INCONCLUSIVE','UNVERIFIED','MANUAL_REVIEW_PENDING') -notcontains [string](Campo $rec 'status')) {
  throw "Metadados invalidos em $nome"
 }
 if ((Campo $rec 'production_homologated') -ne $false -or
     (Campo $rec 'manual_validation_pending') -ne $true -or
     (Campo $rec 'machine_commands_sent') -eq $true) {
  throw "Registro sem travas de producao em $nome"
 }
 $metricas = Campo $rec 'metrics'
 if ($null -eq $metricas) { throw "Sem metricas: $nome" }
 $permitidos = if ($tip -eq 'cache_ab') {
  @('contours','p95_uncached_ms','p95_cached_ms','reduction_percent',
    'cached_items','cache_estimated_bytes','process_rss_bytes',
    'visual_pixels_over8_percent','zoom')
 } elseif ($tip -eq 'interaction_session') {
  @('selection','drag','zoom_pan','scissors','undo_redo','tested_cases','failed_cases',
    'memory_sample_count','rss_p95_mb','rss_max_mb','cpu_p95_percent')
 } else { @('checks_failed_count') }
 foreach ($p in $metricas.PSObject.Properties.Name) {
  if ($permitidos -notcontains $p) { throw "Metrica inesperada em $nome : $p" }
  $v = Campo $metricas $p
  if ($p -eq 'zoom') {
   foreach ($z in @($v)) {
    if ($null -eq $z) { continue }
    $cols = @('factor','visible_contours','cache_items','p95_uncached_ms',
      'p95_cached_ms','visual_pixels_over8_percent')
    foreach ($zp in $z.PSObject.Properties.Name) {
     if ($cols -notcontains $zp) { throw "Campo zoom inesperado em $nome : $zp" }
     $n = Campo $z $zp
     if ($null -ne $n -and $n -isnot [ValueType]) {
      throw "Valor zoom nao numerico em $nome : $zp"
     }
    }
   }
  } elseif ($tip -eq 'interaction_session' -and
      @('selection','drag','zoom_pan','scissors','undo_redo') -contains $p) {
   if (@('PASS','FAIL','NOT_TESTED') -notcontains [string]$v) {
    throw "Resultado manual nao autorizado em $nome : $p"
   }
  } elseif ($null -ne $v -and $v -isnot [ValueType]) {
   throw "Metrica nao numerica em $nome : $p"
  }
 }
}
# Indice gerado apenas a partir dos registros com esquema conhecido.
$items = @()
$paths = @()
foreach ($f in @(Get-ChildItem -LiteralPath $registros -Recurse -File -Filter '*.json')) {
 $rec = Get-Content -LiteralPath $f.FullName -Raw | ConvertFrom-Json
 Validar-Registro $rec $f.Name
 $relative = ($f.FullName.Substring($root.Length + 1) -replace '\\','/')
 if ($relative -notmatch '^AUDITORIA_GITHUB/REGISTROS/\d{4}-\d{2}-\d{2}/(cache_zoom_validation|cache_ab|interaction_session)_[a-f0-9]{16}\.json$') {
  throw 'Caminho de registro fora do padrao. Envio suspenso.'
 }
 if ($f.BaseName -ne (([string]$rec.type) + '_' + ([string]$rec.event_id))) {
  throw 'Nome de arquivo nao corresponde ao conteudo. Envio suspenso.'
 }
 $paths += $relative
 $items += [pscustomobject][ordered]@{
  date_utc = [string]$rec.recorded_utc
  path = $relative
  event_id = [string]$rec.event_id
  type = [string]$rec.type
  status = [string]$rec.status
 }
}
$items = @($items | Sort-Object -Property date_utc, path -Descending)
$mostRecent = if ($items.Count -gt 0) { $items[0].date_utc } else { $null }
$index = [ordered]@{
 schema = 'alencar-laser-audit-index-v1'
 records_count = $items.Count
 latest_report_utc = $mostRecent
 records = $items
 note = 'Only allowlisted technical metrics; manual production validation is pending.'
}
$indexPath = Join-Path $base 'INDICE.json'
JsonSeguro $index $indexPath
Write-Host "INDICE preparado: $($items.Count) registros." -ForegroundColor Cyan
Write-Host 'Originais permanecem em RELATORIOS (ignorados pelo Git).'
if (-not $Publicar) {
 Write-Host 'SEM ENVIO: revise AUDITORIA_GITHUB e execute novamente com -Publicar para publicar.' -ForegroundColor Yellow
 return
}
# Publicacao opt-in: nunca inclui arquivos fora da lista restrita.
$stagedBefore = @(& git -C $root diff --cached --name-only)
if ($LASTEXITCODE -ne 0 -or $stagedBefore.Count -gt 0) {
 throw 'Ja ha arquivos staged no Git. Nao misturar commits de auditoria.'
}
$remote = (& git -C $root remote get-url --push origin) -join ''
if ($LASTEXITCODE -ne 0 -or $remote -notmatch '(^|[/:])alencarcell/alencar-laser(\.git)?$') {
 throw 'Remote origin nao corresponde ao repositorio autorizado.'
}
$allow = @($paths + @("$baseRel/INDICE.json"))
& git -C $root add -- @allow
if ($LASTEXITCODE -ne 0) { throw 'Falha ao preparar os registros para commit.' }
$stagedNow = @(& git -C $root diff --cached --name-only)
if ($LASTEXITCODE -ne 0) { throw 'Falha ao conferir staging.' }
foreach ($p in $stagedNow) {
 if ($allow -notcontains $p) { throw "Staging inesperado: $p. Envio suspenso." }
}
if ($stagedNow.Count -eq 0) {
 Write-Host 'Nenhum novo registro para publicar.' -ForegroundColor Green
 return
}
& git -C $root commit -m 'audit: registrar evidencias tecnicas filtradas' -- @stagedNow
if ($LASTEXITCODE -ne 0) { throw 'Commit falhou; nenhum push executado.' }
& git -C $root push origin "HEAD:refs/heads/$branchEsperada"
if ($LASTEXITCODE -ne 0) {
 throw 'Push falhou. Commit local preservado; confira autenticacao e sincronizacao.'
}
Write-Host "PUBLICADO somente $($stagedNow.Count) arquivo(s) de AUDITORIA_GITHUB na branch experimental." -ForegroundColor Green
