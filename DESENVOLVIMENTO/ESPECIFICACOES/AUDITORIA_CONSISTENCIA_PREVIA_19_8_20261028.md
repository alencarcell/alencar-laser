# Alencar Laser Windows 19.8 — Auditoria de consistência da prévia

**Data:** 28/10/2026  
**Branch:** `feat/refino-curvas-rastreio-diagnosticos-19-8`  
**Escopo:** correções preventivas no motor de simulação e testes, mantendo `main`, a base aprovada, `APLICATIVO` e todo acesso físico ao laser intocados.

## Problemas encontrados e corrigidos

### 1. Tempo nominal diferente na interface e no JSON

**Antes:** a janela apresentava um tempo nominal somando os segmentos ativos na velocidade configurada por camada. Porém, o botão "Exportar relatório JSON" recalculava o tempo por `minutes(plan, feedGlobal, rapidGlobal)`, desconsiderando a velocidade individual. Isso gerava dois tempos diferentes para o mesmo plano.

**Agora:**
- `src/preview_motion_19_8.hpp` disponibiliza `nominalSecondsWithFeeds`, cálculo em segundos que usa velocidade de cada camada nos trechos ativos e velocidade rápida nos deslocamentos inativos.
- `qt/preview_dialog_19_8.hpp` guarda o resultado do mesmo cálculo em `nominalSeconds` e o utiliza **tanto na tela quanto na exportação JSON**. Limpa o valor se o cálculo falhar.
- `tests/preview_motion_19_8_standalone.cpp` verifica plano de 100 mm ativo e 100 mm de viagem, com feed por camada de 6000 versus 3000 mm/min (2 s e 3 s), além de rejeitar velocidade zero.

**Limites:** nominal não significa cronograma real; aceleração, CAM e GRBL permanecem fora dessa métrica.

### 2. Intervalo de varredura alterado silenciosamente ao redimensionar a imagem

**Antes:** `appendMask` dividia a altura da imagem pelo número `ceil(altura/intervalo)` de linhas. Uma imagem de 21 mm com intervalo solicitado de 10 mm resultava em três linhas separadas por **7 mm**, não 10 mm.

**Agora:**
- `src/preview_raster_19_8.hpp` posiciona os centros das linhas pelo intervalo físico pedido (`(linha+0,5) × intervalo`), em vez de redistribuir o espaço. A linha seleciona pixels pelo `y` físico relativo da imagem.
- Para uma imagem menor que metade do espaçamento, usa uma única linha virtual central.
- Testes `preview_raster_19_8_standalone.cpp` e `qt6_test6.cpp --simulation-raster-19-8-test`: máscara preta com 21 mm de altura e 10 mm de passo deve ter linhas em 5 e 15 mm; imagem de 4 mm deve gerar uma linha central em 2 mm.

**Limites:** é amostragem P&B aproximada. A prévia não usa grayscale por potência nem substitui o planejador do LightBurn.

### 3. Risco de varredura Fill muito demorada

**Antes:** o Fill limitava o total de linhas, mas um desenho com centenas de contornos densos ainda podia gerar milhões de testes aresta × linha, executados durante a atualização da janela.

**Agora:**
- `appendFill` estima previamente o custo de interseções e bloqueia o processamento se ultrapassar **6 milhões de verificações geométricas**, acumuladas entre camadas.
- Esse limite se soma aos existentes de 1500 linhas por camada e 250.000 segmentos virtuais no raster.
- O teste C++ portátil cria dois contornos densos de 4096 nós, com 1000 linhas de varredura, e espera rejeição explícita antes de começar a análise cara.
- A janela mostra a mensagem de limite da prévia; não altera projeto nem dá comandos.

## Matriz de validação

| Item | Estado |
|---|---|
| Código das três correções | Publicado na branch experimental |
| Testes portáteis C++20 de nominal, raster e proteção Fill | Atualizados, execução desta rodada não confirmada |
| Regressão Qt de espaçamento físico | Publicada, execução Windows pendente |
| GitHub Actions | Workflow existente; execução e PASS ainda não confirmados |
| Compilação completa Qt6 Windows após as correções | Pendente |
| Testes físicos laser e G-code | Não realizados e não autorizados |

Apenas inspecionar diffs, arquivos e integração **não demonstra PASS** de compilação. Não declarar candidata 19.8 homologada até haver logs e provas visuais dos testes no Windows.

## Próxima ação local segura

```powershell
cd C:\ProjetoAlencarLaser
git status --short
git pull --ff-only origin feat/refino-curvas-rastreio-diagnosticos-19-8
if ($LASTEXITCODE -ne 0) { throw "Git interrompido; nao substituir arquivos locais" }
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteSimulacao
```

Se passar, executar `-SomenteInteracao` e `-Abrir` com a máquina desconectada. Validar exportação JSON, alternância das velocidades de camada, simulação Fill/Imagem e a nova opção "internos primeiro". Manter `.vscode/` local preservado.

**Critério de segurança:** até nova autorização com ensaio de segurança, comandos M3/M4 e controle físico permanecem apenas nomes/metadados na 19.8.
