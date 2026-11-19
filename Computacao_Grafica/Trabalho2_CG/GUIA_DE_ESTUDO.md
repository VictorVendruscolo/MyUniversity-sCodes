# Guia de Estudo — Trabalho 2 de Computação Gráfica (Brinquedoteca)

> Objetivo: você conseguir **apresentar** (PL4 e PP4) e **responder às perguntas** (PO4) sobre o
> `brinquedoteca.cpp`. Cerca de 85% do código repete poucos padrões que você já usou nas aulas.
> Estude os padrões uma vez e gaste o tempo nos 6 blocos novos.
> Para os detalhes rotina por rotina, veja o `EXPLICACAO.md`.

---

## Parte 0 — Plano de 2 horas

| Tempo | O que estudar | Onde |
|---|---|---|
| 0:00 – 0:15 | Os 6 padrões que se repetem, com o código aberto | Parte 1 |
| 0:15 – 0:45 | Ronda de Bézier e luzes presas ao carrinho | Parte 2.1 e 2.2 |
| 0:45 – 1:05 | Janela, porta da barraca, ursinho e amarelinha | Parte 2.3 a 2.6 |
| 1:05 – 1:30 | Teoria: iluminação, z-buffer, textura e animação | Parte 3 |
| 1:30 – 1:50 | Perguntas prováveis: responda em voz alta antes de ler a resposta | Parte 4 |
| 1:50 – 2:00 | Rodar o programa e ensaiar a demonstração | `ROTEIRO_APRESENTACAO.md` |

**O que a prova cobra:**

- **PL4 (2 pts) + PP4 (4 pts):** 20 minutos no total. Plano do Victor: 5 min com o programa rodando (teclas)
  e 15 min com os 21 slides de código. PL4: apresentação (1 pt) e domínio do assunto (1 pt).
  PP4: domínio do código (2 pts) e avaliação do código (2 pts).
- **PO4 (4 pts):** perguntas da professora sobre o código e a teoria (listas L22, L41 e L42).

---

## Parte 1 — Os 6 padrões (aprenda uma vez, vale para o código todo)

### 1.1 Peça = pilha + transformações + objeto pronto

```c
glPushMatrix();
glTranslatef(x, y, z);   // 3) leva para o lugar
glRotatef(a, 0, 1, 0);   // 2) gira
glScalef(sx, sy, sz);    // 1) estica
glutSolidSphere(r, 20, 20);
glPopMatrix();
```

- A **última transformação escrita é a primeira aplicada** ao objeto. Leia de baixo para cima.
- `glPushMatrix` guarda a matriz; `glPopMatrix` devolve. Assim uma peça não mexe na outra.
- Aparece em: ursinho, patinho, torre, blocos, bola, rodas, luminária, carrinho e na função `caixa`.

### 1.2 Peças simétricas

```c
for (lado = -1; lado <= 1; lado += 2)   // lado vale -1 e depois 1
   glTranslatef(0.45 * lado, ...);      // esquerda e direita
```

Aparece em: orelhas, olhos, braços e pernas do ursinho, olhos do patinho e faróis do carrinho.

### 1.3 Textura gerada no código

```c
for (i = 0; i < 64; i++)
   for (j = 0; j < 64; j++)
      imagem[i][j] = cor escolhida por uma fórmula;
```

Só muda a fórmula: madeira (tábua = `i / 16`), listras (`(j / 8) % 4`) e bolinhas (distância ao centro).
`i` é a linha (coordenada t) e `j` é a coluna (coordenada s).

### 1.4 Emissão: o objeto parece aceso

```c
glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, cor);    // liga
... desenha ...
glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, semEmissao); // desliga (máquina de estados!)
```

Aparece em: lâmpada, vidro da janela, faróis, cúpula da sirene e olhos do ursinho.
**A emissão não ilumina os outros objetos.** Quem ilumina são as luzes (`GL_LIGHTi`).

### 1.5 Configurar uma luz

