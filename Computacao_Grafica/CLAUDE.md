# CLAUDE.md — Computação Gráfica (UEMS 2026)

Contexto para sessões do Claude nesta pasta.
Disciplina de Computação Gráfica, Profa. Mercedes Gonzales Márquez, 3º ano de Ciência da Computação.
Aluno: Victor Rech Vendruscolo (nº 12 na lista da turma).

## Objetivo atual: avaliação da 4ª unidade (PL4 + PP4 + PO4)

- Enunciado: `material-site/00-enunciados/PP4-CG-2026.pdf`. Resumo na seção 0 de `material-site/CONTEUDO.md`.
- O projeto precisa ter 5 itens: **(1)** cenário 3D (GLUT, GLU ou Bézier), **(2)** animação, **(3)** z-buffer,
  **(4)** iluminação e **(5)** textura (pesquisa própria; não foi vista em aula).
- A ideia precisa ser **autorizada pela professora** antes da implementação.
- **Apresentação do Victor: 19/11/2026.** Slides + código vão por e-mail até as 8h desse dia.
  O assunto é "PL4 e PP4 - nome do aluno" (e-mail da professora no enunciado).
- Avaliação: PL4 = 2 pts (slides, 10 min), PP4 = 4 pts (rotinas principais, 10 min), PO4 = 4 pts (perguntas orais).

## Regra principal: não sair do que foi ensinado

- Use **só** OpenGL de pipeline fixo + GLU + GLUT (freeglut), em C/C++.
  Nada de shaders, OpenGL moderno, GLFW, GLEW, GLM, SDL ou bibliotecas de carga de imagem.
- Todo recurso usado precisa ter origem no material da disciplina.
  Consulte `material-site/CONTEUDO.md` e os arquivos citados nele.
- **Textura** é a única pesquisa permitida. Prefira as técnicas do Guha cap. 12
  (`material-site/referencias/sumanta-guha/ExperimenterSource/Chapter12/`) e cite a fonte.
- Se algo necessário não estiver no material, **pergunte ao Victor antes de usar**.
  Diga de onde viria e por que é preciso.
- Na PP2 a professora avisou que códigos de base fora das aulas "caracterizam cola".
  Prefira partir dos exemplos da aula e dos códigos que o Victor já fez.

## Mapa do material

| O quê | Onde |
|---|---|
| Resumo do conteúdo ensinado (comece por aqui) | `material-site/CONTEUDO.md` |
| Enunciados PP4 e PP2 (modelo anterior) | `material-site/00-enunciados/` |
| Modelagem: curvas, Bézier, superfícies (slides e lista) | `material-site/01-modelagem-geometrica/` |
| OpenGL 1–3: GLUT, primitivas, projeção, objetos GLUT/GLU, transformações | `material-site/02-opengl/` |
| Animação | `material-site/03-animacao/` |
| Cor | `material-site/04-cor/` |
| Visibilidade (back-face culling, z-buffer) | `material-site/05-visibilidade/` |
| Iluminação (12 códigos, inclui `spots.c`) | `material-site/06-iluminacao/` |
| Livro de experimentos + códigos do Guha (caps. 3, 4, 10, 11, 12) | `material-site/referencias/sumanta-guha/` |
| Códigos já feitos pelo Victor nas aulas | `Cap2/`, `Cap3/`, `Cap4/`, `Cap10/` e arquivos soltos (`Planet`, `Robot`, `SpaceTravel`, `Lampshade`, `LookAtDesk`) |
| Trabalho anterior (PP2), modelo de documentação | `Trabalho1_CG/` (`EXPLICACAO.md`, `GUIA_DE_ESTUDO.md`) |

`material-site/06-iluminacao/codigos/spots.c` já cobre os itens 1 a 4 (falta textura). Veja a seção 10 do `CONTEUDO.md`.

## Etapas

