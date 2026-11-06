# Alencar Laser — Fase 20h: correção experimental de repintura

**Data:** 06/11/2026. **Branch:** `fix/phase20h-transform-repaint-20261106`.

## Defeito encontrado na leitura do código
Em `QtEditorLab::updateSelectionTransform()`, a GUI modifica a transformação de um `QGraphicsItemGroup` ou seus itens, o que já provoca invalidação automática de áreas pelo Qt, e ainda chama `viewport()->update()` incondicionalmente. Esse pedido explícito força a invalidação de toda a viewport para a moldura/alças, inclusive quando elas ocupam uma parte menor da tela.

## Alteração
Adicionado `ALENCAR_TRANSFORM_LOCAL_REPAINT_20H=1` (somente experimento; **OFF por padrão**). Com a opção habilitada:
- Captura o quadrilátero de seleção antes de mover a prévia.
- Após a transformação, invalida uma região que cobre as alças antigas e novas, reutilizando `shapeOverlayDirtyRect199` e margem de segurança já usada para arraste.
- Deixa os `QGraphicsPathItem` e `QGraphicsItemGroup` invalidarem seus caminhos automaticamente pelo Qt.
- Em caso de dados inválidos/ausentes volta à atualização completa.
- Não altera geometria, Undo/Redo, codec `.alaser`, camadas, importador, laser ou instalador.

**Limitação importante:** com centenas de contornos espalhados pela tela, as regiões antigas e novas podem ocupar a viewport inteira. Não prometer aceleração sem medir no Windows. O diagnóstico real P95 157 ms justifica investigar, mas não demonstra que esta chamada é a única causa.

## Validação implementada
`SCRIPTS/TESTAR_REPAINT_TRANSFORM_20H_WINDOWS.ps1` compila um Qt isolado, executa a regressão de transformação central 40 objetos com flag OFF/ON, verifica Undo/Redo e geometria inalterada durante a prévia, compara SHA-256 dos pixels da prévia sob Qt offscreen e valida contadores da otimização. Falha no primeiro erro e mantém relatório local. Teste com mouse físico, zoom, DPI, 575 objetos e histórico de projetos reais ainda é obrigatório.

## Comando Windows
```powershell
cd C:\ProjetoAlencarLaser
git fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Falha fetch' }
git switch fix/phase20h-transform-repaint-20261106
if ($LASTEXITCODE -ne 0) { throw 'Falha branch; nao use reset ou force' }
git pull --ff-only origin fix/phase20h-transform-repaint-20261106
if ($LASTEXITCODE -ne 0) { throw 'Falha pull' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_REPAINT_TRANSFORM_20H_WINDOWS.ps1'
```

Para desfazer a experiência no editor de desenvolvimento, remover a variável de ambiente `ALENCAR_TRANSFORM_LOCAL_REPAINT_20H` (ou não configurá-la); não é necessário reinstalar o software. **Nenhuma modificação em `main` ou `APLICATIVO`.**

## Gate de conclusão
Ainda não homologado: precisa compilar, obter PASS de equivalência visual, A/B em 575 contornos com a mesma fixture, confirmar redução de latência em dados reais e repetir 7/7 gates de regressão. Conexão física do laser continua proibida nesta etapa.