```c
glLightfv(GL_LIGHTi, GL_DIFFUSE, cor);        // cor da luz
glLightfv(GL_LIGHTi, GL_POSITION, pos);       // w = 1: posicional; w = 0: direcional
glLightf (GL_LIGHTi, GL_SPOT_CUTOFF, graus);  // abertura do cone (180 = não é spot)
glLightfv(GL_LIGHTi, GL_SPOT_DIRECTION, dir);
glLightf (GL_LIGHTi, GL_SPOT_EXPONENT, e);    // concentração no centro
glLightf (GL_LIGHTi, GL_LINEAR_ATTENUATION, kl);
glEnable(GL_LIGHTi);
```

As 6 luzes do `setup` repetem esse bloco com valores diferentes.

### 1.6 Malha (`desenharMalha`)

Um plano de n × n quadradinhos em faixas de triângulos, com uma normal (0, 0, 1) e uma coordenada de
textura em cada vértice. Serve para o piso, as paredes e as casas da amarelinha. Só muda a transformação
(o piso é a malha deitada com `glRotatef(-90, 1, 0, 0)`).

---

## Parte 2 — Os 6 blocos novos (estude com calma)

### 2.1 A ronda: trajeto de Bézier (`bezier`, `pontoDoTrajeto`, `atualizarRonda`)

- **Ideia:** o caminho são 8 curvas de Bézier cúbicas (4 de ida, 4 de volta). Cada uma tem 4 pontos de controle.
- **Fórmula:** `P(t) = (1−t)³B₀ + 3t(1−t)²B₁ + 3t²(1−t)B₂ + t³B₃`.
  - Os pesos são os **polinômios de Bernstein** e somam 1.
  - A curva passa por B₀ e B₃. B₁ e B₂ "puxam" a curva.
  - A curva fica dentro do **fecho convexo** dos 4 pontos.
- **Sem quinas:** o fim de um trecho é o começo do próximo, e os pontos vizinhos da emenda ficam numa reta.
- **Tangente:** é a derivada `P'(t)`. Ela diz a direção do movimento.
- **`u`:** vai de 0 a 4. A parte inteira escolhe o trecho; a parte decimal é o `t` do trecho.
- **Velocidade constante:** `u += 0.05 / |P'(t)|`. O tamanho da tangente diz quanto a curva anda quando u anda 1.
  Dividindo, o carrinho anda sempre 0,05 por quadro.
- **Ângulo:** `carroAng = atan2(-dz, dx) * 180 / PI`. O `glRotatef(carroAng, 0, 1, 0)` leva a frente (+x)
  para a direção da tangente. O `-dz` aparece porque girar +x em volta de y por a dá (cos a, 0, −sen a).
- **Fases:** 0 = ida, 1 = na barraca (100 quadros = 2 s), 2 = volta, 3 = fim. No fim do trajeto (u = 4), muda de fase.
- **Meia-volta escondida:** a ida termina virada para −x e a volta começa virada para +x, os dois no centro da barraca.

### 2.2 Luzes presas ao carrinho (`posicionarLuzesDoCarrinho`)

- **Regra:** `glLightfv(..., GL_POSITION, ...)` e `GL_SPOT_DIRECTION` passam pela **matriz modelview do momento**.
- Por isso o código chama `posicionarCarrinho()` (translação + rotação do carrinho) **antes** dos `glLightfv` dos faróis:
  as luzes andam e viram junto com o carrinho. É a lanterna na mão do robô da lista L42 e o `movelight.c`.
- **Sirene:** depois dos faróis, `glTranslatef` até a cabine e `glRotatef(anguloSirene, 0, 1, 0)`.
  As luzes 4 (vermelha) e 5 (azul) apontam para lados opostos e giram juntas (como no `spots.c`).
- **Pisca:** `(quadro / 10) % 2` alterna 0 e 1 a cada 10 quadros; a cúpula troca a emissão entre vermelho e azul.
- **Regras da ronda (`drawScene`):** faróis apagam na fase 1 (dentro da barraca); sirene só nas fases 0 e 2.
- **Ordem:** as luzes do carrinho são posicionadas **antes** de desenhar a cena. Se viessem depois, a cena
  usaria a posição do quadro anterior.

