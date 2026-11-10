# Fase 20c — cache vetorial experimental (10/11/2026)

## Evidência da Fase 20b
- 6 rodadas controladas, 575 contornos, 144 movimentos por rodada.
- P95 paint OFF 29,9839 ms / ON3 29,6815 ms; **não atribuir aceleração a ON3**, pois ambos fazem 575 chamadas de pintura por quadro e repintura bbox P95 de 100%.
- P95 vetorial OFF 16,9274 ms / ON3 16,7388 ms. Sonda introduz overhead e não representa produção.
- Dez contornos somam ~18,2% dos tempos acumulados; não há gargalo exclusivo nos maiores itens.

## Mudança no editor Qt
- Variável `ALENCAR_VECTOR_DEVICE_CACHE_20C=1` ativa `QGraphicsItem::DeviceCoordinateCache` **apenas** em contornos visíveis, não selecionados, cuja caixa projetada inicial é de até 256×256 pixels físicos.
- Orçamento estimado de 16 MiB por reconstrução da cena; só contornos visíveis no viewport inicial entram.
- Estado padrão OFF, sem mudanças no modelo geométrico, `.alaser`, undo/redo, importação ou transporte para o laser.
- Se a sonda `ALENCAR_VECTOR_PAINT_PROBE_20=1` estiver ativa, o cache 20c é desativado para evitar atribuições incoerentes dos resultados por item.
- Qt invalida/atualiza o cache do item conforme sua política; zoom, mudanças de estilo e geometria exigem teste visual no Windows. A estimativa de orçamento não é limite de memória física garantido.
- Este experimento **não** implementa repintura seletiva. Só experimenta cache de apresentação e ainda pode repintar todos os contornos por quadro.

## Testes obrigatórios antes de qualquer merge
1. Conferir `git status --short`; preservar alterações locais.
2. Trocar para `exp/fase20c-cache-vetorial-20261110` depois de `git fetch origin`.
3. Com laser desconectado, compilar no Windows com o script existente `SCRIPTS/TESTAR_19_8_WINDOWS.ps1`, sem instalar em APLICATIVO.
4. Comparar a mesma arte em OFF (`Remove-Item Env:ALENCAR_VECTOR_DEVICE_CACHE_20C -ErrorAction SilentlyContinue`) e ON (`$env:ALENCAR_VECTOR_DEVICE_CACHE_20C='1'`), em sessões novas, com resolução/zoom idênticos. Repetir 3 vezes cada e usar medições sem `ALENCAR_VECTOR_PAINT_PROBE_20`.
5. Verificar visualmente ausência de rastros e desaparecimento de elementos durante seleção, movimento, rotação, zoom, pan, Tesoura, exclusão, Ctrl+Z/Y e mudança de camadas.
6. Medir mediana/P95/P99 de pintura, entrada→pintura, RAM e bbox; usar arquivos de 40 e 575 contornos e um terceiro de 1000+ **somente se disponível**, sem inventar arquivos.
7. Reprovar o experimento por qualquer alteração visual, falha funcional, aumento significativo de RAM ou regressão perceptível. Em caso de falha, retornar OFF.
8. Não homologar, instalar, ligar máquina, fazer merge em main ou liberar G-code por este experimento.

## Limitações conhecidas
A seleção de candidatos ao cache é calculada em cada `rebuild()`. Contornos fora da viewport na reconstrução inicial não recebem cache, mesmo quando entram na vista depois de pan. É uma política conservadora deliberada para teste, não a solução definitiva de cache.
