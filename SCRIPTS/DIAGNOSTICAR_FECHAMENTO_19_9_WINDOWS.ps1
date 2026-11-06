#requires -Version 5.1
<#
Diagnostico nao destrutivo do fechamento do editor Alencar Laser 19.9.
Captura stack C++ se MSYS2/UCRT64 gdb estiver disponivel.
Nao instala dependencias, altera QSettings ou envia comandos a laser.
A sessao pode abrir a janela: reproduza o problema e feche normalmente se funcionar.
Para parar apenas em falha fatal: -CapturarEncerramento
O antigo -CapturarExcecao e aceito como alias, mas nao para em excecoes tratadas.
#>
[CmdletBinding()]
param(
 # Stops only at std::terminate/abort instead of first (possibly caught) throw.
 [switch]$CapturarEncerramento,
 # Deprecated for safety: same behavior as CapturarEncerramento.
 [switch]$CapturarExcecao
)
$ErrorActionPreference = "Stop"
$root = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$ucrt = 'C:\msys64\ucrt64\bin'
$gdb = Join-Path $ucrt 'gdb.exe'
$program = Join-Path $root 'DESENVOLVIMENTO\FONTE\build_WINDOWS_GIT_19_8\alencar_test7_qt6.exe'
$fixture = Join-Path $root 'DESENVOLVIMENTO\FONTE\fixtures\web_nodes_normalized.csv'
$session = Join-Path $root 'RELATORIOS\AUDITORIAS\GITHUB_19_8\diagnostico_inicio'
$logs = Join-Path $root 'RELATORIOS\DIAGNOSTICOS'
$log = Join-Path $logs ("FECHAMENTO_GDB_19_9_{0}.txt" -f (Get-Date -Format 'yyyyMMdd_HHmmss'))
if (!(Test-Path -LiteralPath $program -PathType Leaf)) { throw "Executavel ausente: $program" }
if (!(Test-Path -LiteralPath $fixture -PathType Leaf)) { throw "Fixture ausente: $fixture" }
if (!(Test-Path -LiteralPath $gdb -PathType Leaf)) {
 Write-Warning "Depurador ainda nao instalado: $gdb"
 Write-Host "Abra o terminal MSYS2 UCRT64 e instale com: pacman -S mingw-w64-ucrt-x86_64-gdb"
 Write-Host "O Alencar Laser e seus projetos NAO foram alterados."
 return
}
[void](New-Item -Path $logs -ItemType Directory -Force)
[void](New-Item -Path $session -ItemType Directory -Force)
$oldPath = $env:PATH
$oldDir = (Get-Location).Path
try {
 $env:PATH = "$ucrt;$oldPath"
 Set-Location -LiteralPath $root
 Write-Host "Reproduza o encerramento da janela Alencar Laser. Nenhuma emissao laser sera habilitada." -ForegroundColor Yellow
 Write-Host "Captura GDB: $log"
 $fatalMode = $CapturarEncerramento -or $CapturarExcecao
 if ($CapturarExcecao) {
    Write-Warning "CapturarExcecao agora observa encerramentos fatais; o antigo catch throw interrompia ate erros tratados pelo importador."
 }
 if ($fatalMode) {
    Write-Host "Captura de std::terminate / abort: ativa. Excecoes tratadas normalmente nao interrompem o programa." -ForegroundColor Yellow
 }
 # PowerShell 5.1 promotes native stderr to NativeCommandError under
 # ErrorActionPreference=Stop. GDB may emit harmless auto-load warnings on
 # stderr; capture stdout/stderr as OS file redirects without pipelines.
 $stdoutFile = [System.IO.Path]::ChangeExtension($log, '.stdout.tmp')
 $stderrFile = [System.IO.Path]::ChangeExtension($log, '.stderr.tmp')
 $quote = { param([string]$v) '"' + $v.Replace('"','\\"') + '"' }

 # Stop on uncaught failures only. GDB's catch-throw type filtering can be
 # unsupported in MSYS2, and even a *handled* import error will trip it.
 # A normal user-driven exit produces no backtrace and is not a crash PASS.
 # Keep the original generic diagnostic when no mode is selected.
 $commandSequence = @(
     'set pagination off',
     'set confirm off',
     'set print thread-events off',
     'set auto-load python-scripts off'
 )
 if ($fatalMode) {
     $commandSequence += @(
         'set breakpoint pending on',
         'break std::terminate',
         'break abort'
     )
 }
 $commandSequence += @('run','bt 45','thread apply all bt 15')
 $parts = @('--quiet','--batch','--nx')
 foreach ($command in $commandSequence) {
     $parts += '-ex'
     $parts += (& $quote $command)
 }
 $parts += @('--args',(& $quote $program),'--interactive',(& $quote $fixture),(& $quote $session))
 $gdbArgs = $parts -join ' '
 $proc = Start-Process -FilePath $gdb -ArgumentList $gdbArgs `
   -NoNewWindow -Wait -PassThru `
   -RedirectStandardOutput $stdoutFile -RedirectStandardError $stderrFile
 $stdoutText = if (Test-Path -LiteralPath $stdoutFile) { Get-Content -LiteralPath $stdoutFile -Raw } else { '' }
 $stderrText = if (Test-Path -LiteralPath $stderrFile) { Get-Content -LiteralPath $stderrFile -Raw } else { '' }
 $header = "Alencar Laser 19.9 | GDB exit code: $($proc.ExitCode)`r`n"
 $reportText = $header + "[STDOUT / BACKTRACE]`r`n" + $stdoutText + "`r`n[STDERR / WARNINGS]`r`n" + $stderrText
 [System.IO.File]::WriteAllText($log, $reportText, (New-Object System.Text.UTF8Encoding($false)))
 Remove-Item -LiteralPath $stdoutFile,$stderrFile -ErrorAction SilentlyContinue
 Write-Host "Relatorio: $log" -ForegroundColor Cyan
 Write-Host "GDB encerrou com codigo: $($proc.ExitCode)"
 if ($fatalMode) {
     if ($reportText -match '(?im)^Breakpoint\s+\d+,|Program received signal SIG(ABRT|SEGV|ILL|BUS|FPE)') {
         Write-Warning 'Interrupcao fatal possivel. Examine o backtrace e nao trate este evento como importacao normal.'
     } elseif ($reportText -match '(?i)exited normally') {
         Write-Host 'O processo terminou normalmente nesta sessao; nenhuma falha fatal foi capturada.'
     } else {
         Write-Warning 'Nenhuma parada fatal confirmada. Confira o backtrace e possiveis avisos de breakpoint.'
     }
 }
 Write-Host "Envie o TXT mesmo se aparecerem avisos do GDB."
}
finally {
 $env:PATH = $oldPath
 Set-Location -LiteralPath $oldDir
}