### 2.3 Luz da janela (sol)

- É um **spot muito longe** (15 unidades da janela) com cone de 3,3°: na janela, o cone tem o raio do vidro
  (tan 3,3° × 15 ≈ 0,85). Os raios chegam quase paralelos, como os do sol.
- A mancha no chão tem o formato da janela redonda, esticada pela inclinação. Ela passa pelo rosto do ursinho.
- Expoente 0: a mesma força em todo o cone.
- O vidro escurece (`ceuNoite`) quando a luz da janela é desligada (tecla 2).

### 2.4 Porta da barraca e o z-buffer

- A porta é um retângulo escuro + meio disco (`gluPartialDisk` de −90° a 90°), um pouco à frente da parede.
- Quando o carrinho entra, a parte que passou pela porta fica **atrás** do retângulo escuro: o z-buffer a esconde.
  Quando ele está todo dentro, o cilindro e o cone o escondem.

### 2.5 Ursinho

- **Cabeça:** `glRotatef(anguloCabeca, 0, 1, 0)` dentro do `glPushMatrix` da cabeça. Ela gira sem girar o corpo.
- **Quando:** no `animate`, se `fase == 1` a cabeça gira +3,5° por quadro até 70°; senão volta −7° por quadro até 0.
- **Olhos:** com a luz da sala apagada (`!luzSala`), ganham emissão azul-clara.

### 2.6 Amarelinha

- Cada casa: malha 4 × 4 deitada, a **y = 0,01** (um pouco acima do piso para não dar *z-fighting*: duas superfícies na
  mesma profundidade piscando), borda `GL_LINE_LOOP` e número com `glutStrokeCharacter(GLUT_STROKE_ROMAN, '1')`.
- Um caractere da fonte de traço tem cerca de 100 unidades; `glScalef(0.003, ...)` deixa com 0,3.
- As linhas são iluminadas com a normal (0, 0, 1) que ficou valendo (máquina de estados).

---

## Parte 3 — Teoria (listas L22, L41 e L42)

### 3.1 Animação (L22)

- **Interativa:** a tecla muda uma variável e chama `glutPostRedisplay()`.
- **Idle:** `glutIdleFunc(f)` roda `f` sempre que o sistema está livre: a velocidade depende do computador.
- **Timer:** `glutTimerFunc(ms, f, valor)` roda `f` depois de `ms` milissegundos; `f` se agenda de novo. Período fixo.
  **Usei timer de 20 ms** (cerca de 50 quadros por segundo).
- **Buffer duplo:** `GLUT_DOUBLE` cria dois buffers de cor. Desenha no de trás e `glutSwapBuffers()` troca.
  Sem ele, a tela pisca (o usuário veria o desenho sendo feito).
- **Física da L22:** bola rolando, lançamento (`x = h·t`, `y = v·t − g·t²/2`). No trabalho, a roda rola sem deslizar:
  ângulo (rad) = distância / raio.

### 3.2 Visibilidade e z-buffer (L41)

- **Algoritmos de visibilidade:** no espaço da imagem (por pixel, ex.: z-buffer) ou no espaço da cena (pela geometria,
  ex.: back-face culling).
- **Z-buffer:** duas memórias, o buffer de cor e o de profundidade.
  1. Inicializa o de profundidade com o máximo e o de cor com o fundo.
  2. Rasteriza cada polígono.
  3. Em cada pixel: se o z novo for **menor** (mais perto), pinta e guarda o z. Senão, ignora.
  - A ordem de desenho deixa de importar. A L41 usa a convenção oposta: começa em −∞ e vence o **maior** z.
- **Em OpenGL:** `GLUT_DEPTH` (aloca), `glEnable(GL_DEPTH_TEST)` (liga), `glClear(... | GL_DEPTH_BUFFER_BIT)` (limpa a cada quadro).
  `glDepthFunc(GL_LESS)` é a regra padrão.
