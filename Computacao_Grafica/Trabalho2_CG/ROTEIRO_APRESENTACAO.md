# Roteiro da apresentação — Brinquedoteca (PL4 + PP4)

**Total: 20 min.** Parte 1: 5 min com o programa rodando (teclas). Parte 2: 15 min com os slides de código.
Slides: `PL4_PP4_Victor_Rech_Vendruscolo.pptx` (ou o `.pdf`). As notas de cada slide têm o mesmo texto da Parte 2.
Perguntas prováveis da PO4: `GUIA_DE_ESTUDO.md`, Parte 4.

## Antes de começar (checklist)

1. Compile e abra o programa antes da sua vez:

   ```bash
   cd Trabalho2_CG
   ```

   ```bash
   g++ -g brinquedoteca.cpp -o output/brinquedoteca -lGL -lGLU -lglut
   ```

   ```bash
   ./output/brinquedoteca
   ```

2. Maximize a janela do programa (o `resize` ajusta a imagem).
3. Deixe os slides abertos na capa e o `brinquedoteca.cpp` aberto no VS Code (`Ctrl+G` + número vai direto para a linha).
4. A ronda começa sozinha quando o programa abre. Quando ela termina, o carrinho fica parado na vaga.
   **Espaço** com o carrinho parado começa outra ronda; com o carrinho andando, pausa.

## Parte 1 — Demonstração (5 min, só o programa e as teclas)

Para cada situação, fale três coisas curtas: **para que serve**, **como fiz** (geral) e **de onde veio**.

| Tempo | Tecla | O que aparece | O que falar |
|---|---|---|---|
| 0:00–0:45 | (nenhuma) | Visão geral: piso de madeira, paredes de bolinhas, janela redonda, luminária, barraca, amarelinha, blocos, torre, patinho, ursinho, bola e o carrinho na vaga. | **Item 1, modelos 3D.** Objetos prontos da GLUT (esfera, cubo, cone, toro), quádricas da GLU (barraca, torre, bola, janela) e uma malha própria (piso, paredes, amarelinha), montados com transformações e pilha. Vieram dos exemplos da aula (glutObjects, gluQuadrics, torpedo) e dos meus exercícios (Carro2.0, Fonts). |
| 0:45–1:05 | ← → ↑ ↓ | A câmera gira em volta da sala, sobe e desce. | Câmera com `gluLookAt`: o olho gira em volta do eixo y (seno e cosseno). Veio dos slides de câmera. Volte ao ângulo inicial com o mesmo número de toques. |
| 1:05–1:45 | Espaço | A ronda inteira (uns 14 s): sai da vaga, passa pelo patinho, blocos e torre, entra na barraca, volta pelo ursinho e pela bola e para na vaga. | **Item 2, animação.** Um timer de 20 ms muda a posição a cada quadro. O caminho são 8 curvas de Bézier; a frente segue a tangente e as rodas giram conforme a distância. Bézier da aula de modelagem; tangente do Guha cap. 10. Mostre: o carrinho **some atrás da porta escura** (z-buffer) e o **ursinho vira a cabeça** enquanto ele está lá dentro. |
| 1:45–2:20 | 1 | A sala apaga. O sol entra pela janela e desenha uma mancha redonda no chão. Os olhos do ursinho brilham. | **Item 4, iluminação.** A luz da sala é pontual no teto e tem ambiente próprio, por isso a sala escurece. O sol é um spot longe, com cone de 3,3°, do tamanho da janela: raios quase paralelos. Os olhos têm emissão. Veio do spots.c, plane.c e material.c. |
| 2:20–3:20 | 2, depois Espaço | Noite: o vidro fica azul-escuro. Na ronda, só os faróis e a sirene iluminam; dentro da barraca eles apagam. | Faróis: 2 spots de 25° com atenuação (a luz enfraquece com a distância), presos ao carrinho porque a posição é enviada depois da transformação dele. Sirene: 2 spots, vermelho e azul, girando. Veio do movelight.c, da lanterna do robô (lista L42) e do spots.c. Dica: **Espaço** no meio pausa, para mostrar o cone dos faróis no chão; Espaço de novo continua. |
| 3:20–3:40 | 3, 4 (e de novo) | Os faróis e a sirene apagam e acendem, cada um separado. | Cada luz é uma `GL_LIGHT` própria, ligada e desligada no `drawScene`. Depois aperte **1** e **2** para voltar ao dia. |
| 3:40–4:10 | z (e de novo z) | Sem z-buffer: a barraca mostra as listras de dentro por cima das de fora, e a luminária aparece na frente da barraca. | **Item 3, z-buffer.** Sem o teste de profundidade, vale a ordem do desenho: o que é desenhado depois fica na frente. No código: `GLUT_DEPTH`, `GL_DEPTH_TEST` e `glClear` a cada quadro. Veio dos slides de visibilidade. |
| 4:10–4:40 | t (e de novo t) | Sem textura: piso marrom liso, paredes lisas, barraca vermelha e bola clara. | **Item 5, textura (pesquisa).** Três imagens geradas no código (madeira, listras, bolinhas), sem arquivo. `GL_MODULATE` mistura a textura com a luz. Foi o único item pesquisado fora das aulas: Guha cap. 12. |
| 4:40–5:00 | (nenhuma) | Volta para os slides. | "Isso é o programa. Agora mostro como cada parte está no código." |

