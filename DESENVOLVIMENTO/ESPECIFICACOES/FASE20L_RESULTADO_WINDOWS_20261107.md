# Fase 20l — resultado Windows (07/11/2026)

## Evidência recebida
O operador executou `SCRIPTS/TESTAR_VIEWPORT_575_20L_WINDOWS.ps1` no Windows com Qt 6.12, 575 contornos, 12 zooms por teste e 3 pares alternados OFF/ON.

| Par | OFF P95 ms | ON P95 ms | redução porcentual | OFF bbox P95 | ON bbox P95 | SHA imagens |
|---|---:|---:|---:|---:|---:|---|
| 1 | 28,5777 | 28,2715 | 1,07% | 1 | 1 | igual |
| 2 | 49,6627 | 28,8298 | 41,95% | 1 | 1 | igual |
| 3 | 38,1910 | 30,8625 | 19,19% | 1 | 1 | igual |

Mediana dos três valores OFF = 38,191 ms; mediana dos três ON = 28,8298 ms, aproximadamente 24,51% menor. Esse valor **não** mede ganho global do produto e não é uma inferência estatística robusta (3 pares, amostras pequenas). A caixa de pintura P95 permaneceu 100% nos dois modos. O ganho potencial não pode ser explicado simplesmente por diminuição da bbox. O monitoramento de pixels e histórico passou. Não há tempo de interação real OFF/ON nessa captura.

## Decisão
**CANDIDATA A VALIDAÇÃO MANUAL — NÃO PROMOVER NEM ATIVAR POR PADRÃO.** É a primeira troca de política de viewport nesta linha de investigação com P95 ON menor em todos os pares e hashes iguais. Necessário A/B no editor com mouse físico, seleção, Tesoura, zoom e pan, com o mesmo fixture e DPI, além de repetir regressões 7/7 e testar cópias de arquivos reais. Não conectar o laser. Não confundir redução sintética com prova de causa definitiva.

## Próximo gate
Comparação real no Windows, OFF e ON, usando `ALENCAR_MINIMAL_VIEWPORT_20L` e `ALENCAR_EVENT_PAINT_TRACE_20J` ligado apenas na coleta de diagnóstico. JSON individual do painel de Diagnósticos em cada sessão; medir P95 e P99 de zoom/seleção/transformação, amostras, canvas visual, fidelidade manual, sem usar P95 global isolado para concluir. O tracer adiciona overhead, mas o custo é semelhante entre modos. Não divulgar desenhos nem relatórios brutos.
