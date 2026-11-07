# Alencar Laser — Fase 20k: experimento de repintura após zoom

Data: 07/11/2026. Branch: `exp/phase20k-zoom-repaint-20261107`, derivada de `test/phase20j-event-paint-trace-20261107`.

## Evidência da sessão Windows fornecida pelo operador

Documento: 575 contornos, 13.415 nós; 624 paints. P95 global de pintura: 156,4005 ms. Por evento recente, zoom: 154 amostras, P95 182,5478 ms, máximo 205,813 ms; transformação: 400 amostras, P95 32,9149 ms. Foreground P95 2,3771 ms e prévia da transformação P95 1,2452 ms. Telemetria atribui paint ao último input em 250 ms, logo a categoria NÃO prova causalidade. A categoria idle tem 10 amostras e não deve ser tratada como perfil estável de repouso.

## Implementação experimental

`QtEditorLab::wheelEvent` já modifica escala e scrollbars, e depois chama `viewport()->update()` sem condição. Adicionada a flag **somente opt-in** `ALENCAR_SKIP_WHEEL_FULL_UPDATE_20K=1`, que omite **apenas** essa chamada extra. Não muda cálculo do zoom, geometria, exportação, histórico, desenho, pintura Qt normal ou comandos laser. Sem a flag o código mantém exatamente a operação anterior.

Criado modo de teste `--wheel-zoom-dense-575-20k-test`: 575 objetos, 12 eventos alternados de zoom, hash SHA-256 de 12 imagens de viewport, P95/P99 de pintura, bbox de repaint, comparação de geometria antes/depois e histórico Undo. Nenhuma imagem ou geometria é publicada.

`SCRIPTS/TESTAR_ZOOM_575_20K_WINDOWS.ps1` compila Qt num build separado e executa 3 pares OFF/ON (ordem alternada), verifica hashes exatos, bloqueia alterações do modelo e grava relatório local em `RELATORIOS/AUDITORIAS/GITHUB_19_8/ZOOM_AB_20K`. Se todos os pares ON forem piores, marca regressão. Mesmo com melhora, NÃO promove automaticamente.

## Execução no Windows

Feche o editor de laboratório e mantenha fisicamente o laser desconectado.

```powershell
cd C:\ProjetoAlencarLaser
git fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Falha fetch' }
git switch exp/phase20k-zoom-repaint-20261107
if ($LASTEXITCODE -ne 0) { throw 'Falha branch; não use force/reset/clean' }
git pull --ff-only origin exp/phase20k-zoom-repaint-20261107
if ($LASTEXITCODE -ne 0) { throw 'Falha pull' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_ZOOM_575_20K_WINDOWS.ps1'
```

## Limites

Ainda **não compilado nem executado no Windows** após o commit. O zoom sintético (sem mouse físico ou trackpad) pode diferir do real, e o próprio screenshot de cada etapa contribui para o custo de pintura. P95 por rodada tem amostragem pequena e pode variar por carga. Igualdade de screenshots em Qt offscreen é condição necessária, mas não suficiente para homologar DPI/zoom real e preview de ferramentas. Se o experimento mostrar vantagem, repetir em sessões reais do Windows, com DPI diferentes, regressão 7/7 e arquivos reais antes de ativar por padrão.

A flag 20h de repaint localizado das transformações continua OFF pois regrediu nos 3 pares anteriores. Main, instalador e controle físico permanecem inalterados.