## Parte 2 — Código (15 min, slides 1 a 21)

Cada slide mostra o trecho real do `brinquedoteca.cpp`, com o número da linha.
Aponte as linhas em destaque (número laranja). Os quadros DECISÃO e FONTE repetem os comentários do código.
**Marcos:** aos 5 min, slide 8 (barraca); aos 10 min, slide 14 (z-buffer).
**Se atrasar:** passe rápido pelos slides 10 (ursinho), 11 (amarelinha), 17 (materiais) e 20 (fontes).

**Slide 1 · Capa** (0:10; termina em 0:10)

> Agora mostro o código. O programa está num arquivo só, brinquedoteca.cpp, em C++ com OpenGL de pipeline fixo, GLU e GLUT, como nas aulas. Em cada slide aparece o trecho real, com o número da linha. Os comentários marcados com DECISÃO dizem por que fiz assim, e os marcados com FONTE dizem de onde veio.

**Slide 2 · Mapa do código** (0:40; termina em 0:50)

> O arquivo segue esta ordem: variáveis globais, texturas, ajudantes, a sala, a barraca, a amarelinha, o carrinho e os outros brinquedos. No fim ficam as rotinas da GLUT. O título de cada seção diz qual item da PP4 ela cobre. A execução começa no main: ele chama o setup uma vez e passa o controle para a GLUT, que chama o drawScene a cada quadro, o animate a cada 20 milissegundos e o keyInput quando aperto uma tecla.

**Slide 3 · Os 5 itens no código** (0:40; termina em 1:30)

> Esta tabela liga os cinco itens do enunciado ao código. Modelos: objetos da GLUT, quádricas da GLU e uma malha própria. Animação: timer de 20 milissegundos e um trajeto de curvas de Bézier. Z-buffer: pedido no main, ligado no setup e limpo a cada quadro. Iluminação: seis luzes e os materiais. Textura: três imagens geradas no código, que foi a parte de pesquisa.

**Slide 4 · Estrutura: main e resize** (0:40; termina em 2:10)

Linhas em destaque (número laranja): 1058, 1148, 1152, 1157.

> O main cria a janela com buffer duplo, cores RGB e z-buffer. Depois chama o setup e registra as rotinas: desenho, mudança de tamanho da janela, teclado, setas e o timer. O primeiro timer dispara em 5 milissegundos e depois o animate se agenda sozinho. O resize ajusta a viewport e a projeção em perspectiva: 60 graus de abertura, plano perto em 1 e longe em 100.

**Slide 5 · Estrutura: drawScene desenha um quadro** (1:00; termina em 3:10)

Linhas em destaque (número laranja): 907, 911, 924, 947.

> O drawScene desenha um quadro, sempre na mesma ordem. Limpa a cor e o z-buffer. Posiciona a câmera com gluLookAt: o olho gira em volta do eixo y, por isso seno e cosseno. Liga ou desliga as luzes do carrinho conforme a fase da ronda. Envia a posição das luzes depois da câmera, para elas ficarem presas à sala, e as do carrinho antes de desenhar. Desenha a sala sem brilho e os brinquedos com brilho. No fim, troca os buffers.

**Slide 6 · Estrutura: animate (timer) e teclas** (0:45; termina em 3:55)

Linhas em destaque (número laranja): 1118, 1125, 1129, 1083.

> O animate é chamado pelo timer. Se a animação está ligada, ele dá um passo da ronda, gira a sirene 12 graus e vira a cabeça do ursinho enquanto o carrinho está na barraca. No fim ele se agenda de novo para daqui a 20 milissegundos e pede um novo desenho. Usei timer, e não idle, para a velocidade não depender do computador. As teclas só trocam variáveis: o espaço pausa ou recomeça, e a tecla 3 troca a chave dos faróis, que o drawScene usa.

**Slide 7 · Malha própria: desenharMalha** (0:50; termina em 4:45)

