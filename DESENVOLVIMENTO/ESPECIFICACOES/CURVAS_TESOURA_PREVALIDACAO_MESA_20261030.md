# ALENCAR LASER 19.8 — Curvas, Tesoura e pré-validação da mesa (30/10/2026)

**Status:** experimento NÃO HOMOLOGADO. Base: `fix/validacao-real-19-8-20261029` (PR #12); desenvolvimento isolado em `fix/curvas-tesoura-preflight-19-8-20261030`. `main`, `APLICATIVO` e instalador aprovado intocados.

## Evidências do operador
- Foto: desenhar curva não se comporta integralmente como formas comuns; seleção/manipulação ainda limitada.
- Foto: destaque laranja da Tesoura sobre curva, clique sem remoção perceptível.
- Duas fotos: prévia e editor mostram elementos parcialmente fora da mesa.
- `Alencar_Laser_Diagnostico_TESTE_19_7(8).json`: 3 curvas Bézier e 583 contornos polilinha, 710 eventos hover, nenhuma tentativa na lista `scissor_attempts`, 1 `command_error` sem motivo; **não prova** falha/êxito de corte isoladamente.
- `Alencar_Laser_Previa_19_8(2).json`: 421 segmentos omitidos, 10 recortados, 201,76 mm de percurso laser virtual omitido. Os contadores antigos incluíam deslocamentos sem laser: distinguir modos na interface.

## Alterações propostas e codificadas
1. **Barra:** esconde os modos antigos “Nós” e “Bézier” (IDs/rotinas preservados para compatibilidade/rollback). Mantém `new_bezier`, agora “Desenhar caminhos e curvas”, glifo próprio de segmento+curva e tooltip atualizado.
2. **Edição:** Seleção por clique de contorno Bézier permanece; duplo clique em aresta nativa ativa edição de âncoras/alças; clique afastado retorna à seleção comum. Movimentação do contorno já existia. **Não** declara escala/rotação da Bézier homologadas como polilinhas.
3. **Tesoura Bézier:** implementação anterior `CurveDragSession::cutContourAtEdge` dividia caminho, deixando 100% da geometria. Novo contrato **remove aresta completa entre âncoras**, sem poligonizar cúbicas remanescentes: contorno com aresta única desaparece; remoção da extremidade encurta caminho; aresta interna de caminho aberto cria duas partes; remoção de aresta fechada abre anel. Uma transação Undo/Redo. O preview Qt destaca essa aresta matemática inteira. **Não** equivale a aparar automaticamente até interseções LightBurn; esse recurso permanece pendente e não é alegado.
4. **Aviso na prévia:** banner acima da área rolável; sempre calcula violações, mesmo com a opção de recorte desligada. Distâncias e contadores agora distinguem segmentos laser-ON de deslocamentos OFF. Relatório JSON mantém campos anteriores e acrescenta `outside_laser_segments_omitted` e `outside_laser_segments_clipped`.
5. **Mesa física:** prévia passa as dimensões de `machineWidth/machineHeight` configuradas, em vez de assumir que a área de desenho tem tamanho idêntico. Dados do projeto preservados.
6. **Iniciar trabalho:** comando de início físico **continua desabilitado**. A validação exibida na prévia é um primeiro passo; antes de implementar CAM/controle real deverá existir gate independente, bloqueante, com configuração da área física e origem verificadas. **Não** há “ignorar e disparar” nesta versão.

## Testes introduzidos ou adaptados
- `native_scissors_realcut_19_8_standalone.cpp`: cúbica isolada, corte intermediário em caminho aberto, aresta de caminho fechado, sem ponte fantasma, Undo/Redo e hits inválidos.
- `preview_workarea_19_8_standalone.cpp`: diferencia contadores laser ON e deslocamentos OFF.
- Teste Qt `--drawing-joins-19-8-test` (existente): barra sem dois ícones legados, duplo clique na Bézier, retirada real de geometria com Tesoura e restauração por Undo.
- `TESTAR_19_8_WINDOWS.ps1 -SomenteTesoura` e `-Abrir` incluem o executável portátil novo.
- Compilação e execução Windows ainda **NÃO realizadas** após mudanças. CI, se rodar, não substitui mouse físico.

## Gate operacional com laser desconectado
```powershell
git status --short
git fetch origin
git switch --track origin/fix/curvas-tesoura-preflight-19-8-20261030
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteTesoura
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteDesenho
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteSimulacao
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -Abrir
```

> **IMPORTANTE:** antes de `git switch`, confirmar árvore limpa ou preservar modificações locais; não sobrescrever código local. Não executar em `main`, nem instalar em `APLICATIVO`.

## Pendências que NÃO foram resolvidas
- Caixa de seleção/escala/rotação completa unificada entre formas poligonais e Bézier.
- Recorte por regiões topológicas delimitadas por cruzamentos em Bézier (em vez de remoção de aresta entre âncoras).
- Pré-início físico com gate homologado (transporte/G-code/posicionamento continuam bloqueados).
- Trabalhos reais em 1366x768, DPI, mouse físico e projetos `.alaser` existentes; confirmar fidelidade de importação/exportação e Undo dos dois modelos.

**Critério de homologação do editor:** somente após compilação C++/Qt, regressões completas sem falhas, inspeção visual e testes manuais com cópias dos projetos do operador; segurança de hardware em ciclo independente.