### Etapa 1 — Proposta (PL4)
1. Explique os 5 itens a partir do `CONTEUDO.md`, do zero e com exemplos dos códigos da aula.
2. Proponha **2 ou 3 ideias de cenário** que cubram os 5 itens com o que foi ensinado.
   Para cada ideia, mostre o que cada item usa e de qual arquivo vem.
   Inclua a técnica de textura e uma estimativa de dificuldade.
3. O Victor escolhe a ideia. Ele leva a ideia para a professora autorizar.
4. Monte o conteúdo dos slides: cenário, modelos, animação, z-buffer, iluminação e textura,
   com o máximo de detalhes que o enunciado pede. Pergunte o formato dos slides antes de criar.
5. Registre a ideia aprovada e os ajustes da professora neste CLAUDE.md, na seção "Proposta aprovada".

### Etapa 2 — Implementação (PP4)
- Pasta: `Trabalho2_CG/` (o `Trabalho1_CG/` foi a PP2).
- **Só comece depois que o Victor confirmar a aprovação da professora.**
- Implemente **exatamente** a proposta aprovada. Liste extras só como sugestões, separadas.
- Siga a estrutura dos exemplos da aula: `setup()`, `drawScene()`, `resize()`, `keyInput()`, função de animação e `main()`.
  Use `GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH`.
- Código simples e comentado em português. O Victor precisa explicar cada linha na PO4.
- Textura: se usar BMP, precisa ser 24 bits sem compressão (`getBMPData` do Guha).
  O caminho do arquivo é relativo ao diretório de execução. Uma textura procedural (ex.: tabuleiro) não depende de arquivo.

### Etapa 3 — Preparo para a PO4
- Crie `Trabalho2_CG/EXPLICACAO.md` (o que foi feito e como funciona, rotina por rotina)
  e `Trabalho2_CG/GUIA_DE_ESTUDO.md` (perguntas prováveis e respostas), no mesmo formato do `Trabalho1_CG/`.
- As perguntas devem cobrir o código e a teoria das listas L22, L41 e L42.
  Exemplos: algoritmo do z-buffer, back-face culling, ambiente/difusa/especular, Lambert, `w = 0` ou `1`,
  parâmetros de spot, atenuação, buffer duplo, idle × timer, `glPushMatrix`/`glPopMatrix`.

### Transversal — tabela de rastreabilidade
No `EXPLICACAO.md`, cada recurso usado no código entra numa tabela com três colunas:
o recurso, o trecho do código e **onde apareceu na aula** (arquivo + slide, experimento ou código).
A textura aparece marcada como **pesquisa própria**, com a fonte (Guha cap. 12, Experimenter p. 87–94).

## Compilar e testar

- Comando do projeto (`.vscode/tasks.json`):
  `g++ -g arquivo.cpp -o output/arquivo -lGL -lGLU -lglut` (o binário vai para `output/` na pasta do arquivo).
- Ambiente do Victor: Linux Mint, VS Code. Um dos notebooks usa GPU integrada Intel Iris.
- Na nuvem: instale `freeglut3-dev`, compile e, se possível, rode com `xvfb-run` para capturar um print da cena.
  O teste interativo (teclas, mouse, fluidez) é feito pelo Victor.

## Como trabalhar com o Victor

- Responda em português do Brasil, de forma concisa e direta.
- **Pergunte** quando faltar especificação. Não decida sozinho.
- Explique do zero, ligando cada conceito ao material.
- Textos para entregar: frases simples, voz ativa, sem vocabulário rebuscado.
- Nunca invente referências. Se não tiver certeza da fonte, diga.

## Lacunas conhecidas

- As texturas BMP do Guha não existem no servidor da disciplina.

## Proposta aprovada

- **Histórico:** a ideia da escrivaninha (`Trabalho2_CG/PROPOSTA_IDEIAS.md`, Parte 3) foi recusada.
  A professora pediu uma **brinquedoteca** e autorizou essa ideia sem pedir ajustes (informado pelo Victor em 17/11).
