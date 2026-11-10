# Alencar Laser — Tesoura 20u (candidata experimental, 10/11/2026)

## Objetivo
Ampliar de forma conservadora a Tesoura 20t seguindo a semântica de **Trim Shapes** do LightBurn:
https://docs.lightburnsoftware.com/2.2/Reference/TrimShapes/

## Implementado nesta etapa
- O planner distingue um cruzamento **transversal verdadeiro** sobre o vértice interno de outro contorno: os dois segmentos adjacentes devem estar de lados rigorosamente opostos da reta-alvo.
- Nesse caso, o ponto entra na lista de limites de corte, preservando a prévia e a operação de exclusão por intervalo existentes.
- Tangências (segmentos do mesmo lado), encontros T com ponta de linha aberta, extremos do alvo, segmentos degenerados e sobreposições colineares continuam recusados.
- A classificação usa apenas geometria em memória e não muda coordenadas, arquivos, IDs, rotas de emissão, camadas ou histórico durante o hover.
- Os testes de núcleo adicionados cobrem cruzamento em vértice, dois limites, preservação da linha de referência, Ctrl+Z, tangência e junção T.

## Comparação com LightBurn — ainda pendente
- LightBurn pode unir automaticamente caminhos após Trim Shapes, e Ctrl desabilita essa união. **Esta branch ainda NÃO implementa união automática**. A próxima subetapa só deverá acrescentar união se puder preservar o desenho no mesmo estado de Undo, com um Ctrl para desligar e sem mesclar grupos, camadas diferentes, fontes editáveis nem vértices de grau >2.
- Shift para preservar o segmento removido em outra camada também permanece fora desta subetapa.
- Não forçar eliminação quando o hover é ambíguo. Não estender aproximações para cruzamentos quase coincidentes sem tolerância validada.

## Gates de validação
1. Confirmar branch experimental, sem alterações locais conflitantes; máquina desconectada.
2. Executar `powershell -NoProfile -ExecutionPolicy Bypass -File .\SCRIPTS\TESTAR_19_8_WINDOWS.ps1 -SomenteTesoura`.
3. Executar a suíte completa sem `-SomenteTesoura` após PASS.
4. Testar manualmente o hover em cruzamento de vértice, contato tangente, ponta em T e desenhos densos importados; comparar prévia com corte efetivo.
5. Confirmar Ctrl+Z único e arquivo original intacto; não mesclar em main até inspeção visual.
6. Comparar P95 de pintura e latência em projetos equivalentes (575/1737 contornos), sem alegar ganhos sem medição pareada.

Status: código e testes publicados no GitHub; ainda não compilados nem homologados no Windows nesta branch.