Linhas em destaque (número laranja): 262, 265, 268, 289, 313.

> Esta função faz a malha do piso, das paredes e das casas da amarelinha. São n faixas de triângulos; em cada faixa, cada coluna tem dois vértices, um em cima e um embaixo. Todos têm a mesma normal, +z, e uma coordenada de textura. Fiz a malha fina porque o OpenGL calcula a luz nos vértices: com só quatro cantos, os cones dos spots não apareceriam no chão. Para usar a malha, só giro: no piso, -90 graus em x; na parede da esquerda, 90 graus em y.

**Slide 8 · Quádricas da GLU: desenharBarraca** (0:45; termina em 5:30)

Linhas em destaque (número laranja): 396, 398, 416, 425.

> A barraca usa quádricas da GLU. A quádrica nasce no eixo z, então giro -90 graus em x para ela ficar em pé. O mesmo gluCylinder faz a parede e o teto: o teto é um cone, com raio de cima zero, e começa onde a parede termina, porque o glTranslatef sobe 1,5 dentro da mesma pilha. A porta é um retângulo escuro com meio disco em cima, um pouco à frente da parede. Quando o carrinho entra, o z-buffer esconde atrás da porta a parte que já entrou.

**Slide 9 · Hierarquia: o carrinho e as rodas** (0:50; termina em 6:20)

Linhas em destaque (número laranja): 561, 562, 615, 666.

> O carrinho é um modelo hierárquico. A função posicionarCarrinho leva tudo para a posição e o ângulo atuais; as luzes do carrinho usam a mesma função. Dentro dessa pilha, a carroceria e a cabine são cubos escalados, e cada roda tem a sua própria pilha: vai para o lugar, gira em volta do eixo dela e é desenhada. O giro das rodas é a distância andada dividida pelo raio, então elas rolam sem deslizar. A roda com parafusos veio do meu Carro2.0.

**Slide 10 · Hierarquia: a cabeça do ursinho** (0:35; termina em 6:55)

Linhas em destaque (número laranja): 777, 811, 831, 846.

> O ursinho segue o mesmo padrão. Uma transformação põe o ursinho no lugar, gira 20 graus para a câmera e reduz o tamanho. A cabeça tem a própria pilha e uma rotação em y: orelhas, focinho e olhos estão dentro dela, então giram juntos, e o corpo fica parado. Com a luz da sala apagada, os olhos ganham emissão e brilham. Patinho, torre, blocos e bola usam a mesma ideia.

**Slide 11 · Amarelinha: casas, números e céu** (0:40; termina em 7:35)

Linhas em destaque (número laranja): 440, 463, 474, 491.

> Cada casa da amarelinha é uma malha 4 por 4 deitada no chão, a 0,01 do piso para não empatar com ele no z-buffer. O número usa a fonte de traço da GLUT: um caractere tem cerca de 100 unidades de altura, e a escala 0,003 deixa ele com 0,3. A amarelinha inteira usa uma transformação só: vai para o começo e gira 45 graus para a quina. O céu é um meio disco.

**Slide 12 · Trajeto: curvas de Bézier cúbicas** (0:50; termina em 8:25)

Linhas em destaque (número laranja): 83, 507, 511, 512.

> O trajeto é feito de curvas de Bézier cúbicas. Cada linha da tabela é um trecho com quatro pontos de controle no piso: são quatro trechos na ida e quatro na volta, como no desenho. A função bezier calcula o ponto com os pesos de Bernstein, que somam 1, e a tangente, que é a derivada da curva. O último ponto de um trecho é o primeiro do próximo, e as tangentes têm a mesma direção: por isso o caminho não tem quinas.

**Slide 13 · Ronda: atualizarRonda (fases e velocidade)** (1:00; termina em 9:25)

Linhas em destaque (número laranja): 532, 539, 548, 552.

> A ronda é uma máquina de fases: 0 é a ida, 1 é a espera na barraca, 2 é a volta e 3 é o fim. Na fase 1, o carrinho espera 100 quadros, ou 2 segundos. Nas fases 0 e 2, u avança 0,05 dividido pelo tamanho da tangente: assim o carrinho anda sempre 0,05 por quadro, na curva ou na reta. A parte inteira de u é o trecho e a parte decimal é o t. O ângulo vem do atan2 da tangente. Quando u chega a 4, a fase muda.

**Slide 14 · Z-buffer no código** (0:50; termina em 10:15)

Linhas em destaque (número laranja): 1148, 972, 907, 1090.

