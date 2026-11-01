# Alencar Laser — Gerador de Engrenagens (19.9 experimental)
Data: 01/11/2026
Status: implementação de interface e núcleo offline; requer Windows Qt + avaliação visual e dimensional.

## Uso
1. Abrir a candidata experimental, com controle físico bloqueado.
2. Em `Gerar > Engrenagem paramétrica (mm)...`, escolher número de dentes, módulo, ângulo de pressão (20° é um valor de referência comum), furo central e centro X/Y.
3. Conferir o diâmetro externo, o círculo primitivo tracejado, limites da mesa e a prévia.
4. Confirmar para inserir dois contornos fechados agrupados (se o furo for diferente de zero), em uma operação reversível com Ctrl+Z.
5. Ajustar contornos, exportar ou salvar projeto pelo fluxo editorial existente. Não transmitir à máquina a partir deste teste.

## Considerações técnicas
- Diâmetro primitivo = módulo × número de dentes.
- Raio externo = raio primitivo + módulo; raio de raiz = raio primitivo - 1,25 × módulo. Ângulo de pressão influencia o raio de base.
- Perfil evolvente aproximado por segmentos: bom para prototipagem e inspeção de geometria, **ainda não** dimensionado para transmissão mecânica de precisão. Deve-se medir a largura real de corte (kerf), acrescentar folgas e testar um protótipo antes de produzir peças acopladas.
- A forma externa e o furo são contornos distintos; use Linha, não preenchimento, ao preparar futuro corte. A sequência física correta dos caminhos e intertravamentos **não estão homologados**.
- Compatibilidade e montagem de pares (mesmo módulo e ângulo de pressão, centro a centro, interferência, engrenamento) serão objeto de testes futuros.

## Pendências
- Validar CI Qt Windows, DPI, mouse, Undo/Redo e persistência em arquivo de cópia.
- Refinar os dentes com raios de concordância, tolerâncias/kerf e perfis paramétricos reeditáveis após reiniciar.
- Evoluir para pares engrenados, engrenagem interna, cremalheira e layouts de MDF 3 mm.
- Manter máquina, controladora e emissão bloqueadas até homologação específica.

## Referência funcional (não reutilizar nem distribuir o código GPL do Inkscape)
- https://wiki.inkscape.org/wiki/Extension_reference
- https://inkscape.gitlab.io/inkscape/doxygen-extensions/render__gears_8py_source.html
