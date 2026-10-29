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
