# Alencar Laser Qt6 — editor multiferramentas 19.8
Data: 27/10/2026
Branch: `feat/editor-multiferramentas-19-8`
Status: **experimento de engenharia; pendente compilação Windows, testes Qt e inspeção manual**.

## Problemas relatados no notebook
- A logo oficial está correta, mas seu cabeçalho desperdiça altura de desenho.
- Tesoura funciona em algumas formas, mas não em topologias específicas.
- Linha criada isoladamente a cada arraste: faltava continuar nós no mesmo caminho.
- Faltava seleção arrastando uma caixa sobre uma ou várias formas.
- Faltava menu de ações com botão direito.
- Lápis livre existente deve continuar como ferramenta distinta.

## Bloco de código entregue nesta branch
1. Cabeçalho da logo: escala máxima 162x42 (mantém proporção), 44 px de altura; subtítulo oculto. Logo binária **não** alterada.
2. Linha contínua: clique para inserir novos nós, preview do segmento seguinte, Shift fixa múltiplos de 45 graus, clique inicial fecha, clique direito conclui caminho aberto, Esc cancela. Uma transação de histórico por caminho. Gesto antigo arrastar do primeiro ponto até o segundo preservado.
3. Caixa de seleção: da esquerda para direita seleciona inteiramente contidos; da direita para esquerda seleciona contornos cruzados. Suporta Shift (acrescentar), Ctrl (alternar), Ctrl+Shift (remover), agrupamentos e camadas visíveis/desbloqueadas.
4. Menu contextual: selecionar tudo, inverter, limpar seleção, duplicar, agrupar/desagrupar, apagar, desfazer/refazer, selecionar ferramentas.
5. Atalhos Ctrl+A e Ctrl+Shift+I.
6. Tesoura **não modificou a geometria**: status diferencia hit ausente, grupo/camada bloqueada e trecho recusado por segurança; inclui contadores diagnósticos. Não liberar recortes duvidosos.
7. Teste de regressão `--editor-multitool-19-8-test`, relatório JSON `RESULTADO_EDITOR_MULTIFERRAMENTAS_19_8.json` e execução focada `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteEdicao`. Integrado também à suíte `-Abrir`.

## Fluxo seguro no Windows
```powershell
cd C:\ProjetoAlencarLaser
git status --short
git fetch origin
git switch --track origin/feat/editor-multiferramentas-19-8
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteEdicao
```
Se branch local já existir, usar `git switch feat/editor-multiferramentas-19-8` e `git pull --ff-only`. Em qualquer conflito ou modificação local, interromper sem sobrescrever.

Após passar: executar `-Abrir` na mesma branch. Com o laser desconectado, testar logo compacta, linha fechada/aberta, seleção por caixa em sentidos opostos, context menu, Tesoura em desenhos reais, LBRN2 real, undo/redo e projeto .alaser v5. Nunca alterar `main`, APLICATIVO ou base homologada antes dos gates.

## Limites conscientes
- Tesoura por intervalos não equivale completamente à lógica LightBurn: vértices, tangências, grupos e caminhos compostos ainda exigem classificação geométrica e fixtures reais.
- Linha contínua ainda não une automaticamente ao extremo de uma forma pré-existente, nem desenha curvas Bézier pelo arraste dos nós.
- Menu contextual é inicial; recursos de solda/offset/alinhamento avançado não estão homologados.
- LBRN2 permanece parcial.
- Código e teste Qt não foram compilados neste ambiente; é obrigatório obter o JSON real do Windows antes de qualquer afirmação de PASS.

## Homologação
NÃO APROVADO. PR experimental em rascunho. Controles físicos/laser bloqueados.