- **Back-face culling:** a face é da frente se `E · N ≥ 0` (E = vetor de visão, N = normal). A normal de uma face com P1, P2, P3
  é `n = u × v`, com `u = P2P3` e `v = P2P1`. **Funciona só para um poliedro convexo fechado.** Não usei: a cena tem vários
  objetos e formas côncavas (o ursinho).
- **Cor (L41):** RGB de 0 a 1 em OpenGL (0 a 255 nas imagens das texturas). HSV: H = matiz (0° a 360°), S = saturação, V = valor.
  Cones do olho: cor; bastonetes: pouca luz.

### 3.3 Iluminação (L42)

- **Fontes:** ambiente (vem de todo lado, sem direção), direcional (raios paralelos, `w = 0`), pontual (`w = 1`) e spot
  (pontual com cone).
- **Modelo:** `I = Ia·Ka + Id·Kd·cos θ + Ie·Ke·cosⁿ φ`.
  - **Difusa (Lambert):** depende do ângulo θ entre a normal N e a direção da luz L. **Não depende do observador.**
  - **Especular:** depende do ângulo φ entre a reflexão R e o observador. n = `GL_SHININESS` (maior = brilho menor e mais forte).
  - **Ambiente:** aproxima as múltiplas reflexões da sala; sozinha não dá realismo.
- **Spot:** `GL_SPOT_CUTOFF` (abertura, 0 a 90 graus; 180 = luz comum), `GL_SPOT_DIRECTION`, `GL_SPOT_EXPONENT` (concentração).
- **Atenuação:** `fator = 1 / (kc + kl·d + kq·d²)`. **Só vale para luz posicional.** Faróis: kl = 0,1 e kq = 0,1.
- **Normais:** a luz precisa delas. `glutSolid*` e as quádricas já geram; nas malhas uso `glNormal3f`.
  `glScalef` estraga o tamanho das normais; `GL_NORMALIZE` corrige.
- **`GL_SMOOTH` × `GL_FLAT`:** smooth interpola as cores calculadas nos vértices; flat usa uma cor por face (`sceneflat.c`).
- **Luz calculada nos vértices:** por isso o piso e as paredes são malhas finas; senão os cones de luz não apareceriam.
- **`GL_COLOR_MATERIAL`:** o `glColor3f` vira a cor ambiente e difusa do material.
- **Valores padrão:** luz ambiente (0,0,0,1); difusa e especular (1,1,1,1) só na `GL_LIGHT0` (nas outras, preto);
  posição (0,0,1,0); cutoff 180; expoente 0; atenuação constante 1.
- **Limitação:** o pipeline fixo não faz sombras: a luz atravessa os objetos.

### 3.4 Textura (pesquisa: Guha cap. 12)

- **O que é:** uma imagem colada na superfície. Cada vértice recebe (s, t) da imagem; o OpenGL interpola dentro do polígono.
- **5 passos:** gerar a imagem → `glGenTextures`/`glBindTexture` → `glTexParameteri` → `glTexImage2D` →
  `glTexCoord2f` ou `gluQuadricTexture`, com `glEnable(GL_TEXTURE_2D)`.
- **`GL_REPEAT` × `GL_CLAMP`:** acima de 1, repeat repete a imagem; clamp estica a borda (Exp. 12.6 a 12.9).
- **`GL_NEAREST` × `GL_LINEAR`:** nearest pega o texel mais próximo (bordas nítidas: madeira e listras);
  linear mistura os vizinhos (borda suave: bolinhas) (Exp. 12.11).
- **`GL_MODULATE` × `GL_REPLACE`:** modulate multiplica a textura pela cor iluminada (a luz continua valendo);
  replace ignora a luz (Exp. 12.17).
- **Por que gerar no código?** Não depende de arquivo nem do caminho de execução. Os BMPs do livro não estão no servidor.

---