- **Código:** `Trabalho2_CG/brinquedoteca.cpp`. **Slides:** PowerPoint (.pptx).
- **Versão 3 (18/11):** sala 12 x 12 (altura 5), vista pela quina. O Victor decidiu manter tudo (19/11).
- **Slides finais (19/11):** `Trabalho2_CG/PL4_PP4_Victor_Rech_Vendruscolo.pptx` (+ `.pdf`), 21 slides **só de código**:
  cada slide mostra o trecho real do `.cpp` com o número das linhas, destaques, DECISÃO e FONTE. Os slides da versão 1
  e os `_v3` (slides por cena, com imagens) foram apagados a pedido do Victor.
- **Apresentação:** 5 min com o programa rodando (teclas) + 15 min de código. Roteiro: `Trabalho2_CG/ROTEIRO_APRESENTACAO.md`.
- **Comentários do código (19/11):** mapa do código no cabeçalho, seções marcadas com `[ITEM n]` e as marcas
  `DECISÃO` (por que foi feito assim) e `FONTE` (de onde veio). Só comentários mudaram; o comportamento é o mesmo.
- **Etapa 3 feita para a versão 3:** `Trabalho2_CG/EXPLICACAO.md` (rotina por rotina + rastreabilidade) e
  `Trabalho2_CG/GUIA_DE_ESTUDO.md` (plano de 2 h, padrões, 6 blocos novos, teoria L22/L41/L42, 38 perguntas).
- **Cenário:** piso de madeira, paredes de bolinhas, janela redonda (vidro emissivo + moldura de toro) e luminária de teto.
- **Barraca:** cilindro + cone da GLU com textura de listras; porta sempre aberta (retângulo escuro + meio disco).
- **Carrinho de ronda:** trajeto de curvas de Bézier cúbicas (4 trechos de ida, 4 de volta), velocidade constante,
  direção pela tangente (`atan2`). Fases: ida (passa pelo patinho, blocos e torre) → 2 s dentro da barraca →
  volta (passa pelo ursinho e pela bola) → fim na vaga. Espaço pausa; no fim, começa outra ronda.
- **Ursinho:** vira a cabeça enquanto o carrinho está na barraca; olhos brilham (emissão azul-clara) com a luz da sala apagada.
- **Amarelinha:** 8 casas (malhas 4 x 4) + números com `glutStrokeCharacter` + céu com `gluPartialDisk`.
- **Outros:** blocos de montar e patinho (cores fortes), torre de argolas, bola de praia.
- **Z-buffer:** `GLUT_DEPTH`, `GL_DEPTH_TEST`, limpeza a cada quadro; tecla `z`. O z-buffer também esconde o carrinho
  atrás da porta escura quando ele entra na barraca.
- **Iluminação:** `GL_LIGHT0` sala (pontual, com ambiente próprio 0,25, tecla `1`); `GL_LIGHT1` janela (spot longe,
  cone de 3,3°: raios quase paralelos, mancha do tamanho da janela, tecla `2`); `GL_LIGHT2/3` faróis (spots de 25°,
  atenuação, tecla `3`, apagam na barraca); `GL_LIGHT4/5` sirene (spots vermelho e azul girando, tecla `4`, só na ronda);
  ambiente global 0,15; emissão na lâmpada, vidro, faróis, sirene e olhos; `GL_NORMALIZE`.
  Limitação: sem sombras no pipeline fixo, a luz atravessa os objetos. Feixe de luz visível ficou de fora:
  precisaria de transparência (`GL_BLEND`), que não foi dada em aula.
- **Textura (pesquisa, Guha cap. 12):** gerada no código, sem BMP. Madeira (adaptada de `createChessboard`),
  listras da barraca e da bola (adaptadas de `createStripes`, com `gluQuadricTexture`) e bolinhas das paredes
  (`GL_LINEAR`). `GL_MODULATE`.
- **Teclas:** espaço (pausa/recomeça a ronda), setas (câmera), `1`/`2`/`3`/`4` (luzes), `z` (z-buffer), `t` (textura), Esc.
