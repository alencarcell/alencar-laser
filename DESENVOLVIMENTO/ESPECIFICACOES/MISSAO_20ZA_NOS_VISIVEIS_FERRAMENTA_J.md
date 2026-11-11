# Missão Alencar Laser — 20za | Nós visíveis no corte entre nós (J)
Data: 10/11/2026
Base: `feat/corte-entre-nos-painel-objetos-20z-20261110`
Branch experimental: `feat/corte-entre-nos-nos-visiveis-20za-20261110`

## Objetivo e motivação
Após testar a 20z, o operador informou que a ferramenta **J — Entre nós** ficou boa, mas os nós precisam aparecer durante o corte. A alteração é **somente visual**: não modifica a lógica já validada de `Editor::removePolylineEdge`, os dados do desenho ou as operações de laser.

## Comportamento implementado
- Quando o mouse encontra uma aresta editável de polilinha, a aresta fica **ciano (#22D3EE)**, como na 20z.
- As **duas extremidades** da aresta aparecem como círculos cianos de raio 5 pixels com contorno claro.
- Os demais nós do contorno aparecem como pontos menores (raio 2,8 pixels) com azul acinzentado translúcido.
- Em contornos com até **50 nós**, todos os outros nós aparecem; para os maiores, o código limita os nós adicionais a aproximadamente **49 nós vizinhos** da aresta, preservando a fluidez.
- Cada marcador usa `QGraphicsItem::ItemIgnoresTransformations`: mantém tamanho em pixels durante zoom, sem deslocar a coordenada real do nó.
- Os marcadores não aceitam eventos do mouse, não entram no documento e **não modificam o histórico**.
- Os marcadores só existem enquanto houver hover sobre aresta válida no modo J. São removidos ao sair da viewport, mudar de ferramenta, invalidar a geometria ou reconstruir a cena.
- Se o mouse se move dentro da mesma aresta, a sobreposição é reutilizada, evitando recriação de dezenas de itens a cada frame.
- Contornos fechados mostram os dois extremos corretos da aresta de fechamento (`último -> primeiro`).
- Tesoura K (recorte entre cruzamentos), Ctrl+Z, camadas, grupos, curvas Bézier e formato `.alaser v5` permanecem sem mudanças.

## Critérios de aceitação
1. Linha aberta com 4 nós: J destaca 1 aresta e mostra **2 nós fortes + 2 discretos**; o clique remove apenas aquela aresta.
2. Zoom para cima/baixo: marcadores mantêm tamanho legível em pixels; o contorno e os nós reais não são deslocados.
3. Ao sair do canvas ou mover para ponto sem aresta, não fica marcador fantasma.
4. Em desenho denso, só aparecem nós de um contorno, limitados ao entorno da aresta, sem poluição do projeto inteiro.
5. Corte, novo corte, Ctrl+Z e Ctrl+Y mantêm geometria e histórico consistentes.
6. Contorno fechado: o fechamento usa último/primeiro nó, sem selecionar uma aresta diferente.
7. A opção de ferramenta J e o painel Objetos da 20z continuam funcionando. Máquina permanece desconectada.

## Regressões Qt incorporadas
No teste GUI `--edit-tools-19-7-test`:
- quatro marcadores no contorno sintético de quatro nós;
- flags de tamanho fixo em pixels;
- hover deslocado na mesma aresta não perde/duplica nós;
- saída da viewport limpa prévia e marcadores;
- reentrada e hover reconstroem a marcação;
- clique/corte e rebuild removem marcadores antigos.

## Validação no Windows
Estando em `C:\ProjetoAlencarLaser`, preservar alterações locais antes de trocar de branch:

```powershell
git status --short
git fetch origin
git switch --track origin/feat/corte-entre-nos-nos-visiveis-20za-20261110
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1" -SomenteTesoura
```

Se o PASS focado ocorrer, rodar a suíte completa:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ".\SCRIPTS\TESTAR_19_8_WINDOWS.ps1"
```

Depois abrir no modo interativo pelo script existente com `-Abrir` e testar J no desenho real. Não homologar antes da inspeção visual. Não efetuar merge em main nem substituir a versão instalada.

## Estado
Código e testes publicados em branch experimental. **Compilação e homologação visual no Windows ainda pendentes.**
