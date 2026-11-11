# ALENCAR LASER — Missão 20zb | Repintura localizada e medição de fases

Data: 11/11/2026
Base preservada: `feat/corte-entre-nos-nos-visiveis-20za-20261110` (PR #30).
Branch experimental: `perf/repaint-local-20zb-20261111`.
Alvo: Windows 11 / Qt 6; laser **desconectado**.

## Evidência e problema

Relatório fornecido pelo operador: `Alencar_Laser_Diagnostico_TESTE_19_7(20261111-013014).json`, versão interna `TESTE_19.8`.
Amostras: 719 pinturas, paint P50 6,1953 ms; P95 178,8958 ms; P99 246,7722 ms; pico 299,1173 ms. A área retangular de repintura P95 foi 1,00 da viewport; background P95 30,5606 ms, foreground P95 0,9411 ms, hit-test P95 0,0438 ms. Cenário com 580 contornos/13.226 nós, 787 hovers, 8 cortes de Tesoura, 5 undo e 3 redo; nenhum evento descartado; hardware não conectado.

A leitura NÃO demonstra que o desenho dos nós da ferramenta J cause a latência. Pintura Qt/itens, composição e chamadas de atualização ainda precisam de atribuição por fases. A otimização aqui reduz **pedidos desnecessários de atualização da viewport** em caminhos específicos e registra sua frequência, sem prometer redução de P95 antes de medição controlada.

## Alteração cirúrgica 20zb

- Em `QtEditorLab::activate`, desenhar novamente somente o retângulo antigo e novo do overlay de seleção/nós, em vez de `viewport()->update()` integral. Clique repetido sem mudança visual não solicita repintura adicional.
- Em `clearSelection`, invalidar a antiga região dos indicadores, preservando a limpeza de estilos anteriores.
- Em `setTool`, invalidar as regiões do indicador anterior/novo. O highlight J e os marcadores continuam sendo itens da cena e têm invalidação própria pelo Qt.
- Usar `sceneBoundingRect` do item ativo com margem em pixels para os nós. Para um único nó (path move-only), incorporar sua posição canônica ao retângulo.
- Usar a rotina preexistente `shapeOverlayDirtyRect199` para a moldura de seleção, inclusive alças de rotação.
- Em overlay nativo Bézier ou coordenadas inválidas, preferir a repintura integral por segurança.
- Contadores no JSON: `foreground_overlay_local_updates_20zb` e `foreground_overlay_full_updates_20zb`. São **pedidos explícitos** desta rotina, não o número total de paints Qt.
- Perfilamento opcional: `ALENCAR_PAINT_PHASE_PROFILE_20ZB=1`, exporta `paint_phase_paired_samples_20zb`, `paint_phase_paired_total_p95_ms_20zb`, `paint_phase_paired_other_inclusive_p95_ms_20zb` e `paint_phase_paired_invalid_20zb`. O tempo "other inclusive" abrange itens Qt, composição e overhead, não somente vetores.

A ferramenta J, marcadores cianos, Tesoura K, Editor::removePolylineEdge, IDs, undo/redo, arquivos `.alaser v5`, importações e camada de transmissão permanecem inalterados.

## Testes e validação Windows

No PowerShell, **preservar antes** qualquer alteração local de `git status --short`:

```powershell
cd C:\ProjetoAlencarLaser
git status --short
git fetch origin
git switch --track origin/perf/repaint-local-20zb-20261111
# Se a branch já existir localmente: git switch perf/repaint-local-20zb-20261111
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_19_8_WINDOWS.ps1' -SomenteTesoura
```

O teste Qt `--edit-tools-19-7-test`, já executado pelo modo focado, passa agora por:
1. Selecionar um retângulo sintético pequeno;
2. Trocar Seleção → Nós → Seleção e limpar seleção;
3. Confirmar que a rotina solicitou atualizações locais, **não** atualização integral para esse objeto;
4. Confirmar a identidade do estado serializado e do histórico, após as trocas;
5. Prosseguir com regressões existentes de Tesoura K, J, hover, marcadores, corte/undo/redo.

Se passar, executar a suíte completa e abrir:

```powershell
$env:ALENCAR_PAINT_PHASE_PROFILE_20ZB = '1'
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_19_8_WINDOWS.ps1' -Abrir
```

Com o laser desconectado, repetir tarefas equivalentes na mesma arte e zoom da sessão anterior e exportar manualmente o diagnóstico. Desativar o modo de medição ao concluir:

```powershell
Remove-Item Env:\ALENCAR_PAINT_PHASE_PROFILE_20ZB -ErrorAction SilentlyContinue
```

**Interpretar medidas com cautela:** pintar com perfilamento tem custo próprio; não comparar o paint P95 perfilado diretamente com P95 antigo sem usar as mesmas condições. Ideal fazer uma medição com perfilamento para atribuição e duas séries sem perfilamento (20za vs 20zb) com carga/zoom/gestos equiparáveis. Inspecionar latência, área de pintura, FPS percebido e fidelidade do visual.

## Gates de aceitação

- [ ] Compilação Windows + `-SomenteTesoura` PASS.
- [ ] Suíte completa `-Abrir` PASS.
- [ ] Nós J, dois destaques, fechamento, zoom, corte, Ctrl+Z/Y sem regressão visual.
- [ ] Seleção, Nós, troca de ferramenta e limpeza sem fantasmas visuais.
- [ ] Diagnóstico comparativo sem aumento consistente de memória/pintura e ganho reproduzível em repinturas menores.
- [ ] Laser desconectado; nenhuma emissão, movimento ou conexão física liberados.

**Status:** código publicado para auditoria; compilação Windows, benchmark comparativo e homologação visual da 20zb pendentes. **Não mesclar em `main` nem sobrescrever `APLICATIVO` até passar nos gates.**
