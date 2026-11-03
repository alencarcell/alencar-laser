# Alencar Laser — decisão de arquitetura e desempenho
Data: 2026-11-02
Estado: DECISÃO TÉCNICA / NÃO HOMOLOGA FABRICAÇÃO
Aplicação: Alencar Laser Windows C++/Qt 6
Objetivo prioritário: baixa latência de interação, menor consumo de memória medido e estabilidade ao editar projetos grandes.

## Decisão
1. Manter C++ e Qt 6 como núcleo residente do editor Windows (geometria, viewport, seleção, Bézier, projetos, camadas, prévia e estado da interface).
2. Não manter navegador, servidor Web, Node nem runtime Python abertos para poder desenhar/editar.
3. Aproveitar algoritmos comprovados da linha Web Python, especialmente CAM, geração de G-code, streaming GRBL, diagnóstico e tratamento de estados, inicialmente como **referência verificável**. Se a integração direta reduzir risco, permitir um **único processo auxiliar Python iniciado sob demanda** apenas para preparação/execução de trabalhos; encerrá-lo ao ficar ocioso, se o estado da máquina permitir. Medir o custo real dessa decisão.
4. Não migrar tudo a C++ sem testes comparativos. Portar apenas gargalos comprovados e módulos cujo ciclo de vida Python inviabilize o objetivo de memória.
5. O processo da interface não pode bloquear esperando cálculos, streaming ou respostas da máquina. Troca de mensagens versionada, assíncrona, com estados e limites de tamanho; para arquivos grandes preferir artefato temporário com hash, evitando enviar G-code volumoso em JSON.
6. O Windows será dono único da conexão física da máquina: não conectar simultaneamente Web e Desktop à mesma controladora. Sem comandos físicos na branch principal atual até os gates de validação.

## Pontos concretos encontrados no código de 2026-10-27
- `DESENVOLVIMENTO/FONTE/src/editor_tools.hpp` mantém histórico `past/future` de `EditorState` completo (máximo 40) e copia `EditorState next=current` em operações: possível multiplicador de RAM e tempo conforme tamanho do documento. Perfil de memória e latência primeiro; avaliar diffs/command stack depois, preservando undo/redo.
- `DESENVOLVIMENTO/FONTE/qt/qt6_test6.cpp` integra QGraphicsScene, nós e renderização em Qt; medir reconstrução de path, render por objeto e desempenho do zoom/seleção em arquivo denso. Evitar recompor todos os caminhos a cada quadro sem necessidade.
- `DESENVOLVIMENTO/FONTE/src/grbl_observer.hpp` e `serial_observer_win.hpp` são observadores de diagnóstico com envio muito restrito; não representam o streamer CAM/GRBL da Web.
- `CONTINUIDADE_ALENCAR_LASER.md` descreve estrutura 19.8 ainda incompleta na branch main. Não presumir que main reflita a versão local mais nova do Windows.
- Linha Web de referência documental: v0.10.58.7 (Python + JS), ensaiada fisicamente em versões anteriores, mas com gargalos em arquivos densos. Não sobrescrever a linha Web nem a base Windows 19.7.1 verificada.

## Instrumentação mínima — obrigatório antes e depois de otimizações
1. Registrar commit, configuração de build (Debug/Release), Windows, CPU, RAM, arquivo de teste e versão.
2. Testar documento pequeno, documento médio e o caso denso `suporte para alexa` (~575 objetos, ~13.415 nós).
3. Medir working set e memória privada dos processos Windows (Qt e eventual worker Python) no cenário: abertura/ocioso, zoom/pan, seleção, arraste de nós, undo/redo, salvar/abrir.
4. Medir duração p50/p95 das interações e travamentos perceptíveis separadamente do consumo de memória. Aumentar FPS ao custo de muita RAM não é vitória automaticamente.
5. Medir o benefício por mudança única (A/B na mesma máquina e mesmo arquivo). Não prometer percentuais sem resultados.
6. Registrar também consumo total Qt+worker enquanto um trabalho é gerado/executado; não esconder RAM do Python.
7. Comparar G-code da lógica Web original e da integração candidata em trabalhos conhecidos; preservar unidade em mm, S-Max, caminhos, furos e ilhas.

## Ordem da execução
- Gate A: inventário das versões Web e Windows reais; confirmar fonte/hashes, identificar módulos independentes de interface e mensagens do GRBL.
- Gate B: baseline Qt com código original e projetos reais; primeiro reduzir redraw/reconstrução desnecessária e histórico volumoso **após medição**.
- Gate C: protótipo isolado do motor Web em processo sob demanda, sem conectar à máquina: importar trabalho, gerar G-code, comparar tempo, memória e saída.
- Gate D: escolher por benchmark: manter worker Python ou converter partes em C++. Só então integrar ao Windows principal com retorno assíncrono à interface.
- Gate E: ensaios físicos progressivos (S0, limites, pausa/parada, desconexão) e depois trabalho pequeno supervisionado, sob aprovação específica. O laser permanece bloqueado na branch principal enquanto esses testes não passarem.
- Gate F: publicar candidata instalável com rollback e logs, após validação em Windows real.

## Atenções de segurança
- O diagnóstico Web anterior registrou divergência `machine_state.frame_power=0` e `machine_config.framePower=0.8`; verificar estado efetivo antes de qualquer enquadramento sem emissão.
- Falha do processo auxiliar não pode deixar estado do botão Iniciar/Pausar indefinido; cortar conexão e requerer decisão do operador, sem religar ou reenviar automaticamente.
- Pausa por software não substitui parada física de emergência, exaustão e supervisão.
- Não usar esta decisão como atestado de homologação, nem eliminar backups.

## Critério de aceitação
A solução escolhida deve demonstrar, em Windows real, melhor responsividade **e** consumo de memória total não pior que as alternativas no mesmo trabalho, sem regressão em precisão, preservação de arquivos, undo/redo e controle de máquina.
