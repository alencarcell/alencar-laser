# Alencar Laser 19.9 — Instrumentação RAM/CPU e histórico eficiente
Data: 02/11/2026 · branch experimental PR #13

## Mudanças implementadas
- `Editor::change`: mantém uma única cópia mutável do estado e retém o estado anterior no histórico por movimento, eliminando a segunda cópia profunda que antes ocorria em `past.push_back(current)`.
- `Editor::undo/redo`: transfere o estado atual para a pilha oposta por movimento. Nenhum comando, geometria, ID ou arquivo é alterado.
- O limite de **40 estados de desfazer** permanece inalterado. **IMPORTANTE**: memória permanente do histórico ainda é proporcional a nós × estados. Não afirmar que todo o problema de RAM está resolvido.
- Nova métrica `historyNodeFootprint()` é somente leitura e varre as capacidades dos vetores de nós no momento de exportação do diagnóstico. O campo `history_retained_node_bytes_lower_bound` é um **limite inferior aproximado**, não o tamanho total da memória residente.
- `MEDIR_MEMORIA_WINDOWS_19_9.ps1` amostra Working Set, Private Bytes e CPU de um processo identificado; não abre laser, não lê desenhos e grava CSV local.
- Teste portátil `alencar_test19_9_history_memory`: valida histórico, desfazer/refazer, limite de 40 e métricas de nós.

## Como medir
1. Abrir a versão experimental pelo procedimento de compilação/validação existente, mantendo máquina desconectada.
2. Com o editor aberto, executar:
```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\MEDIR_MEMORIA_WINDOWS_19_9.ps1" -DuracaoSegundos 180
```
3. Usar **cópia** dos projetos de 13 mil e de ~120 mil nós: abrir, selecionar 1 e muitos objetos, arrastar, Undo/Redo, salvar/reabrir.
4. Exportar o diagnóstico JSON do próprio aplicativo, e comparar CPU/RAM com versões anteriores usando o mesmo roteiro.
5. Para o teste portátil:
```powershell
cmake --build .\DESENVOLVIMENTO\FONTE\build_WINDOWS_GIT_19_8 --target alencar_test19_9_history_memory
.\DESENVOLVIMENTO\FONTE\build_WINDOWS_GIT_19_8\alencar_test19_9_history_memory.exe
```
O executável pode estar em subdiretório, dependendo do gerador CMake.

## Gates de segurança
- Não comprometer Undo/Redo, backup .alaser v5, recuperação e versão aprovada.
- Manter `main`, `APLICATIVO` e G-code físico bloqueados.
- Qt Windows e comparação em projeto de 120 mil nós **AINDA SEM RESULTADO** desta revisão. A compilação local deve acontecer antes de afirmar PASS.
- Se houver regressões na movimentação de estados, reverter a mudança e restaurar commits da branch experimental.