## Parte 4 — Perguntas prováveis (com respostas)

**Animação e transformações**

1. *Quem chama `drawScene`?* — O GLUT, quando a tela precisa ser redesenhada ou depois de `glutPostRedisplay()`.
2. *Por que timer e não idle?* — O timer tem período fixo (20 ms); com idle, a velocidade dependeria do computador.
3. *O que acontece sem `glPushMatrix` nas rodas?* — As transformações de uma roda se somariam às da próxima e elas sairiam do lugar.
4. *Em que ordem as transformações do carrinho são aplicadas?* — Da última escrita para a primeira: sobe o raio da roda,
   gira para a tangente e leva ao ponto do trajeto.
5. *Como o carrinho sabe para onde apontar?* — Pela tangente da Bézier: `atan2(-dz, dx)` dá o ângulo.
6. *Por que `u += 0.05 / sqrt(dx*dx + dz*dz)`?* — Para andar 0,05 por quadro em qualquer trecho (velocidade constante).
7. *Como o ursinho sabe que o carrinho entrou?* — Pela fase: com `fase == 1`, o `animate` aumenta `anguloCabeca`.
8. *Como a roda gira certo?* — Rola sem deslizar: ângulo = distância andada / raio da roda.

**Bézier (lista de modelagem)**

9. *O que são os pesos de Bernstein?* — `C(n,i)·(1−t)ⁿ⁻ⁱ·tⁱ`. Na cúbica: (1−t)³, 3t(1−t)², 3t²(1−t), t³. Somam 1.
10. *A curva passa pelos pontos de controle?* — Só pelo primeiro e pelo último. Os do meio puxam a curva. Ela fica no fecho convexo.
11. *Como juntar dois trechos sem quina?* — O último ponto de um é o primeiro do outro, e os pontos vizinhos da emenda ficam alinhados.
12. *O que é de Casteljau?* — Calcular o ponto da curva com interpolações lineares sucessivas entre os pontos de controle (dá o mesmo P(t)).

**Z-buffer**

13. *Explique o algoritmo do z-buffer.* — Parte 3.2.
14. *Onde ele está no código?* — `GLUT_DEPTH` no `main`, `glEnable(GL_DEPTH_TEST)` no `setup`, `glClear` no `drawScene`.
15. *O que a tecla z mostra?* — Sem o teste, o último polígono desenhado fica por cima: faces de trás aparecem na frente.
16. *Por que a amarelinha fica em y = 0,01?* — Para não ficar na mesma profundidade do piso (z-fighting).
17. *Como o carrinho some na barraca?* — A porta escura e a parede ficam na frente dele; o z-buffer esconde o que está atrás.
18. *Por que não usar só back-face culling?* — Ele só resolve um poliedro convexo fechado; a cena tem vários objetos e formas côncavas.

**Iluminação**

19. *Qual a diferença entre `w = 0` e `w = 1`?* — 0: direcional (raios paralelos, posição infinita); 1: posicional.
20. *Por que as luzes são posicionadas depois do `gluLookAt`?* — A posição passa pela modelview; depois da câmera, a luz fica presa à sala.
21. *Como os faróis andam com o carrinho?* — `glLightfv` vem depois de `posicionarCarrinho`, então usa a mesma matriz do carrinho.
22. *Explique os parâmetros de spot que você usou.* — Faróis: cutoff 25°, expoente 8, direção (1; −0,25; 0) no sistema do carrinho.
    Sirene: cutoff 30°, expoente 5. Janela: cutoff 3,3°, expoente 0.
