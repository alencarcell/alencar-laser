# Fase 20m — correção Ctrl+D múltiplos objetos (07/11/2026)

## Evidência da falha
O operador informou que os `command_error` surgiram ao usar Ctrl+D com muitos objetos. No arquivo da sessão ON fornecida, há 8 `command_error`; o relatório não exporta os detalhes de cada exceção. A inspeção do código confirmou que o botão `duplicate_obj` exigia `ids.size()==1` e o atalho Ctrl+D acionava esse botão. Portanto, a hipótese é fortemente sustentada por fonte e relato, mas só o teste Windows validará o reparo.

## Correção publicada
Branch `fix/phase20m-bulk-duplicate-20261107`, derivada da candidata de viewport 20l:
- Editor::duplicateObjects(ids, {3,3}) faz cópia integral e atômica do estado em **um único registro Undo**, com IDs de objeto e nós novos.
- Expande seleção parcial de grupos e de texto vetorial editável; preserva camadas e remapeia grupos, IDs de membros e metadados de texto.
- Rejeita camada bloqueada/oculta, origem inexistente e mais de 2000 objetos em uma única operação (limite conservador para evitar consumo de memória exagerado).
- Ctrl+D, botão de duplicar e menu de contexto agora aceitam seleção de várias formas. Duplicados ficam selecionados e deslocados +3 mm.
- Não altera a estratégia opt-in de viewport, a geometria original, o arquivo .alaser v5, o instalador nem comandos físicos do laser.

## Gates preparados e ainda pendentes no Windows
- `alencar_test_bulk_duplicate_20m` testa 575 objetos, geometria, contagem de nós, IDs novos, grupos, proteção de camadas, texto editável, Undo e Redo.
- `--bulk-duplicate-shortcut-575-20m-test` abre uma janela Qt offscreen, simula Ctrl+D com 575 objetos e verifica o ciclo Undo/Redo.
- `SCRIPTS/TESTAR_DUPLICACAO_MULTIPLA_20M_WINDOWS.ps1` compila ambos os alvos numa pasta de build separada, executa e gera JSON local. **Não executado no Windows nesta etapa**, não alegar reparo homologado.

## Comando
```powershell
cd C:\ProjetoAlencarLaser
git fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Falha ao atualizar refs' }
git switch fix/phase20m-bulk-duplicate-20261107
if ($LASTEXITCODE -ne 0) { throw 'Branch bloqueada. Nao use reset, clean ou force' }
git pull --ff-only origin fix/phase20m-bulk-duplicate-20261107
if ($LASTEXITCODE -ne 0) { throw 'Falha na atualizacao' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_DUPLICACAO_MULTIPLA_20M_WINDOWS.ps1'
```

## Sobre a comparação de desempenho anterior
O relatório da sessão OFF terminou em 141 contornos, 37.365 nós, P95 global 221,815 ms. A sessão ON terminou em 573 contornos, 13.409 nós, P95 global 38,8759 ms; zoom P95 OFF 172,2923 ms, ON 36,1456 ms. Como cargas e operações são distintas, **não** promover ganho percentual causal. O indicador ON justifica nova sessão pareada com mesma geometria, enquanto a correção Ctrl+D merece prioridade por ser um defeito funcional reproduzível em código.

## Saída
Somente com gates verdes e teste manual de duplicação mista (um, muitos, grupos, camadas e texto) poderá se integrar ao candidato offline. Repetir os 7 gates completos antes do instalador. Laser fisicamente desconectado.
