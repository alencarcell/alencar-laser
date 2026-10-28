# Alencar Laser Qt 19.8 — texto suave, Bézier, rastreamento, diagnósticos e mesa compacta
Data: 28/10/2026. Fonte visual: vídeo real `20261028_135547.mp4` (aprox. 50 s).
Estado: **código experimental no GitHub. Ainda NÃO compilado no Windows nesta branch; não homologado**.

## Evidência do vídeo
- Letra "R" vetorizada diretamente na mesa apresenta trechos retos facetados em ampliação.
- Uso de Tesoura, seleção por contorno e movimentação ficou mais maduro, mas ainda há geometria complexa e ações que merecem teste manual.
- A faixa de identificação da mesa ocupa altura útil; botões podiam ser mais compactos.

## Referências oficiais LightBurn
- Trace Image: https://docs.lightburnsoftware.com/1.7/Reference/TraceImage/
  - Ferramentas > Rastrear imagem; menu direito sobre imagem; Alt+T.
  - Limiar (threshold), cutoff, ignore less than, optimize, smoothness, transparência, sketch trace, prévia roxa, manter/excluir bitmap.
- Draw Lines: https://docs.lightburnsoftware.com/2.2/Reference/DrawLines/
  - Criar curvas com alças por arraste; modo S para nós suavizados.
- Edit Nodes: https://docs.lightburnsoftware.com/2.2/Reference/EditNodes/
  - Editar nós e tangentes cúbicas; formas primitivas/texto exigem conversão para caminho.
- Curve Quality: https://docs.lightburnsoftware.com/latest/Reference/SettingsPreferences/
  - Curvas são visualizadas como pequenos segmentos; qualidade e antialiasing alteram a precisão na tela.

## Reuso responsável de Alencar Laser Web
- Evidência verificada do Web v0.10.58.6/v0.10.58.7: índice espacial para selecionar e encontrar segmentos intersectantes, operações persistentes de nós, proteção de contornos não selecionados e medição de desempenho. Nenhum código de rastreamento de imagem Web completo/validado foi localizado; NÃO afirmar migração de um rastreador inteligente que não foi inspecionado.
- Fonte documental: `RELATORIO_IMPLEMENTACAO_v0.10.58.6.md`, `RELATORIO_IMPLEMENTACAO_v0.10.58.7.md`.

## Alterações desta branch experimental
1. **Texto**: substitui `toSubpathPolygons` (tesselação visual pouco precisa) por leitura de elementos da fonte TrueType com subdivisão adaptativa de Bézier cúbica; tolerância geométrica de 0,016 mm, limite máximo de nós; preserva subcaminhos/vazados e cantos. Texto confirmado continua convertido em contornos de polilinha; NÃO é texto editável semanticamente nem Bézier nativa.
2. **Bézier**: botão `Nova curva` cria uma cúbica nativa com duas âncoras e alças visíveis (via `CurveDragSession.addEditableCubic`), com histórico próprio Ctrl+Z/Y, deslocamento e handles editáveis. NÃO converte automaticamente polilinhas antigas em Bézier.
3. **Rastreamento**: novo `raster_trace_19_8.hpp`. Importa PNG/JPG/BMP/WebP (botão Arquivo/Imagem ou arraste), exibe bitmap **somente como overlay temporário**. Menu direito > Traçar imagem / Alt+T abre diálogo para limiar Otsu inicial, ignorar pequenas ilhas, otimização de nós, suavização opcional, escala em mm e prévia P&B com linhas roxas. Somente o botão Criar traçado grava os contornos como uma única operação de desenho. Imagem original nunca é sobrescrita. Limites 960 px lado do traçado / 500 mil arestas / 80 mil nós / 3500 contornos. Buracos e bordas separados por orientação do anel.
   - NÃO implementa OCR, correção semântica de letras, sketch trace adaptativo para fotos com iluminação irregular, ajuste de transparência separado ou equivalência plena do LightBurn. Texto pequeno exige validação manual.
   - O bitmap de referência NÃO é persistido no .alaser e precisa ser rasterizado antes de fechar/reabrir. Mantém operações da máquina BLOQUEADAS.
4. **Diagnóstico Tesoura**: o coletor antes só aceitava nomes antigos de eventos — `scissor_no_hit`, `scissor_no_safe_span` e `scissor_continuous_span_removed` eram descartados silenciosamente. Lista corrigida. Em coleta opt-in, JSON de exportação inclui até 100 tentativas com resultado `miss/grouped/hidden/locked/ambiguous/cut`, ID interno do objeto, segmento, zoom e tempo desde o início. NÃO inclui coordenadas, nome/caminho do arquivo, texto, imagem ou conteúdo vetorial. Novo evento de ativação do modo Tesoura/Bézier, criação de curva e rastreamento. Limpar diagnóstico também elimina as amostras.
5. **Camadas**: clicando numa cor com seleção, a mudança é imediata; informa quantas formas foram transferidas. Sem seleção, ativa a camada. Sem mover Bézier nativa através da paleta nesta beta.
6. **Área útil**: reduz margens externas do editor, largura de rótulos e altura dos botões Área/Mesa/Desenho/Zoom; preserva funcionalidade e logo oficial original.

## Testes de regressão
- `powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteRefino`
- Cubre R e O (vazado), R pequeno, bitmap preto/ branco com furo, componente 1 px removido, contorno e orçamento de nós, whitelist de eventos, nova Bézier com handles e undo/redo, caixa de diagnóstico ativada com tentativa real da Tesoura.
- JSON: `RELATORIOS\AUDITORIAS\GITHUB_19_8\refino\RESULTADO_REFINO_19_8.json`.
- Se PASS, `-SomenteImportacao -ArquivoLbrn2 "<caminho do SUPORTE PRA ALEXA_backup.lbrn2>"`, depois `-Abrir` (sem laser conectado) e mouse físico.
- Testar PNG real de logo, imagem com letra pequena e imagem com furos; comparar visualmente P&B, prévia roxa, contornos, suavização e Undo; teste com zoom elevado na letra R.

## Segurança e limites
- `main` / versão homologada / projeto Alencar Laser Web intactos.
- Nenhuma configuração CAM, G-code, firmware, potência, velocidade, movimento, conexão física ou emissão habilitada.
- Ainda faltam compilação Qt6/UCRT64 Windows, testes de memória/tempo em imagens complexas e verificação visual dos rastros/ curvas.
- Em falha, não usar versão para fabricação; manter backup dos originais, não fazer merge antes de homologar.