23. *O que a atenuação faz aqui?* — Os faróis iluminam bem o brinquedo perto e pouco a parede longe.
24. *Por que a mancha do sol tem o tamanho da janela?* — A fonte está longe: os raios chegam quase paralelos, e o cone tem o raio do vidro.
25. *Difusa depende do observador?* — Não. Só depende de N e L (Lambert). A especular depende.
26. *O que é emissão? Ela ilumina os outros?* — Faz o objeto parecer aceso; não ilumina os outros objetos.
27. *Por que o piso é uma malha e não um quadrado?* — A luz é calculada nos vértices; com 4 vértices, os cones de luz não apareceriam.
28. *Para que serve `GL_NORMALIZE`?* — `glScalef` muda o tamanho das normais; ele as volta para tamanho 1.
29. *O que a luz ambiente da `GL_LIGHT0` faz?* — Clareia tudo enquanto a luz da sala está ligada; some quando ela apaga.
30. *Por que a luz atravessa a barraca?* — O pipeline fixo não calcula sombras.

**Textura**

31. *Textura foi vista em aula?* — Não. É a pesquisa pedida pelo enunciado: Guha cap. 12 (Experimenter p. 87–94).
32. *Quais os passos para usar uma textura?* — Parte 3.4.
33. *Por que a mesma imagem vira listras em pé na barraca e gomos na bola?* — Nas quádricas, s dá a volta em torno do eixo.
34. *Diferença entre `GL_NEAREST` e `GL_LINEAR`?* — Nearest: texel mais próximo, borda nítida. Linear: mistura, borda suave.
35. *Por que `GL_MODULATE`?* — Para a luz continuar valendo sobre a textura (o escuro escurece a madeira).
36. *O que `glTexCoord2f(rep * i / n, ...)` faz no piso?* — Dá a cada vértice um (s, t); com rep = 4 a imagem se repete 4 vezes (GL_REPEAT).

**Gerais**

37. *Por que não fez o feixe de luz visível?* — Precisaria de transparência (`GL_BLEND`), que não foi dada em aula.
38. *De onde veio cada parte?* — Slide 21 e a tabela do `EXPLICACAO.md`: tudo das aulas e dos seus exercícios; só a textura é pesquisa.

---

## Parte 5 — Glossário rápido

| Função | O que faz | Onde na aula |
|---|---|---|
| `glutTimerFunc` | agenda uma função depois de x ms | `rotatingHelix3.cpp`, `spots.c` |
| `glutSwapBuffers` | troca os buffers (buffer duplo) | slides de animação |
| `gluLookAt` | posiciona a câmera (olho, centro, para cima) | slides de animação e câmera |
| `gluPerspective` | projeção perspectiva (fovy, aspecto, near, far) | códigos de iluminação |
| `glPushMatrix` / `glPopMatrix` | guarda e devolve a matriz | carro, planeta, robô |
| `gluCylinder` / `gluDisk` / `gluPartialDisk` / `gluSphere` | quádricas da GLU | `gluQuadrics.cpp`, `torpedo.cpp` |
| `glutSolidCube` / `Sphere` / `Cone` / `Torus` | objetos prontos da GLUT, com normais | `glutObjects.cpp` |
| `glutStrokeCharacter` | escreve um caractere com linhas | `Fonts.cpp` |
| `glLightfv` / `glLightf` | configura uma luz | `light.c`, `spots.c` |
| `glMaterialfv` | configura o material (especular, emissão) | `material.c` |
| `glColorMaterial` | glColor vira material | `colormat.c` |
| `glNormal3f` | define a normal atual | `litTriangle.cpp`, `spots.c` (piso) |
| `glTexImage2D` | envia a imagem da textura | Guha cap. 12 (pesquisa) |
| `glTexCoord2f` | coordenada de textura do vértice | Guha cap. 12 (pesquisa) |
| `gluQuadricTexture` | coordenadas de textura nas quádricas | `texturedTorpedo.cpp` (pesquisa) |

---

## Parte 6 — Roteiro da apresentação e da demonstração

O roteiro completo está em `ROTEIRO_APRESENTACAO.md`:

- **Parte 1 (5 min):** a ordem das teclas, o que aparece na tela e o que falar
  (para que serve, como fiz e de onde veio).
- **Parte 2 (15 min):** a fala de cada slide de código, com o tempo e as linhas em destaque.
  As notas do PowerPoint têm o mesmo texto.
