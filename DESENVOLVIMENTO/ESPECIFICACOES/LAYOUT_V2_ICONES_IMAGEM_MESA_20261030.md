# Alencar Laser TESTE 19.8 — Layout V2, imagens e prévia restrita à mesa

**Data:** 30/10/2026 • **Status:** implementação experimental na branch `fix/validacao-real-19-8-20261029`. PR #12 em rascunho. **Não homologado, não mesclar `main`, não tocar `APLICATIVO`**.

## Regras aprovadas pelo usuário

- Interface intuitiva/familiar para quem já conhece LightBurn; identidade Pégaso própria, preto/laranja, em português; topo menos alto e mais nítido.
- Ícones para abrir/salvar, desfazer/refazer, simular, alinhar à esquerda/centros/direita/topo/base, girar 90º, espelhar horizontal/vertical; textos completos em tooltips/acessibilidade, comandos também nos menus.
- **Um único botão Pausar/Continuar.** Não manter dois botões. Controles futuros de iniciar, enquadrar, parar, relógios estimado/decorrido.
- Imagens importadas clicáveis/movíveis/redimensionáveis, sem gerar vetores automaticamente; salvar/reabrir com imagem e dimensões.
- Prévia virtual nítida dentro dos limites da mesa com aviso de elementos externos; sem alterar o arquivo real e sem habilitar máquina física.

## Implementação por componente

### Layout Qt / Pégaso

- Cabeçalho com altura máxima 48 px, pégaso como símbolo isolado em 38×38 px e lettering ALENCAR LASER Qt de alta nitidez, mantendo fallback à logo anterior se o kit estiver ausente.
- Ações principais compactadas com glifos desenhados por Qt Painter, sem fonte/icon pack externo. A função dos botões antigos continua acessível nos menus.
- Barra de alinhamento e transformação via `QToolButton` (31 px) e IDs acessíveis: `quick_align_0` a `quick_align_5` utilizados onde cabível, `quick_flip_horizontal`, `quick_flip_vertical`, `quick_rotate_90`.
- Espelhamento vertical reutiliza `Editor::mirrorY` com pivô no centro da seleção, como `mirrorX` já fazia no eixo horizontal. Inversão/undo seguem o mesmo motor de histórico vetorial.
- Faixa operacional inferior `operation_strip_19_8` com quatro ícones **DESABILITADOS** (start, pause/continue único, stop, frame). Indicadores "Estimado —" / "Decorrido —" não alegam andamento físico. A prévia offline possui botão Pausar/Continuar funcional.

### Prévia offline: limitar à mesa

- Novo `src/preview_workarea_19_8.hpp` com corte Liang-Barsky por segmento, coordenadas finitas, inclusive linhas verticais/horizontais; reune segmentos válidos por deslocamentos de laser OFF. Se o trecho inteiro está fora, omite. Se parcialmente, recorta no limite.
- `Qt/preview_dialog_19_8.hpp` inclui opção ligada por padrão "Simular somente dentro da mesa", recalcula tempo/distâncias usando somente o plano filtrado, indica número de trechos ignorados/recortados e distância laser omitida. Quando desmarcada, a prévia mostra o plano experimental completo, para diagnóstico.
- **Atenção:** o motor não foi homologado fisicamente; essa restrição é PREVISUALIZAÇÃO, nunca código de emissão. A mesa usada para a simulação deve ser revista quando área de desenho e dimensão física forem diferentes. O usuário deve aprovar ambos os valores antes de simular trabalhos físicos no futuro.
- Teste portátil `tests/preview_workarea_19_8_standalone.cpp` cobre interior/exterior, horizontais, verticais, clipping de ponta e ligações OFF. Ligado ao CI GitHub Linux e ao `-SomenteSimulacao` Windows.

### Bitmap importado como objeto

- `QtEditorLab` desenha caixa laranja com alça no canto inferior direito e permite selecionar/arrastar/redimensionar proporcionalmente (fator deriva da imagem original). Modo Selecionar, mouse físico; vetores continuam em motor separado. O PNG não é convertido em vetor no movimento.
- Persistência no **mesmo .alaser v5**: `qt/raster_project_19_8.hpp` adiciona campo `raster_object_19_8` nos metadados `design/layers.json` (campo desconhecido é tolerado por leitores compatíveis, mas a interoperabilidade com outras versões deve ser testada).
- Guarda PNG base64, x/y/w/h reais, dimensões em pixels, hash SHA-256; restringe imagem e buffer a limites seguros. O pacote integral possui digest sobre layers.json; na reabertura SHA256 individual e integridade geral são verificadas ANTES de trocar estado gráfico.
- Após salvar, projeto v5 mantém a imagem; ao reabrir sem bitmap, remove a imagem temporária anterior. Recuperação offline/snapshot usa o mesmo mecanismo.
- **Limitações ainda pendentes:** rotacionar bitmaps, múltiplas imagens independentes, editar brilho/contraste/fotogravação no objeto, enviar seleção de imagem para camadas, histórico Undo/Redo integrado para deslocamento/redimensionamento e testes de arquivos grandes. O pacote suporta **uma** imagem de referência por documento nesta fase; não afirmar compatibilidade completa com LightBurn ou homologação comercial.

### Testes e relatórios

- `--layout-icons-19-8-test` verifica cabeçalho, existência de botões, alinhamento real, espelhamento vertical e bloqueio dos botões físicos.
- `--raster-object-19-8-test` testa roundtrip PNG/x/y/w/h, proteção SHA256 e arquivo legado sem imagem.
- A opção `-SomenteInteracao` executa ambos antes dos testes anteriores de texto/transformações.
- A opção `-SomenteSimulacao` inclui teste portátil de geometria restringida à mesa.
- CI GitHub Windows Qt6 compila a interface e CI portátil Linux executa recorte da mesa; **a automação não testa movimentação com mouse real, qualidade de imagem ou arquivo comercial do usuário**.

## Critérios antes de liberar editor V2

1. Todas as regressões automáticas sem falha: `-SomenteInteracao`, `-SomenteSimulacao`, `-Abrir`. Se falhar, manter PR rascunho.
2. Reabrir cópia de `.alaser` com PNG importado, confirmar pixels e medidas após selecionar, mover e redimensionar.
3. Zoom ao redor de imagem não desloca o posicionamento, escalar não quebra proporção; alternar para vetor não perde clique.
4. Simulação de desenho dentro e fora da mesa; os dados do desenho original devem permanecer intactos, relatórios devem indicar omissões.
5. UI em 1366×768 e DPI 100/125/150%, ícones legíveis, tooltips PT-BR, sem barra superior cortada; ícone Pégaso reconhecível.
6. Botões físicos, conexão e emissão **permanecem desabilitados**. Nenhuma migração automática de `APLICATIVO` ou `main`. Preservar reversão/backups.

## Como testar no Windows após `git pull --ff-only`

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteInteracao
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteSimulacao
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -Abrir
```

**Observação:** testes atuais fazem diagnóstico, não validam automaticamente todos os gestos reais ou a segurança física. Teste de produção com AlgoLaser Alpha Mk2 20W é fase separada.
