# Alencar Laser — Fase 20i: estresse de transformação com 575 contornos

Data: 06/11/2026. Branch experimental `fix/phase20h-transform-repaint-20261106`.

## Mudanças já publicadas
- Criado `groupedCenterTransformDense20iTest()` no Qt de laboratório: 575 retângulos vetoriais sintéticos, seleção total, movimento pelo centro com 12 eventos, coleta de P95/P99 de `paintEvent`, fração da caixa envolvente de pintura, hash SHA-256 dos pixels da prévia, preservação de geometria antes do commit e Undo/Redo.
- Novo modo: `--grouped-center-transform-575-20i-test`.
- Novo `SCRIPTS/TESTAR_TRANSFORM_575_20I_WINDOWS.ps1`: compilação Qt isolada, 3 pares por padrão com ordem OFF/ON alternada, comparação exata de hashes de pixel, validações de integridade e JSON local sem dados do projeto.

## Comando Windows
```powershell
cd C:\ProjetoAlencarLaser
git pull --ff-only origin fix/phase20h-transform-repaint-20261106
if ($LASTEXITCODE -ne 0) { throw 'Falha ao atualizar' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_TRANSFORM_575_20I_WINDOWS.ps1'
```

Laser desconectado. Fechar o editor de teste antes de executar. Se houver mudanças locais que impeçam o pull, não apagar/forçar.

## Critérios e ressalvas
**Ainda não executado nem compilado no Windows após estes commits.**
- Cada par precisa manter SHA-256 da prévia idêntico OFF e ON, geometria pré-commit idêntica, Undo/Redo, e flag de invalidação localizada ON sem fallback.
- Métricas P95/P99 devem ser comparadas sem atribuir automaticamente causalidade. Amostras por rodada podem ser baixas, e o teste Qt sintético não substitui arraste com mouse físico.
- Verificar regresso com suíte de 7 gates e projeto real em cópia, zoom/pan, DPI múltiplo e seleção de contornos densos antes da promoção da mudança.
- Não ativar `ALENCAR_TRANSFORM_LOCAL_REPAINT_20H` como padrão. Main, instalador, APLICATIVO e interface física preservados.