> O z-buffer aparece em quatro lugares: GLUT_DEPTH no main cria o buffer, GL_DEPTH_TEST no setup liga o teste, o glClear limpa a profundidade a cada quadro e a tecla z liga e desliga. O algoritmo: para cada pixel, o OpenGL guarda a profundidade do ponto mais perto e só pinta um ponto novo se ele estiver mais perto. Superfícies coladas ficam um pouco à frente: a amarelinha a 0,01 do piso, a janela a 0,03 da parede e a porta a 0,04 da barraca. Sem isso, as profundidades empatam e a cor pisca.

**Slide 15 · Iluminação: luz da sala e luz da janela** (1:00; termina em 11:15)

Linhas em destaque (número laranja): 977, 988, 989, 1028.

> No setup ficam as cores e os parâmetros das luzes. A luz 0, da sala, é pontual e tem parte ambiente própria: com ela apagada, a sala escurece de verdade. A luz 1, da janela, é um spot colocado longe, com cone de 3,3 graus: na janela, esse cone tem o raio do vidro, então a luz passa justa e os raios chegam quase paralelos, como o sol. O expoente 0 deixa a mesma força em todo o cone. O ambiente global é baixo, 0,15.

**Slide 16 · Iluminação: faróis e sirene que andam** (0:50; termina em 12:05)

Linhas em destaque (número laranja): 997, 1000, 579, 587.

> Os faróis são spots de 25 graus, com expoente 8 e atenuação: a luz enfraquece com a distância. A sirene são dois spots coloridos, de 30 graus, em lados opostos. A posição de uma luz passa pela matriz modelview do momento. Por isso chamo posicionarCarrinho antes de enviar a posição: as luzes andam e viram junto com o carrinho, como a lanterna na mão do robô da lista. A sirene ainda sobe até a cabine e gira com anguloSirene.

**Slide 17 · Materiais: brilho, emissão e cor** (0:45; termina em 12:50)

Linhas em destaque (número laranja): 239, 241, 335, 1032, 1037.

> A função brilho liga o especular nos plásticos e desliga no piso, nas paredes e na pelúcia. O vidro da janela tem cor preta e emissão: aparece só a cor que ele emite, céu claro de dia e escuro de noite. A emissão é desligada logo depois, porque o material é um estado do OpenGL. A sirene troca a emissão entre vermelho e azul a cada 10 quadros. GL_COLOR_MATERIAL faz o glColor3f virar a cor do material, e GL_NORMALIZE corrige as normais depois do glScalef.

**Slide 18 · Textura (pesquisa): gerar a imagem** (0:50; termina em 13:40)

Linhas em destaque (número laranja): 137, 138, 166, 188.

> A textura foi a parte de pesquisa, no capítulo 12 do Guha. Uma textura é um vetor de 64 por 64 pixels, com três bytes por pixel. Na madeira, cada 16 linhas formam uma tábua com um tom de marrom, e a junta pinta a linha entre as tábuas e uma emenda em lugar diferente em cada tábua. Nas listras, a cor muda a cada 8 colunas. Nas bolinhas, o pixel dentro do círculo de raio 10 recebe a cor da bolinha, e o resto fica branco. Gerei as imagens no código para não depender de arquivo.

**Slide 19 · Textura (pesquisa): enviar e aplicar** (0:50; termina em 14:30)

Linhas em destaque (número laranja): 205, 206, 210, 218, 226, 1049.

> Depois de gerar, envio as imagens. glGenTextures cria três ids. Para cada um, glBindTexture escolhe a textura, glTexParameteri define a repetição e o filtro, e glTexImage2D copia o vetor. Antes de desenhar, ligarTextura ativa a textura certa. Na malha, cada vértice recebe um glTexCoord2f; nas quádricas, gluQuadricTexture gera as coordenadas. Com GL_MODULATE, a cor da textura é multiplicada pela cor iluminada: por isso a textura reage às luzes.

**Slide 20 · De onde veio cada recurso** (0:20; termina em 14:50)

> Esta tabela resume as marcas FONTE do código. Tudo vem dos códigos e slides da aula, das listas ou dos meus exercícios. Só a textura é pesquisa própria, no capítulo 12 do Guha, como o enunciado permite.

**Slide 21 · Encerramento** (0:05; termina em 14:55)

> Obrigado. Deixo as teclas na tela para as perguntas: posso ligar e desligar cada item enquanto respondo.

## Depois: e-mail (até as 8h)

- **Para:** e-mail da professora (no enunciado da PP4).
- **Assunto:** `PL4 e PP4 - Victor Rech Vendruscolo`
- **Anexos:** `PL4_PP4_Victor_Rech_Vendruscolo.pdf` (ou o `.pptx`) e `brinquedoteca.cpp`.
