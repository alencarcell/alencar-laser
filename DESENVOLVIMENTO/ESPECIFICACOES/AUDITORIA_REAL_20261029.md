# Auditoria real Alencar Laser 19.8 — 29/10/2026

## Evidencias do usuario

- Sessao diagnostica Qt 6.12.0 no Windows, 1.600.849 ms, 74 eventos e zero descartados; maquina desconectada e zero sondagens.
- 7 movimentos Bézier de contorno; 4 de nos; 1 texto vetorial; 1 rastreio de imagem; 10 recusas da Tesoura por formas agrupadas, 6 sem deteccao de linha e 2 cortes registrados, sem prova de que o trecho visual completo foi removido.
- Desempenho: paint P95 51,0583 ms; P99 70,6192 ms; max 415,543 ms; input-to-paint P95 37,1046 ms; wheel P95 0,7122 ms; RSS 132.038.656 bytes.
- O arquivo real enviado (nao incorporado ao GitHub) e um .alaser v5 experimental com 597 contornos polilinha, 16.691 nos, 13 contornos Bézier, um texto original reeditavel e dois modos de camada (Line/Fill). De 576 caminhos marcados como abertos, 571 repetem exatamente o primeiro no ao final. Nao existe imagem bitmap binaria no pacote.
- Previa experimental: 8.349 movimentos, 3.277,8 mm de percurso de gravacao virtual, 1.229,8 mm de viagem otimizada, 76,1 s nominal e 322,5 s segundo modelo de aceleracao simplificado. Nao corresponde a tempo fisico homologado.

## Defeitos e prioridades

P0. Zoom precisa funcionar durante Linha continua e Desenhar Bézier, sem corromper nos.
P0. Selecao deve ser unificada para polilinha, curvas Bézier, texto e imagem, inclusive caixa, arraste, undo/redo, escala e camadas. A arquitetura atual distingue sessao nativa e editor de polilinhas.
P0. Tesoura nativa hoje divide a aresta, mas nao remove um intervalo completo com destaque seguro; nao tratar isso como recorte correto. Regras topologicas conservadoras e teste com arquivo real necessarios.
P0. Bitmap original atualmente e overlay de sessao: nao se seleciona, nao e objeto com historico e NAO e persistido no .alaser. Corrigir modelo de objeto, imagem binaria com hash no pacote v5, undo, posicao e escala, roundtrip; preservar compatibilidade com v5 legado e nao alegar sucesso de salvamento enquanto nao houver bitmap persistido.
P1. Fotogravacao OFFLINE: imagem original e processada lado a lado, brilho/contraste/gama/negativo, limiar, tramas ordenadas/difusao de erros, largura em mm, intervalo/DPI, melhorias de tracado; nenhuma emissao fisica.
P1. Previa pixelada: imagem atual de 740x440 e ampliada como bitmap. Hotfix de 2x supersampling/adaptacao ate 30 mil movimentos e interpolacao suave e apenas transicao; arquitetura final deve renderizar trajetos vetoriais por zoom e cache, medindo fluidez.
P1. Texto direto: clique em texto existente deve reabrir texto/fonte, sem duplicar; converter texto em caminhos e remover metadados de origem, com Undo unico. Outros contornos ja sao caminhos.
P1. Rotacao: desenhar caixa orientada e alcas que acompanham angulo durante e depois da rotacao; AABB atual nao satisfaz.
P2. Inserir Offset, Soldar, Uniao, Subtracao, Intersecao, Espelhamento e Distribuicao X/Y; rotulos e mensagens integralmente em portugues.

## Referencias verificadas

- https://docs.lightburnsoftware.com/legacy/BasicUsageEssentials
- https://docs.lightburnsoftware.com/latest/Reference/Selection/
- https://docs.lightburnsoftware.com/latest/Reference/TransformControls/
- https://docs.lightburnsoftware.com/latest/Reference/Text/
- https://docs.lightburnsoftware.com/latest/Reference/ConvertToPath/
- https://docs.lightburnsoftware.com/latest/Reference/TraceImage/
- https://docs.lightburnsoftware.com/latest/Reference/AdjustImage/
- https://docs.lightburnsoftware.com/2.1/Reference/CutSettingsEditor/ImageMode/
- https://docs.lightburnsoftware.com/2.1/Reference/Preview/

