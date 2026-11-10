# Regressões de 10/11/2026 — Ctrl+D e engrenagens de relógio

## Evidências
- Na interface TESTE_19.8, o diálogo "Gerador de Engrenagens" produz perfil evolvente comum (24 dentes, módulo 3, ângulo de pressão 20°). Não representa uma roda de escape/engrenagem de dentes inclinados do relógio de pêndulo.
- A branch de origem `exp/fase20c-cache-vetorial-20261110` herdou essa UI; o cabeçalho do estudo de escape 20s não foi encontrado nessa branch pelo caminho esperado. Recuperar a implementação correta de sua branch de origem, após comparar histórico, **sem redesenhar de modo especulativo**.
- O comando Ctrl+D exigia uma seleção de apenas um objeto (`ids.size()==1`), assim como o menu de contexto. Isso explica a recusa em duplicar vários.

## Correção experimental implementada
- `Editor::duplicateSelection` copia uma seleção como uma transação única, incluindo membros de grupos e textos editáveis, com novos IDs de objetos e nós, novos IDs de grupos e remapeamento de referências de texto.
- A UI aceita mais de um objeto em Ctrl+D, botão e menu de contexto; novas cópias ficam selecionadas.
- Limites defensivos: até 2000 objetos, 4000 contornos, 200000 nós; falhas são atômicas por meio de `change()`.
- Ainda **não compilado no Windows** e não é versão homologada. O núcleo precisa de testes de grupo, seleção parcial, texto, colisões de IDs, Undo/Redo e 575 objetos. Se falhar, não mesclar.
- Potencial atenção: memória e tempo de cópia da pilha undo em documentos com milhares de objetos.

## Engrenagens / escape
**Pendente**: localizar, recuperar e reintegrar a modalidade específica de dentes inclinados/roda de escape do relógio (geometria, parâmetros e visualização). Não confundir com pinhão evolvente de 20° ou com `CompoundGearDialog`. A imagem atual não demonstra se a ferramenta original foi removida ou apenas perdeu a entrada na interface. Inspecionar a branch onde foi desenvolvida antes de fazer merge.

## Segurança
Manter `main` e `APLICATIVO` intactos; trabalhar na branch de correção; laser desconectado; nenhuma emissão/movimento/G-code habilitado.
