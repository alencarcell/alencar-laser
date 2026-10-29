param(
 [Parameter(Mandatory=$true)][string]$Arquivo,
 [string]$Raiz = 'C:\ProjetoAlencarLaser'
)
$ErrorActionPreference = 'Stop'
if (-not (Test-Path -LiteralPath $Arquivo -PathType Leaf)) {
 throw "Arquivo LightBurn não encontrado: $Arquivo"
}
$item = Get-Item -LiteralPath $Arquivo
if ($item.Length -gt 67108864) { throw 'Arquivo maior que 64 MB; leitura bloqueada por segurança.' }
$settings = New-Object System.Xml.XmlReaderSettings
$settings.DtdProcessing = [System.Xml.DtdProcessing]::Prohibit
$settings.XmlResolver = $null
$settings.MaxCharactersInDocument = 67108864
$counts = @{}
$unknownElements = @{}
$root = ''
$format = ''
$shapeCount = 0
$reader = [System.Xml.XmlReader]::Create($item.FullName,$settings)
try {
 while ($reader.Read()) {
  if ($reader.NodeType -ne [System.Xml.XmlNodeType]::Element) { continue }
  if (-not $root) {
   $root = $reader.Name
   $format = $reader.GetAttribute('FormatVersion')
  }
  if ($reader.LocalName -eq 'Shape') {
   $type = $reader.GetAttribute('Type')
   if (-not $type) { $type = '(sem Type)' }
   if (-not $counts.ContainsKey($type)) { $counts[$type] = 0 }
   $counts[$type]++
   $shapeCount++
  } elseif ($reader.Depth -le 2) {
   $key = $reader.LocalName
   if (-not $unknownElements.ContainsKey($key)) { $unknownElements[$key] = 0 }
   $unknownElements[$key]++
  }
  if ($shapeCount -gt 100000) { throw 'Mais de 100 mil Shape: bloqueio preventivo.' }
 }
} finally { $reader.Dispose() }
$pasta = Join-Path $Raiz 'RELATORIOS\DIAGNOSTICOS'
New-Item -ItemType Directory -Path $pasta -Force | Out-Null
$saida = Join-Path $pasta ('RESUMO_LBRN2_' + (Get-Date -Format 'yyyyMMdd_HHmmss') + '.txt')
$linhas = New-Object System.Collections.Generic.List[string]
$linhas.Add('ALENCAR LASER — INVENTÁRIO SEGURO DE FORMATOS LBRN2')
$linhas.Add('Projeto: não registrado no relatório')
$linhas.Add('XML raiz: ' + $root)
$linhas.Add('FormatVersion: ' + $format)
$linhas.Add('Total de Shape: ' + $shapeCount)
$linhas.Add('Tipos de formas:')
foreach ($key in @($counts.Keys | Sort-Object)) { $linhas.Add('  ' + $key + ': ' + $counts[$key]) }
$linhas.Add('Elementos XML de nível alto:')
foreach ($key in @($unknownElements.Keys | Sort-Object)) { $linhas.Add('  ' + $key + ': ' + $unknownElements[$key]) }
$linhas.Add('Dados de desenho, coordenadas, nomes de textos e parâmetros de gravação NÃO incluídos.')
$linhas | Set-Content -LiteralPath $saida -Encoding UTF8
Write-Host "Relatorio sem geometria pessoal: $saida" -ForegroundColor Green
