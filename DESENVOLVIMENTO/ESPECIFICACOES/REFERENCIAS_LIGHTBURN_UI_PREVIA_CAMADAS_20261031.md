# Referências visuais LightBurn — lote de seis fotos (31/10/2026)

**Finalidade:** documentar referências de UX e comportamento observadas em fotos enviadas pelo operador, para comparação futura com **Alencar Laser Community**. **Não** implica cópia da aparência, código, imagens ou outros ativos proprietários do LightBurn.

**Fonte:** seis imagens anexadas na conversa de 31/10/2026: `1000898319.jpg`, `1000898318.jpg`, `1000898317.jpg`, `1000898316.jpg`, `1000898315.jpg` e `1000898314.jpg`. **As fotos não foram anexadas ao repositório.** Aguardar relatório posterior do LightBurn prometido pelo operador antes de transformar a referência em escopo de implementação.

## Inventário do que se vê

| Arquivo | Área do LightBurn visível | Observação ancorada na foto |
|---|---|---|
| `1000898319.jpg` | Janela `Visualizar` | Legenda `Corte` (preto) / `Mover` (vermelho), arte “Jéssica Alencar” com percursos vermelhos, linha do tempo de reprodução e controles inferiores; é referência de **leitura clara dos movimentos e cortes** |
| `1000898318.jpg` | `Configurações de corte` → Preencher | Velocidade `6000,0 mm/min`, potência máx. `20,00%`, modo `Preencher`, sobrevarredura `2,5%` / `2,50 mm`, preenchimento bidirecional; `Intervalo de linhas` `0,020 mm` e `1270,00 LPI`, ângulo `0°`, `1` passagem |
| `1000898317.jpg` | Mesmo painel, outro intervalo | Exemplo `Intervalo de linhas` `0,010 mm` e `2540,00 LPI`; opções visíveis de hachura cruzada e estratégias de preenchimento, sem comprovar quais foram ativadas ou executadas |
| `1000898316.jpg` | `Cortes/Camadas` e `Laser` | Linha de camada `C00`, modo `Preencher`, velocidade/potência `6.000 / 20`; chaves visíveis `Saída`, `Exibir` e `Ar`; resumo inferior de cor, passagens, velocidade/potência e intervalo `0,100 mm`; painel Laser com `Pausar`, `Parar`, `Iniciar`, enquadramento e G-code |
| `1000898315.jpg` | Barra superior e texto | Editor com campos de fonte/tamanho/espaçamento, alinhamento X/Y, rotação e ícones de ferramentas; referência para **organização compacta e previsível** |
| `1000898314.jpg` | Barra superior e área de desenho | Posição X/Y, largura, altura, cadeado de proporção, escala, âncora de nove pontos, unidades mm/in e contagem de pontos/formas. Não inferir a função exata dos ícones ilegíveis |

**Importante:** `0,010 mm` e `0,020 mm` são **exemplos observados no LightBurn**, não recomendações automáticas para qualquer laser/material; LPI derivado de `25,4 / intervalo_mm`. A capacidade física e a gravação do Alencar Laser dependem do equipamento, material e teste seguro. A faixa experimental do Alencar Laser atual (`0,050–5,000 mm`) é outra questão de projeto; não ampliá-la para `0,010 mm` apenas por copiar o screenshot (risco de volume computacional/percurso extremo).

## Comparação inicial com código do Alencar Laser 19.8 (31/10/2026)

Código consultado na branch privada `fix/curvas-tesoura-preflight-19-8-20261030`:

| Área | Recursos já identificados | Oportunidade a avaliar (não implementada aqui) |
|---|---|---|
| Prévia offline | Simulação de movimento, trajetórias, Fill/imagem, reprodução/pausa, barra de progresso, zoom, relatório JSON, opção de exibir viagens em vermelho, limitação virtual à mesa | Legenda de cores permanente e consistente; leitura rápida de corte vs. viagem; resumo amigável de distâncias e tempo; ergonomia dos controles |
| Camadas | Modos Line/Fill, potência nominal, velocidade virtual, M3/M4 ilustrativos, passagens, intervalo por camada e LPI teórico; paleta de camadas e controles de exibir/saída/ar | Inspeção da tabela de camadas com colunas e edição rápida, clareza dos estados e vínculo fiel entre linha selecionada, parâmetros e prévia |
| Fill virtual | Intervalo global e por camada, varredura bidirecional, overscan virtual, ordem interna, limites de mesa | Rever ângulo de varredura, hachura cruzada, agrupamento e estratégias de preencher formas; NÃO afirmar suporte atual sem teste; detectar descompasso entre configurações e prévia |
| Transformação e texto | Controles para posicionamento/alinhamento/rotação/fontes no editor experimental | Melhorar hierarquia visual de X/Y, largura/altura, escala, trava de proporções e ponto de referência |
| Operação Laser | Barra `OPERAÇÃO • BLOQUEADA`, controles físicos desativados na beta | Fotos do LightBurn são **somente referência de futura arquitetura**: nunca habilitar Iniciar, Enquadrar, Home ou G-code sem trilha própria de segurança GRBL |

## Proposta de prioridades quando o relatório chegar

1. **Alta (editor offline):** usabilidade da tabela de camadas; relação entre intervalo, LPI e seleção de camadas; clareza visual Corte vs. Mover na prévia; tela de Fill com indicadores coerentes e persistência.
2. **Média (offline):** largura/altura/X/Y/unidade/âncora; controles de texto e suas propriedades; inspeção de preenchimento cruzado e ângulo; testes de lotação, parâmetros extremos e DPI.
3. **Separada/condicionada:** operação real do Laser, conexão GRBL, Home/Frame e G-code. **Sem habilitar emissões físicas** por referência visual.

## Critérios de aceitação a formalizar

- [ ] Preview diferencia visualmente corte e deslocamento, inclui legenda, total de trajetos e mantém correspondência com o relatório JSON.
- [ ] Ao mudar intervalo em camada Fill, LPI se atualiza corretamente; o intervalo alterado salva/reabre e muda apenas os caminhos virtuais da camada escolhida.
- [ ] Controles de saída/visibilidade/ar apresentam estados distintos e seguros; diferença entre “não emitir” e “não exibir” é compreensível.
- [ ] Configurações de velocidade, potência e passagens são coerentes entre lista de camadas, edição detalhada e prévia, sem enviar comandos físicos.
- [ ] Operações de edição permanecem recuperáveis via Undo/Redo e não alteram a arte de reserva fora da mesa.
- [ ] Validar a interface em 125%/150% DPI, pequena largura de janela e com muitas camadas.
- [ ] Tratar valores de 0,010/0,020 mm como referência **comparativa**, não habilitá-los sem análise de limites da simulação.
- [ ] Considerar relatório futuro do LightBurn para decidir mudanças concretas, identificando se cada dado é de GRBL real, captura de UI ou simulação.

## Relações

- Issue #16: vídeo sobre Traçar imagem e inspeção vetorial.
- Issue #15: alternância entre contornos e magnetismo de nós.
- Issue #14: política Community, auditoria antes da abertura pública.
- PR #13: editor experimental 19.8; **não mesclar nem ampliar com estes requisitos antes de triagem**.

**Estado:** referências registradas; sem alteração de código, `main`, `APLICATIVO`, visibilidade do repositório ou comandos físicos.
