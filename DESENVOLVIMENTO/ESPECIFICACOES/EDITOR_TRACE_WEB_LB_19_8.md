# Alencar Laser Qt6 19.8 — estudo LightBurn e bloco de refinamento (28/10/2026)

**ESTADO:** código experimental na branch `feat/editor-trace-diagnosticos-curvas-19-8`. Verificações estruturais não equivalem a compilação; Windows Qt, prévia do rastreio e mouse real ainda não homologados. `main` e `APLICATIVO` intocados.

## Evidência direta
- Vídeo real `20261028_135547.mp4`, 49,95 s, 1280×720: projeto vetorial no editor, letra R facetada ao ampliar, trabalho com Tesoura, formas e camada.
- Palavra 'R' vetorizada por `QPainterPath::toSubpathPolygons`, gerando retas. Caminhos importados LBRN2 e texto convertido em polilinhas não se tornam Bézier nativa por apenas clicar na ferramenta.
- Clique na paleta de cores já transferia seleção; **trocar pelo seletor direito de camadas não transferia**. Ambos os gestos foram unificados.
- O diagnóstico coletava contadores genéricos da Tesoura, sem contexto do objeto/camada e sem um método simples para compartilhar o JSON.
- A barra `DESENHO/MESA` consumia espaço desnecessário acima da mesa.

## Referências LightBurn verificadas
- https://docs.lightburnsoftware.com/2.2/Reference/TraceImage/ — traçar bitmap, threshold/cutoff, prévia, ignorar pequenos componentes, suavidade, otimização, proteção a desenho desuniforme, preservar vazados, traço agrupado.
- https://docs.lightburnsoftware.com/2.2/Reference/EditNodes/ — âncoras e alças tangentes, alternar linhas e curvas com S/L, não confundir primitiva/texto com curvas sem converter.
- https://docs.lightburnsoftware.com/latest/Reference/ConvertToPath/ — text object/primitive exige converter em caminho para editar nós diretamente; após converter perde semântica do texto.
- https://docs.lightburnsoftware.com/latest/GetStarted/ColorsAndLayers/ — clique na cor com seleção transfere à camada; sem seleção, ativa camada de criação; todos os objetos vetoriais ficam com contornos na mesa por padrão.
- https://docs.lightburnsoftware.com/legacy/UI/Menus — Optimize Selected Shapes simplifica com tolerância de erro e pode adaptar para linhas/arcos.

## Referência Alencar Laser Web
- Versão registrada v0.10.58.7; zoom, seleção, Tesoura com índice espacial, nós persistentes, ligação reta após exclusão. NÃO confundir com a versão Qt.
- O ZIP da Web com código do rastreador **não foi localizado neste turno**. Não afirmar que o rastreador Qt seja cópia 1:1 nem que herde GRBL/CAM.
- Reaproveitamento consciente: manutenção de IDs/nós, índice de seleção, transações Ctrl+Z, instrumentação de latência e rejeições seguras.

## Implementado na branch (SEM Windows PASS)
1. **Texto:** gera polilinhas mais detalhadas via `QPainterPath::toSubpathPolygons` em resolução 18×, convertendo resultados de volta a mm e impondo limite 60 mil pontos. É aproximação refinada, **não Bézier nativa/editável como texto**.
2. **Bézier nativa:** menu Editar → Suavizar curva Bézier nativa selecionada. `CurveDragSession::smoothSelected` converte arestas retas em cúbicas com tangentes Catmull-Rom local (um Ctrl+Z). Limite 3000 nós, apenas nos CurveDocument v5. Selecionar polilinha não executa esse recurso.
3. **Imagem:** abrir/arrastar PNG/JPG/BMP/WebP para overlay temporário (máx 1600px e 1,5MP) e menu direito **Traçar imagem**. Diálogo limiar automático Otsu ou manual, descarte de manchas, proteção a detalhes pequenos e simplificação RDP com erro em pixels. Preview P&B com vetores magenta; commit agrupado em uma transação na camada ativa. Captura buracos como contornos separados; não compõe automaticamente um preenchimento real com regra even-odd, não gera CAM/G-code.
4. **Limite geométrico:** contornos extraídos de bordas de pixel, simplificação inicial 0,45 px; em detalhes pequenos usa 30% dessa tolerância. Não há reconstrução perfeita de letra legível em imagem ruim nem reconhecimento OCR. Ruído isolado removível, acentos próximos preservados. 1000 contornos/50 mil nós por importação, 250 mil bordas por trace, freio contra RDP muito lento.
5. **Camadas:** além da paleta, selecionar opção no combo do painel com objeto(s) selecionado(s) passa a mover a seleção imediatamente. Guarda undo; sem seleção continua sendo definição da camada ativa.
6. **Tesoura:** em clique recusado armazena `scissor_click_details` com `reason_code`, quantidade de interseções, zoom, ID de objeto, subcaminho, nº de nós, camada e flags bloqueada/visível; cortes aceitos também geram evento. Até 100 ocorrências, **somente durante coleta explicitamente ativa**, nenhuma coordenada ou conteúdo do desenho, nada enviado pela rede. Botão para copiar JSON.
7. **Barra superior:** botões Área, Mesa e Desenho compactos, medidas abreviadas, margens/altura reduzidas; mantém IDs de automação, tooltips e controle visual de dimensões.

## Testes automatizados adicionados
- `--trace-editor-19-8-test`: imagem sintética com retângulo vazado, uma mancha isolada e letra i com ponto; tolerância e contornos, overlay sobrevivendo rebuild, grupo único + Ctrl+Z, densidade de letra R, diagnóstico de clique Tesoura e suavização de Bézier com undo.
- O script `SCRIPTS/TESTAR_19_8_WINDOWS.ps1` recebeu `-SomenteRastreio`; `-Abrir` também inclui o gate.
- Já existente `-SomenteCamadas` cobre transferência direta pelo seletor do painel, desagrupar, Line/Fill e persistência.

## Orientação Windows
1. Verificar `git status --short --branch`, preservar alterações em `.vscode/settings.json` etc.
2. `git fetch origin`; `git switch --track origin/feat/editor-trace-diagnosticos-curvas-19-8` se branch inexistente localmente.
3. `powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteRastreio`
4. `powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteCamadas`
5. Se ambos PASS, rodar suíte `-Abrir` e testar manualmente imagem P&B sobre um **arquivo de teste**, letra R no zoom alto, curva nativa de 3 âncoras, mudança de camada via painel, Tesoura em trecho permitido e recusado, botão Copiar Diagnóstico.
6. Logs: `RELATORIOS/AUDITORIAS/GITHUB_19_8/rastreio/RESULTADO_RASTREIO_EDITOR_19_8.json`, além de `RELATORIOS/COMPILACAO/BUILD_GITHUB_19_8.txt`.

**Nunca enviar comando à máquina, habilitar TCP/GRBL, sobrescrever base 19.7.1 homologada ou mesclar PR sem testes.**
