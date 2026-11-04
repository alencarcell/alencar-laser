# Alencar Laser 19.9 — QA manual do cache Bézier
Data: 04/11/2026
Escopo: branch experimental `fix/curvas-tesoura-preflight-19-8-20261030`.
Estado: PENDENTE DE EXECUCAO MANUAL; NAO HOMOLOGADO.

## Evidencia automatizada recebida
- `VALIDAR_CACHE_ZOOM_19_9.ps1`: `PASS_AUTOMATED_MANUAL_PENDING` no Windows (captura do operador).
- Relatorio local: `RELATORIOS/AUDITORIAS/GITHUB_19_8/BEZIER_PERFORMANCE_19_9/AUDITORIA_CACHE_ZOOM_19_9_20261104_172740.json`.
- Cache A/B de 2.134 contornos, SHA da origem preservado, P95 sem cache 553,0985 ms e com cache 41,522 ms (ganho reportado 92,49%).
- Curvas visiveis nos zooms 1x, 2x e 4x; pixels com divergencia > 8 niveis/canal = 0%.
- Esta medicao automatizada NAO demonstra automaticamente comportamento do mouse real.

## Preflight obrigatorio
1. Manter laser fisicamente desconectado; nao testar GRBL, G-code, emissao, homing ou movimento.
2. Confirmar branch experimental; executar `git status --short`; preservar arquivos locais nao rastreados.
3. Usar COPIA de um arquivo `.lbrn2` de referencia; registrar SHA-256 antes e depois.
4. Nao usar o expositor com grupos para simular compatibilidade ainda nao implementada.
5. Deixar cache do aplicativo de producao DESLIGADO; benchmarking opt-in nao autoriza promocao.

## Testes manuais com mouse fisico
Preencher cada linha com PASS / FAIL / N/A, evidencias e observacoes. Sem evidencia, registrar PENDENTE.

| ID | Acao | Criterio esperado | Resultado |
| --- | --- | --- | --- |
| M01 | Abrir a copia de projeto e ajustar desenho a tela | Todas as curvas sem perdas ou artefatos | PENDENTE |
| M02 | Selecionar e desselecionar repetidamente curvas pequenas e grandes | Sem travamento, curvas e handles inteiros, nenhuma imagem fantasma | PENDENTE |
| M03 | Arrastar uma curva, desfazer e refazer | Posicao final/retorno corretos e sem raster residual | PENDENTE |
| M04 | Zoom pela roda, entrando e saindo entre escalas baixas e altas | Tracos nitidos, sem sumicos ou borramento temporario persistente | PENDENTE |
| M05 | Pan horizontal e vertical depois do zoom | Sem residuos na viewport e sem selecao deslocada | PENDENTE |
| M06 | Tesoura: hover sobre trecho valido e fora dele | Cursor e destaque corretos; fora do trecho nenhum preview fantasma | PENDENTE |
| M07 | Tesoura: cortar arco de curva densa e usar Ctrl+Z/Ctrl+Y | Trecho completo, sem fragmentos ou alteracao em objetos adjacentes | PENDENTE |
| M08 | Selecionar contornos sobrepostos e alternar ferramentas | Hit-test corresponde ao desenho, sem congelamento | PENDENTE |
| M09 | Fechar/reabrir copia de trabalho (quando suportado) | Arquivo coerente, camadas e curvas preservadas | PENDENTE |
| M10 | Repetir com segundo desenho compativel de outra topologia | Sem regressao ou extrapolacao de um unico projeto | PENDENTE |

## Condicoes para avaliacao
- Repetir A/B de mesmo SHA em pelo menos 3 pares; guardar tempos P95 e RSS de cada par.
- Diferenciar resultado de benchmark do comportamento da interface: cache pode ser rapido e ainda gerar defeitos de selecao/zoom.
- Se ocorrer FAIL: manter modo OFF, guardar apenas diagnosticos sem dados pessoais e informar passo reproduzivel, versao, zoom e ferramenta.
- Antes de qualquer producao: compilacao completa Windows, regressao Qt, verificacao manual e revisao de consumo de memoria.
- Nao alterar main, APLICATIVO, instalador, projetos originais ou hardware durante esta fase.

## Registro da rodada
- Data e build/commit:
- Arquivo de referencia (apenas SHA, nao conteudo):
- Qt / Windows / resolucao / escala de tela:
- Resultado M01..M10:
- P95 sem cache / com cache (3 pares):
- RSS:
- Falhas reproduziveis:
- Decisao final: PENDENTE (nunca homologar automaticamente)