## Primeira fase de codigo

Branch de trabalho isolada fix/validacao-real-19-8-20261029 derivada de feat/refino-curvas-rastreio-diagnosticos-19-8. Implementacoes experimentais nesta branch:
- Zoom durante desenho de linha e curva, com reconstrução apenas da pre-visualizacao e sem mexer nos nos do documento;
- Clique direto no interior de texto reeditavel, recuperando frase e fonte;
- Botao e atalho Ctrl+Shift+C para converter texto em caminhos individuais, preservando geometria e Undo;
- Supersampling adaptativo 2x na previa, com smooth scaling;
- Regressoes portateis e GUI de zoom, reedicao e conversao.

STATUS: alteracoes de fonte publicadas, mas NAO compiladas/testadas no Windows apos este lote. Main, APLICATIVO e base 19.7.1 preservados. Laser fisico desconectado, proibido enviar movimento/emissao. Validar -SomenteEdicao, -SomenteInteracao, -SomenteDesenho e -SomenteSimulacao antes do gate completo -Abrir. Usar copia do .alaser real e relatorios JSON para revisao posterior.


## Correção focal de texto e pesquisa LightBurn — 29/10/2026

### Relato observado no Windows
- Ao pedir reedição, os contornos anteriores permanecem visíveis atrás do campo de texto.
- Alterar o zoom ou rolar a mesa causa aparente salto do texto em edição, porque o QLineEdit pertence ao viewport (coordenadas em pixels), mas o objeto é armazenado no documento em mm. O campo antigo era posicionado apenas uma vez e ainda recebia clamp aos limites da janela.
- O usuário quer o **ponto azul de curvar texto** do LightBurn: arrastar cria arco circular, duplo clique redefine curvatura; texto continua editável. Não confundir com Converter para caminho, que perde origem textual.

### Correções no código da branch de trabalho
- Durante reedição, os IDs dos componentes do texto original ficam invisíveis apenas na cena, inclusive no Fill composto. Nenhum contorno, grupo, metadado ou Undo é excluído. Concluir/cancelar restaura estado visual; Escape cancela o rascunho.
- A origem do QLineEdit continua fixada nas coordenadas de documento e se reposiciona a cada alteração de zoom, barras de rolagem ou tamanho do viewport. Retirada a lógica antiga de clamp, que deslocava indevidamente a posição relativa; a formatação realimenta o mesmo cálculo.
- Testes GUI ampliados para verificar: texto anterior oculto durante edição, Zoom/Scroll sem mudar âncora ou Undo, Escape restaura contornos e mantém o projeto.
- **Status:** ajustes de código conferidos estaticamente no GitHub, sem compilação/execução Windows depois desta rodada. Continuar em PR #12 rascunho; não mesclar nem instalar no APLICATIVO.

### Curvar texto: desenho do recurso, ainda não implementado
Referência oficial:
- https://docs.lightburnsoftware.com/2.2/Reference/Text/ — Text Bend/Bending Text: ponto azul quando texto selecionado; arraste vertical curva ao redor de círculo; duplo clique restaura; opção Distort altera se glifos são deformados ou apenas orientados.
- https://docs.lightburnsoftware.com/2.0/Reference/ApplyPathToText/ — texto aplicado a caminho Bézier/vetor é uma função distinta.
- https://docs.lightburnsoftware.com/latest/Reference/ConvertToPath/ — converter em caminho cria vetores editáveis, perdendo propriedade de texto.

Implementar em fase posterior: (1) texto permanece tipo de objeto editável com frase, fonte, espaçamento, ancoragem e curvatura no metadata; (2) alça azul visível apenas na seleção de texto, drag gera preview em arco com centro/raio e limites seguros; (3) opção **Deformar letras** separada de **Manter letras sem deformação**; (4) double-click na alça desfaz curvatura, Ctrl+Z/Y reverte em uma transação; (5) salvar/reabrir .alaser v5 sem perder texto ou raio; (6) opção Aplicar texto a caminho com contorno vetorial escolhido; (7) **Converter em caminhos** congela a geometria atualmente curva e remove origem textual, com um Undo reversível. Testar pontuação, espaço, caracteres acentuados, várias fontes e colisões/hole Fill. Interface totalmente PT-BR. Não criar alça interativa sem pipeline completo de geometria e persistência.
