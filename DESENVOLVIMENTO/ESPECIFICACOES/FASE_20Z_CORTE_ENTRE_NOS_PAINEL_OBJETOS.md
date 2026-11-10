# Alencar Laser — Fase 20z | Corte entre nós e painel Objetos

Data 2026-11-10. Branch experimental: `feat/corte-entre-nos-painel-objetos-20z-20261110`, baseada em `fix/tesoura-cortes-sucessivos-20y-20261110`.

## Motivação de bancada
Usuário observou que a aba **Objetos** mostrava dimensões da mesa, escala, rotação e alinhamento mesmo quando havia apenas uma forma selecionada (ações de alinhamento recusadas). Também não havia uma ferramenta acessível para **apagar somente uma aresta entre dois nós consecutivos**, sem precisar de interseção. A função de núcleo `Editor::removePolylineEdge` já existia, mas estava acessível principalmente pelo menu de contexto da forma ativa.

## Implementado
- Nova ferramenta **Entre nós**, modo `Tool::EdgeTrim` e atalho **J**, botão fixo na barra e item do menu Editar/menu de contexto.
- Hover marca em **ciano apenas UMA aresta inteira entre os dois nós canônicos adjacentes**, não o intervalo entre cruzamentos. Clique aplica exatamente `removePolylineEdge(key, edge)`.
- A edição não requer seleção prévia ou interseções. Respeita camada editável e grupos; nunca emite laser e não altera outras figuras.
- Contorno fechado: remove a aresta e abre o caminho, preservando as restantes. Contorno aberto: pode gerar dois restos independentes; Ctrl+Z é uma única transação.
- Prévia armazenada somente enquanto a posição e geração da geometria não mudaram; alterações ou troca de ferramenta invalidam o destaque. Antes de um rebuild, a prévia é excluída da cena para evitar ponteiros suspensos.
- Mantida a Tesoura clássica **K** para intervalos delimitados por cruzamentos, inclusive curvas Bézier nativas.
- Aba **Objetos** remove controles redundantes de mesa (já disponíveis acima do desenho) e mostra objeto selecionado, camada, agrupamento, quantidade de nós e aberto/fechado. Inclui acesso direto ao modo Entre nós.
- Rotação/escala permanecem disponíveis quando há seleção; alinhamentos somente quando há **2 ou mais** objetos selecionados, evitando o erro de alinhamento sem seleção.
- Mantidos nomes e comandos legados `rotate_exact_195`, `scale_exact_195`, `align_left_195` para testes anteriores.

## Testes acrescentados
- C++: remoção apenas do lado intermediário de 4 nós; dois restos, grupo=0, Undo/Redo; corte em retângulo fechado; bloqueio de caminho agrupado sem novo histórico.
- Qt: ativação da ferramenta pelo painel Objetos, prévia na viewport, clique real remove um lado em um passo Undo, resumo contextual com um objeto e alinhamento disponível apenas com dois, Ctrl+Z restaura contorno exato.
- A suíte existente `-SomenteTesoura` executa os testes C++ e `--edit-tools-19-7-test` Qt.

## Portões de validação
1. Garantir que as alterações não rastreadas no PC sejam preservadas; não sobrescrever `APLICATIVO`.
2. `powershell -NoProfile -ExecutionPolicy Bypass -File .\SCRIPTS\TESTAR_19_8_WINDOWS.ps1 -SomenteTesoura`.
3. Se PASS, suíte completa sem modificadores.
4. Na GUI com laser desconectado: criar linha com pelo menos 4 nós, ativar **J**, selecionar a aresta do meio em ciano, clicar, conferir os dois restos; Ctrl+Z restaura. Testar um retângulo fechado e exclusão de lado na borda.
5. Confirmar que **K** e **J** não se confundem, que um contorno selecionado em laranja não é prévia, e que o painel Objetos responde a 0, 1 ou 2+ selecionados.
6. Validar desenho real importado com sobreposição e grupos, sem permitir exclusões involuntárias.

Status: código e regressões no GitHub. **Ainda não compilado no Windows nem homologado visualmente**. Versão oficial preservada; não mesclar enquanto os gates estiverem pendentes.
