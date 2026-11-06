# Alencar Laser — Fase 20c, 06/11/2026

## Escopo concluído no GitHub
Criado `SCRIPTS/ANALISAR_FASE20C_COMPLEXIDADE.ps1` para inspecionar relatórios locais previamente gerados por `COLETAR_FASE20_VETORES_AUTO.ps1`. Sem alterações no renderizador, formato .alaser, seleção, Undo/Redo, comunicação física ou instalador.

## Execução Windows
Com PowerShell aberto em `C:\ProjetoAlencarLaser`:

```powershell
git pull --ff-only origin fix/curvas-tesoura-preflight-19-8-20261030
if ($LASTEXITCODE -ne 0) { throw 'Falha no git pull' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\ANALISAR_FASE20C_COMPLEXIDADE.ps1'
```

O script escolhe o último resumo FASE20 local ou recebe `-ResumoJson` explícito. Validadores bloqueiam relatórios sem seis rodadas OFF/ON/ON/OFF/OFF/ON, com probe ausente, ou que aleguem ganho/homologação/comandos de hardware. Mostra custos acumulados por faixa, custo médio por chamada e proporção de custo por rodada; os dados brutos ficam exclusivamente no computador.

## Limites
- Não executado no Windows neste commit; depende dos JSONs locais. O commit demonstra apenas autoria da ferramenta.
- A instrumentação pode alterar os tempos; não comparar diretamente com medições sem probe.
- ON3 com 575 contornos mantém política equivalente à OFF, portanto diferenças entre modos NÃO demonstram aceleração.
- Percentis de fases independentes não são somados; taxas são feitas exclusivamente com totais compatíveis.
- Não ativar cache, rasterização ou simplificação dos paths sem gate específico de equivalência visual, geometria, Undo/Redo e teste real.
- Branch experimental; main, APLICATIVO e laser preservados.

## Próximo gate
Após PASS local e revisão por faixa, desenhar uma prova de conceito **opt-in** que compare o mesmo desenho sob zoom, pan e arraste, verificando pixels, contornos, escala, Undo/Redo e CPU. Não promover à produção antes da avaliação manual.
