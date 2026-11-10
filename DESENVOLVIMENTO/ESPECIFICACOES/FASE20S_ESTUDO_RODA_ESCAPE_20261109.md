# Alencar Laser — Fase 20S: estudo geométrico da roda de escape

Data: 09/11/2026. Branch experimental `feat/20s-escape-wheel-study-20261109`, derivada da fase 20R de engrenagens angulares e zoom.

## Objetivo e fronteira de segurança
Abrir uma ferramenta **separada** no menu Gerar → Engrenagens → Roda de escape — estudo geométrico (OFFLINE). A prévia contém uma roda com dentes assimétricos e referências para o pivô e dois pontos da âncora. **O arquivo do editor recebe apenas o contorno da roda e, se solicitado, o furo do eixo.**

A geometria não foi validada para escapamento Graham completo. Posição de pivô, arco de referência e palhetas são *linhas de estudo*, NÃO peças aprovadas nem linhas de corte. Não há simulação real de travamento, impulso, queda, força, contato ou dinâmica do pêndulo.

## Referências técnicas
- Rainer Hessmer, `https://hessmer.org/gears/DeadbeatEscapementBuilder.html`: referência independente de gerador geométrico de escapamento, que explicita diâmetros, número de dentes, ângulos das faces, vão das palhetas, lock/drop/lift e distância entre eixos.
- J. L. Tigner, *Horological Times*, setembro/1978, `https://www.awci.com/wp-content/uploads/ht/1978/1978-09-web.pdf`: exemplo de escapamento morto quadrado com roda de 30 dentes, arco de 7,5 passos e distância nominal de pivôs de aproximadamente 1,414 × raio externo.
- Jerome Faier, *Horological Times*, março/2016, `https://www.awci.com/wp-content/uploads/2016/03/HTMarch2016-NonMembersOnly.pdf`: diferença técnica entre face de bloqueio e face de impulso na palheta e por que um ângulo único não determina o funcionamento.

Esta implementação é cálculo independente de estudo; não copia o algoritmo de terceiros.

## Geometria implementada
- Padrão: 30 dentes, diâmetro externo 80 mm, raiz 75% do raio externo, furo Ø6 mm, espessura angular de ponta 0,6°, offsets das faces 4,2°/2,6°, arco nominal entre referências da âncora de 7,5 passos; rotação horária.
- Cada dente tem 5 vértices (raiz inicial, duas pontas externas, raiz final, ponto médio de vale). Logo, 30 dentes geram 150 nós no contorno exterior, com 96 nós para o furo opcional.
- Rotação anti-horária inverte o viés das faces, sem inverter o sentido do contorno vetorial.
- Referências: arco entre dois pontos simétricos da circunferência externa, pivô no eixo vertical a `2*R*cos(spanAngle/2)`. Com 30 dentes e arco 7,5 passos, o valor é `sqrt(2)*R`. Não afirmar que o desenho das palhetas foi derivado: **não foi**.
- Rejeita formatos fisicamente impossíveis da prévia (diâmetros/raios inválidos, raízes sobrepostas, espessura de ponta que invade faces, furo muito largo, amplitude de palhetas fora do intervalo).
- Projeto exportável: **roda e furo somente**. Guia de pivô, palhetas e arco permanecem exclusivamente na prévia, com coloração distinta e avisos explícitos.

## Interface
`DESENVOLVIMENTO/FONTE/qt/escapement_study_dialog_20s.hpp`: seletor de dentes, diâmetro, raiz, furo, espessura angular de ponta, faces anteriores/posteriores, vão em passos, sentido de rotação, centro X/Y. Prévia com zoom por roda/botões, pan por arraste e Ajustar.

Os controles ficam num painel rolável para evitar corte da prévia. O botão de criação fica desabilitado quando a configuração é inválida.

## Desempenho
A fase 20S apenas **mede** o tempo `QWidget.grab` da prévia em modo offscreen (24 amostras; P95 registrado) para futuras comparações pareadas. Não altera o rasterizador principal, o cache ou a política de viewport; logo não há uma nova alegação de aceleração. O relatório anterior do operador tinha paint P95 ~196ms para aproximadamente 13.493 nós com estratégia `bounding_rect`; esta não é uma medida pareada com a prévia isolada.

## Validação preparada (ainda pendente no Windows)
Script `SCRIPTS/TESTAR_RODA_ESCAPE_20S_WINDOWS.ps1` configura e compila numa pasta de build exclusiva e executa 8 gates:
- 6 regressões anteriores 20R (involuta, angular, ângulo medido, GUI legada, angular e zoom);
- teste C++ novo de roda de escape, 15/24/30/45/60/90 dentes, duas direções, raízes/furo e guias;
- teste Qt novo de prévia, rotação, zoom, inserção somente de roda e furo, Undo/Redo, zero linhas-guia exportadas e métrica P95.

Lançador `SCRIPTS/ABRIR_ESTUDO_RODA_ESCAPE_20S_WINDOWS.ps1` só abre depois de 8 gates verdes na mesma revisão Git e SHA256 do executável.

### Executar (PowerShell)
```powershell
cd C:\ProjetoAlencarLaser
git fetch origin
if ($LASTEXITCODE -ne 0) { throw 'Falha ao atualizar refs' }
git switch feat/20s-escape-wheel-study-20261109
if ($LASTEXITCODE -ne 0) { throw 'Troca de branch bloqueada; nao force' }
git pull --ff-only origin feat/20s-escape-wheel-study-20261109
if ($LASTEXITCODE -ne 0) { throw 'Falha ao atualizar branch' }
powershell -NoProfile -ExecutionPolicy Bypass -File '.\SCRIPTS\TESTAR_RODA_ESCAPE_20S_WINDOWS.ps1' -Abrir
```

## Próximos gates de engenharia para relógio funcional
1. Construção geométrica completa da âncora: palhetas de entrada e saída, faces de bloqueio e impulso; pivôs e ângulos de oscilação.
2. Simulação cinemática (marcha, queda, bloqueio, impulso) com detecção de colisão antes de liberar o corte.
3. Folga de funcionamento, raio mínimo/espessura efetiva de ponta, tolerância do MDF de 3 mm, compensação da largura de corte e orientação das fibras do MDF.
4. Teste com **cópias** dos projetos e só depois máquina desconectada → teste de corte controlado com aprovação explícita do operador.

Estado real: **código publicado**, sem compilação nem homologação Windows desde a criação desta branch. Instalação principal, main, bibliotecas de hardware e arquivos de projeto originais permanecem intactos. Não criar instalador de produção com esta fase.
