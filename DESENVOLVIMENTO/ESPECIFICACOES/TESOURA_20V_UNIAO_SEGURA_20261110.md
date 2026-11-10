# TESOURA 20v — União conservadora após Trim Shapes

Data: 2026-11-10. Base: `feat/tesoura-topologia-20u-20261110`.

## Contrato funcional experimental
- O editor recebe `removeScissorSpan(span, autoJoin)`; mantém default `false` para compatibilidade de chamadas existentes.
- Na GUI da Tesoura poligonal, o clique normal ativa a tentativa de união; **Ctrl+clique** desativa. O recorte e a tentativa de união acontecem na MESMA `change()` e no mesmo Ctrl+Z.
- Só é elegível um extremo recém-exposto pelo recorte, com outro extremo **exatamente coincidente** (tolerância 1e-9 mm: não é snap).
- Caminhos devem ser abertos, individuais, desagrupados, editáveis, na mesma camada, e não pertencer a texto editável.
- Somente duas extremidades podem se encontrar; um terceiro caminho cruzando o ponto também bloqueia união.
- No máximo uma união por clique; demais pares exigem operação separada. Sem junção, o corte continua como na 20u.
- Arestas Bézier nativas não passam por esse caminho; formato ALASER v5, parâmetros de máquina e metadados de camada permanecem intocados.

## Limitação funcional
- A versão 20u recusa vários encontros em T como limites de corte. Assim, esta subetapa pode não unir desenhos que o LightBurn uniria, mesmo tendo extremidades coincidentes. **Não afirmar equivalência funcional ao LightBurn**.
- Não há inserção de nó em cruzamento interior, snap automático, união de três caminhos, união entre camadas ou preservação Shift do trecho eliminado.
- A inspeção visual no Windows é obrigatória para avaliar se a conexão real é desejável no tipo de arte usado.

## Testes acrescentados
`tests/test19_8_scissors_span.cpp`: cenário sintético de intervalo já definido para testar isoladamente união pós-corte em um único Undo, restauração integral por Ctrl+Z, Ctrl sem união e isolamento entre camadas. A validação do planner em T permanece separada.

## Passos antes de homologar
1. `powershell -NoProfile -ExecutionPolicy Bypass -File .\SCRIPTS\TESTAR_19_8_WINDOWS.ps1 -SomenteTesoura`
2. Após PASS, suíte completa sem switches.
3. Inspecionar com laser desconectado a prévia e o resultado em desenhos reais, Ctrl+clique versus clique, Undo/Redo, objetos agrupados e camadas bloqueadas.
4. Confirmar desempenho com arquivos de 575 e 1737 contornos e documentar eventuais aumentos.
5. Não alterar `main`, `APLICATIVO` ou arquivo original de usuário.

Status: código e testes publicados, compilação Windows ainda não realizada nesta branch.
