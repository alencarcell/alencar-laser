# Correção 20w — prévia laranja versus clique (10/11/2026)

## Evidência de bancada
Capturas: prévia laranja visível ao usar Tesoura em contornos densos; clique ocasionalmente não corta.
JSON opt-in: 13 `scissor_continuous_span_removed`, 4 `scissor_no_safe_span` (três `vertex_or_tangent`, um `single_closed_intersection`), um `miss`; 1043 hovers; 567 contornos no resumo final; máquina desconectada.

## Causa de interface observada no código
`updateScissorHover()` já calcula e apresenta um `ScissorSpan` seguro. Antes da 20w, `mousePressEvent()` repetia sempre o hit-test na mesma posição. Se houver sobreposição com mais de um contorno, a ordenação e o alvo reavaliado podem não corresponder ao destaque visível.

## Ajuste
- Se a posição do clique difere no máximo 1 pixel da última posição de hover, a geometria não mudou (mesma `editor.generation`), a Tesoura segue ativa e a prévia laranja está presente, utiliza-se **o mesmo intervalo seguro exibido**.
- Caso contrário, o código recalcula a prévia e mantém as recusas existentes. Não força corte em vértice, tangência, sobreposição ou anel com interseção insuficiente.
- O corte continua na mesma transação de Undo (incluindo a união opcional do 20v); Ctrl ainda suprime a união.
- Regressão Qt: verifica que prévia do mesmo ponto pode ser reutilizada, mas prévia de outro ponto não.

## Limites
Este ajuste resolve uma inconsistência **potencial** entre prévia e alvo do clique, mas os 4 casos de recusa no JSON não provam que todos eram causados por nova busca. Três são topologia de vértice/tangência e um é anel com um único cruzamento. Não afirmar equivalência total com LightBurn.

## Validação
1. Compilar e executar `SCRIPTS/TESTAR_19_8_WINDOWS.ps1 -SomenteTesoura`.
2. Executar suíte completa após PASS.
3. Abrir GUI com ambiente Qt configurado, repetir no MESMO desenho e cursor a tentativa de corte laranja, confirmando objeto, antes/depois e Undo.
4. Testar hover seguido de pan/zoom/mudança de documento: uma prévia desatualizada jamais pode ser aplicada.
5. Sem merge na `main`, sem sobrescrever desenhos, sem conexão laser antes da homologação.

Status: alteração publicada em branch; compilação e inspeção visual Windows pendentes.
