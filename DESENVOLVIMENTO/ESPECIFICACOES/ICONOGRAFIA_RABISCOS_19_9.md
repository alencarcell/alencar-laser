# Alencar Laser 19.9 — Ícones referenciados pelos desenhos do proprietário
Data: 02/11/2026 • Status **PROVISÓRIO** • PR #13 experimental

## Fonte de verdade
O original manuscrito enviado (grupos de **Arquivo e sistema**, **Espelhamento e alinhamento**, **Distribuição e dimensionamento**, **Desenho**, **Edição de nós**, **Movimento**) define o significado e a geometria principal. A prancha gerada por IA foi aprovada apenas provisoriamente e **não** substitui o original.

| Desenho manual | ID do glifo no código | Característica que deve ser fiel |
|---|---|---|
| Folha com sinal + | `new` | documento dobrado + adição no canto |
| Pasta aberta | `open` | pasta reconhecível |
| Disquete | `save` | salvar distinto de abrir/importar |
| Documento com seta | `import` | importar = seta entrando/saindo da folha |
| Arcos de desfazer/refazer | `undo`, `redo` | opostos, ponta clara |
| Monitor com olho | `preview` | monitor **e** olho, não somente olho |
| Engrenagem / ferramentas cruzadas | `settings`, `device_settings` | geral ≠ dispositivo |
| Triângulos acima/abaixo do eixo | `flip_v` | reflexão vertical com **linha horizontal** |
| Triângulos esquerda/direita do eixo | `flip_h` | reflexão horizontal com **linha vertical** |
| Triângulos de lados opostos de eixo diagonal | `mirror_axis` | reflexão em **linha diagonal** (ícone pronto como referência; função só se existir) |
| Alvo | `align_both` | alvo de centros X/Y |
| Retângulos empilhados / lado a lado | `align_center_y`, `align_center_x` | alinhamento com linha guia |
| Duas barras + setas verticais | `distribute_vertical` | distribuir objetos no eixo Y |
| Duas barras + setas horizontais | `distribute_horizontal` | distribuir objetos no eixo X |
| Barras com largura/altura igual | `same_width`, `same_height` | equivalência de dimensão não é distribuição |
| Círculo com setas de giro | `rotary_config` | equipamento rotativo, não rotação livre |
| Casa | `home` | posição inicial; não confundir com mandar máquina à origem |
| Cursor, lápis, formas geométricas | `select`, `pencil`, `rectangle`, `ellipse`, `triangle`, `pentagon`, `hexagon`, `octagon`, `star` | silhuetas fechadas e completas |
| Polígono com nós marcados | `edit_nodes` | distinguir seleção inteira de edição de nós |
| Traços com + / − / X e formas de curva | `add_node`, `remove_node`, `delete_edge`, `smooth_node`, `corner_node`, `line_to_curve`, `curve_to_line`, `axis_align` | semântica distinta por ação |
| Direções cardeais e diagonais | `move_nw`, `move_n`, `move_ne`, `move_w`, `move_e`, `move_sw`, `move_s`, `move_se` | seta completa, direção inequívoca |
| Controles físicos | `play`, `pause`, `stop`, `fit` | universal e **sempre bloqueado** até homologação |

## Contrato técnico de rendering
- Fonte principal: `src/tool_glyphs.hpp`, desenhado por `QPainter`, imagem **64×64 px**, proporção `devicePixelRatio=2`, área lógica **32×32**.
- **Correção essencial**: eliminar `p.scale(2,2)`. O Qt já transforma coordenadas lógicas para pixels no QPixmap com DPR=2. Reaplicar escala resultava em glifos **recortados**, explicando símbolos em formato de fragmentos na captura do proprietário.
- Margem de segurança: nenhum pixel significante deve tocar as 2 primeiras ou últimas colunas/linhas físicas da imagem.
- Modos: normal, ativo e desabilitado; o ícone não altera estado funcional do comando. Testar no UI real em escala 100% e 200%, fundo claro e escuro quando disponível.
- Teste: `alencar_test7_qt6.exe --icon-pack-19-9-test <fixture> <pasta>`. Retorna `RESULTADO_ICONES_19_9.json` e **`PRANCHA_ICONES_REAIS_19_9.png`** (prova do renderer Qt, não simulação IA).
- `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteEngrenagens` produz prancha em `RELATORIOS/AUDITORIAS/GITHUB_19_8/icones_19_9/`. Actions Qt Windows exige o teste e preserva a prancha como artifact, se a suíte passar.

## Limites / não inventar funcionalidades
Glifos adicionais existem como *catálogo visual*. **Criar um ícone não ativa nem comprova a implementação do comando** (p. ex., `mirror_axis`, comandos novos de nós e direções); só ligar à UI depois de verificar a ação real e testes de Undo. Ações físicas de iniciar, pausar, parar, origem e enquadrar permanecem desativadas.

## Aceitação do proprietário e próximo refinamento
Manter a aprovação **provisória**. Confrontar a prancha PNG real, os rabiscos e o executável no Windows. Refinar largura/ângulos das setas, espelhamento e contraste, sem cortes, sem texto truncado. Não alterar `main`, `APLICATIVO`, firmware, CAM, emissão, backups ou instalador homologado sem aprovação.


## Revisão de distinção visual — 02/11/2026

Depois da aprovação **provisória** do catálogo, foram refinadas as seguintes ambigüidades reais da prancha Qt:

- `settings` — uma engrenagem de ajuste com arco interior; distinta de `gear_simple` — engrenagem isolada de dentes explícitos.
- `mechanism` — duas engrenagens de **diâmetros diferentes** em diagonal; `gear_pair` — duas engrenagens **iguais** lado a lado; `rack` — cremalheira de dentes retos com pinhão.
- `line_to_curve` — reta em cima, curva embaixo e seta de conversão; `curve_to_line` — curva em cima e reta embaixo. O desenho destaca o resultado em laranja para não depender apenas de uma seta minúscula.
- `smooth_node` — curvatura com representação adicional de tangentes; `corner_node` — canto angular, sem tangentes.
- Ações **já existentes** em `Gerar > Engrenagens` recebem seus ícones: simples, par, cremalheira e oficina. Não foram criadas novas funções de hardware.

Testes do Qt foram ampliados para comparar pares em imagem real **reduzida a 20 px**, não apenas por hashes diferentes; exigem diferença perceptível mínima em 18 pixels da grade 20×20. O teste também gera `PRANCHA_CRITICOS_19_9.png` com exemplares em 20, 24 e 58 px. O ensaio Qt comprova que todas as quatro ações do menu de engrenagens têm ícones atribuídos.

**Estado de aprovação:** ainda experimental. A imagem original manuscrita continua a referência primária; a qualidade gráfica precisa de avaliação do proprietário na imagem real e na tela em DPI 100%/200%. PR #13 draft; `main`, `APLICATIVO`, dispositivos e emissão física intactos.


## Refinamento isolado — excluir aresta (02/11/2026)
- `delete_edge`: novo símbolo com duas extremidades de segmento separadas por uma lacuna e um X central, para distinguir o comando de `remove_node` nas miniaturas pequenas.
- Alteração apenas em `src/tool_glyphs.hpp`; sem mudanças no motor de geometria, na Tesoura, no Undo ou em comandos de máquina.
- Commit `dcac734`; **testes locais Qt/Windows e aprovação visual desta mudança ainda pendentes**. Validar com `-SomenteIcones`, inclusive a prancha em 20 px; não declarar homologado antes da nova execução.
