# ALENCAR LASER — PRIORIDADE ABSOLUTA: FLUIDEZ E ESTABILIDADE

Data: 02/11/2026
Branch experimental: `fix/curvas-tesoura-preflight-19-8-20261030` (PR #13, draft)
Versao aprovada em `main` e pasta `APLICATIVO`: NAO MODIFICAR

## Decisao do proprietario
Interromper a expansao de recursos e concentrar as proximas intervencoes na fluidez e confiabilidade do editor Windows Qt 6. Prioridade de engenharia acima de acabamento estetico, novas ferramentas, mecanismos, novos importadores e habilitacao de hardware. Nenhuma tarefa em segundo plano esta presumida; executar em sessoes explicitamente iniciadas e registrar resultado.

## Problema reproduzido em Windows
- Projeto denso: aproximadamente 417 contornos e 122.390 nos (ou versao editada com 415 contornos e 120.081 nos).
- Captura real: Windows exibiu `Nao esta respondendo` durante operacoes de edicao.
- Baseline local mais recente: paint P50=73,8873 ms, P95=1062,0717 ms, P99=1125,2906 ms, max=1615,7267 ms; hit-test P95=159,6687 ms; memoria residente ~297 MB.
- Outra sessao: P95=1178 ms e max=8687 ms, sem teste A/B controlado. Nao atribuir causalidade apenas a uma alteracao.
- Acoes observadas: selecao de ~417 objetos, movimentacao de 417, ferramenta Tesoura, Undo e troca de camada. Arquivos de diagnostico preservados pelo usuario.

## Estado de codigo auditado
- `QtEditorLab::rebuild()` executa `scene_.clear()`, recria caminhos de contornos e reconstrói indices/objetos; custo potencial elevado quando acionado durante edicao.
- `QtEditorLab::applySelectionStyles()` percorre itens alterados e usa `setPen/setZValue`, causando invalidacoes de pintura na selecao em massa.
- `picker_.query(...)` mede tempo de hit-test; precisa de instrumentacao por fase e por gesto para identificar tempo gasto na consulta ou no preparo do indice.
- O commit `7ae2ef9` inclui modo visual rapido para mais de 80 mil nos, `BoundingRectViewportUpdate` e lote de estilo com mais de 31 objetos. Politica portatil validada; **Qt Windows / projeto real ainda nao homologados**.

## Sequencia de trabalho vinculante

### P0 — Medir antes de alterar mais
1. Compilar `7ae2ef9` no Windows via `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -Abrir` e preservar o stash anterior.
2. Instrumentar tempos separados: carregar e decodificar, `rebuild()`, `pathOf`, `QGraphicsView::paintEvent`, selecao/estilos, `picker_.query`, Tesoura e Undo; exportar contagens sem coordenadas nem dados privados.
3. Reproduzir com copia do projeto de 120 mil nos, mantendo resolucao/zoom/DPI e gestos fixos. Repetir 3 vezes, comparar mediana e P95/P99, memoria e responsividade da janela.
4. Validar se ha novos artefatos graficos, linhas desaparecidas, objetos fantasma ou selecao incorreta. Qualquer regressao grave => reversao isolada do experimento.

### P1 — Eliminar refabricacao redundante de geometria
- Perfil do uso de `rebuild()`; avaliar atualizacao incremental apenas dos `Key`/contornos modificados.
- Cache `QPainterPath` por revisao estavel de geometria, com invalidacao correta em editar, mover, excluir, Undo/Redo, trocar camada, importar e abrir.
- Manter documento e geometria canonicos intocados; nunca usar caminhos reduzidos como fonte para salvar, gerar G-code, exportar ou acertar selecao.
- Qualquer aproximacao ou nivel de detalhe por zoom sera **apenas visual**, opt-in ou reversivel, com avaliacao pixel/visual.

### P2 — Selecoes grandes e edicao responsiva
- Minimizar `setPen/setZValue` e atualizacoes da viewport em selecoes de centenas de itens.
- Verificar se `NoViewportUpdate` sempre retorna ao estado anterior em todas as saidas/erros.
- Evitar gerar snapshots completos de ~120 mil nos por evento de mouse; criar historico por gesto/operacao atomica (sem alterar a semantica de Undo).
- Otimizar hover da Tesoura e hit-test com indice valido e consultas proporcionais apenas aos candidatos.

### P3 — Controle de qualidade e metas
- Benchmarks repetiveis: ~13.409 e ~120.081 nos; selecionar 1 e 417 objetos; zoom/pan; mover; Tesoura; desfazer/refazer; salvar e reabrir .alaser v5.
- Alvos de engenharia, nao resultados: diminuir o paint P95 em pelo menos 50% frente ao baseline comparavel, reduzir o hit-test P95 em pelo menos 50% e eliminar janelas `Nao esta respondendo` no roteiro de testes.
- Preservar contagem/coordenadas dos nos e a fidelidade geometrica, arquivos, grupos, camadas, simetria visual e reversibilidade.
- CI GitHub pode estar limitado; testar localmente sem afirmar PASS na nuvem. Cada etapa deve registrar: arquivo alterado, commit, teste efetuado, resultados medidos e risco residual.

## Bloqueios ate a estabilidade
- Nao implementar novas funcionalidades nao relacionadas a fluidez.
- Nao realizar merge na `main`, substituir `APLICATIVO`, publicar instalador homologado ou sobrescrever projetos de origem.
- Nao conectar a controladora para testes do editor, nao transmitir comandos de movimento, homing, laser ou G-code; diagnostico fisico tera gate separado.
- Nao prometer melhoria sem medicao no Windows.

## Estado de retomada
Ultima revisao publicada antes deste plano: `7ae2ef9`.
A proxima acao nao e redesenhar icones nem ampliar LightBurn: e medir os gargalos do Qt com os projetos do usuario, depois corrigir `rebuild()`, pintura e hit-test conforme evidencias.


## Atualizacao 03/11/2026 — Arraste agrupado de selecao numerosa
- **Novo codigo experimental Qt:** para 32 ou mais itens de contorno no arraste de selecao, o `QtEditorLab` usa `QGraphicsScene::createItemGroup` temporariamente e move somente o pai via `setPos` em cada movimento do mouse, em vez de iterar por centenas de `QGraphicsPathItem`. Selecao menor segue caminho antigo. Durante a previa, a geometria do `EditorState` nao muda.
- Ao soltar/cancelar, o grupo e zerado e destruido com `destroyItemGroup`, seus itens retornam ao pai original sem translacao; a operacao canonica continua com `editor.moveObjects` uma unica vez.
- Contadores novos no JSON: `grouped_drag_starts` e `grouped_drag_frame_updates`; nao representam ganho de velocidade, somente exercicio desse caminho.
- **Regressao GUI nova:** `--grouped-drag-19-9-test` verifica 40 objetos, agrupamento transitorio, ausencia de transformacao residual, commit geometrico exato, Undo e Redo. `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteFluidez` compila apenas o necessario e roda esse teste + a regressao do historico; a suite integral tambem o invoca.
- Commits: `4179bfe`, `42ad599`, `f8f6d51`, `feec727`. **Sem Qt Windows PASS ate nova execucao**; nao homologar antes de compilacao e teste real.
- Comparar no Windows com o mesmo arquivo de ~122.390 nos: selecionar 417, arrastar repetidamente, observar responsividade, `paint_p95_ms`, `hit_test_p95_ms`, `rebuild_p95_ms`, memoria e novos contadores. Comparar com baseline anterior em igualdade de zoom e gestos.
- Risco sob avaliacao: agrupamento temporario altera a arvore de itens somente durante a previa. Por isso a regressao exige zero pais ou posicao residual e renderizacao exata apos soltar e Undo/Redo. Se falhar, reverter a mudanca de grupo na branch experimental. Nao mexer em `main`, `APLICATIVO` ou no hardware.
