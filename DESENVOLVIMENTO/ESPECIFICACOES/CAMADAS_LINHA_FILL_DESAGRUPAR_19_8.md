# Alencar Laser 19.8 — Desagrupar, texto sem overlay pendente e Camadas
Data: 28/10/2026
Status: experimental, **Windows não compilado/testado nesta branch**, sem homologação.

## Referências LightBurn (consultadas em 28/10/2026)
- https://docs.lightburnsoftware.com/latest/Reference/CutsLayersWindow/
  - Paleta de cores atribui operações a objetos; modos vetoriais Line, Fill e Offset Fill; camadas com opções de visibilidade, saída e configuração.
- https://docs.lightburnsoftware.com/2.1/Reference/Grouping/
  - Group Ctrl+G, Ungroup Ctrl+U; grupos podem conter objetos de várias camadas; desfazer grupo não converte nem deforma geometria.
- https://docs.lightburnsoftware.com/2.1/Reference/CutSettingsEditor/
  - Line percorre contornos; Fill gera linhas de gravação no interior de formas fechadas.
- https://docs.lightburnsoftware.com/latest/Reference/CutSettingsEditor/SubLayers/
  - Line+Fill atual via subcamadas; evitar apresentar o antigo Fill+Line como opção atual.
- https://docs.lightburnsoftware.com/2.0/Reference/Text/
  - Esc encerra criação de texto; texto é editável como texto no LightBurn, o que ainda não ocorre no Alencar Laser.

## Entregas da branch
1. **Texto**: trocar a ferramenta com campo vazio cancela o overlay e restaura o foco; campo preenchido é finalizado e salvo como contornos. Perda de foco também conclui/cancela. Campo agora captura apenas uma largura variável em torno da string; atalhos das ferramentas são desabilitados durante digitação e reabilitados ao sair.
2. **Desagrupar**: nova Editor::ungroupSelection(ids) resolve e desagrupa todos os grupos referidos pela seleção em **uma transação Ctrl+Z**, preservando objeto, contorno, camada e ID. Recusa grupos com membros ocultos/bloqueados (sem alterações parciais).
3. **Tesoura**: botão do painel "Desagrupar → Tesoura (K)" e menu direito "Desagrupar e ativar Tesoura". Não executa corte automaticamente; após desagrupar, usuário escolhe os arcos.
4. **Camadas**: a camada já podia ser criada pelo editor e receber formas com o botão "Enviar seleção" ou Shift+clique na paleta. Adicionado seletor "Linha" e "Preenchimento" ao painel Camadas. Mudança salvável em `.alaser` v5 como `layers[].mode` com valores `Line` ou `Fill`, preservada por undo/redo. `editor-meta-1` antigo sem modo abre assumindo `Line`. Modo desconhecido é recusado para evitar perda.
5. **Prévia visual**: `Line` exibe apenas contornos; `Fill` dá cor semitransparente a formas poligonais fechadas. **Não gera hatch/scanlines de fabricação, nem configura velocidade/potência, nem exporta G-code**.
6. Teste Qt GUI `--layers-ungroup-19-8-test`, script `-SomenteCamadas` e inclusão no gate completo `-Abrir`. Testa UI de modo/camada, múltiplos grupos, Ctrl+U, undo/redo e salvar/reabrir projeto com modo.
7. Teste pré-existente de texto estendido: campo vazio desaparece ao clicar em Seleção sem modificar documento.

## Limitações declaradas
- Camadas e Fill são **propriedades editoriais**; LightBurn real usa configurações de processo e subcamadas, que ainda não estão implementadas.
- O raster de Fill para cortes/laser não está implementado. Buracos de letras originadas em vários contornos podem não apresentar a composição de preenchimento correta na visualização atual; não usar para produção.
- Tesoura: a separação dos grupos libera seleção individual, mas não resolve tangências, sobreposições ambíguas ou curvas Bézier nativas.
- Texto confirmado é convertido em caminhos, não permanecendo uma entidade textual editável.
- Controle físico da máquina, G-code, CAM, Wi-Fi/TCP e emissão continuam bloqueados. `main` e `APLICATIVO` não foram alterados.
- O código ainda exige compilação Qt 6/UCRT64 no Windows e teste do mouse real.

## Comandos de validação
1. `git status --short` para inspecionar alterações locais antes de trocar branch.
2. `git fetch origin`, depois `git switch --track origin/feat/camadas-preenchimento-desagrupar-19-8` quando a árvore estiver segura.
3. `powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteCamadas`
4. `powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteInteracao`
5. Apenas com ambos PASS: `powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -Abrir`.
6. Testar visualmente texto vazio → Seleção, texto digitado → Seleção, Ctrl+U/redo, duas camadas Linha/Fill, cor, ocultar/bloquear, salvar/reabrir cópia, Tesoura após desagrupar.

Em falha, NÃO fazer merge, nem publicar instalador ou tocar a versão homologada.
