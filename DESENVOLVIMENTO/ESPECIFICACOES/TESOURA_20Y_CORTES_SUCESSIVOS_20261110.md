# ALENCAR LASER — Tesoura 20y: cortes sucessivos independentes
Data: 10/11/2026. Base: `fix/tesoura-preview-estado-20x-20261110`.

## Sintoma real
O usuário consegue cortar linhas que se cruzam, mas depois de um recorte a Tesoura às vezes recusa outro trecho intersectado, como se a geometria estivesse agrupada. O diagnóstico anterior apresentou principalmente `vertex_or_tangent`, não `grouped`. Não presumir bloqueio por grupo.

## Duas causas específicas no código
1. Após o primeiro Trim, os restos de caminho abertos terminam exatamente na interseção anterior. A 20x classifica vários destes **terminais em T** como contato ambiguamente tangente, criando uma zona `unsafe` que bloqueia o próximo intervalo mesmo quando outro cruzamento o delimita.
2. A GUI 20v/20x chama `removeScissorSpan(cutting, true)` para todo clique normal, podendo unir caminhos independentes na mesma forma se as pontas coincidirem. Isso não cria um *grupo*, mas muda a identidade/topologia e pode fazer o usuário perceber as duas linhas como uma única figura.

## Alterações experimentais
- No planejador, a ponta de outro caminho aberto no INTERIOR da linha que se deseja cortar passa a contar como limite de corte exato, apenas quando a ponta física coincide com a projeção até `1e-8 mm`. Pontos de outros caminhos que sejam vértices internos ou quase encontros não recebem essa regra. Sobreposição colinear continua insegura.
- Quando a ponta do PRÓPRIO alvo aberto está sobre outra linha, não é criada zona artificial insegura: o início/fim já é uma fronteira natural para o Trim.
- Cliques normais preservam as figuras independentes. `Alt+clique` solicita a união conservadora opcional da 20v; `Ctrl` sempre a desativa. Os caminhos nunca são agrupados pela Tesoura.
- A prévia segura continua em ciano, objetos apenas selecionados continuam laranja, e o mesmo intervalo anunciado é usado no clique.
- Nenhum movimento, emissão ou transporte a hardware foi habilitado.

## Regressões adicionadas
- Figura horizontal cruzada por quatro verticais: 1º corte entre duas interseções, 2º corte na vertical que passa a tocar uma extremidade aberta, 3º corte no mesmo resto horizontal entre outro cruzamento e a ponta; teste simétrico no resto direito.
- Preservação de todos os objetos de referência e metadados desagrupados.
- Undo de cada passo e restauração completa da geometria após vários Ctrl+Z.
- T exato recebe fronteira; *near miss* não é inventado como cruzamento.
- A regra já existente continua rejeitando tangências verdadeiras (duas arestas do mesmo lado), nós degenerados e sobreposições.
- Teste Qt verifica que normal/Ctrl não unem e apenas Alt isolado solicita união.

## Gates antes de incorporar à versão oficial
1. Rodar `powershell -NoProfile -ExecutionPolicy Bypass -File .\SCRIPTS\TESTAR_19_8_WINDOWS.ps1 -SomenteTesoura` com máquina desconectada.
2. Depois do PASS, rodar a suíte completa.
3. Abrir a candidata e repetir manualmente o desenho que falhava: cortar a primeira linha, depois a segunda, depois um resto da primeira; observar prévia ciano, resultado e Ctrl+Z. Normal não pode mesclar figuras.
4. Verificar que Alt só une quando pontas forem exatas, únicas, mesma camada, fora de grupos e de texto editável.
5. Não mesclar em `main`, não instalar em `APLICATIVO` e não sobrescrever o desenho original até homologação visual.

Status: código e regressões foram publicados no GitHub; ainda não compilado/testado no Windows nesta branch.
