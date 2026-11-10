# PP4 — Etapa 1: os 5 itens e 3 ideias de cenário

Este documento cobre os passos 1 e 2 da Etapa 1 do `CLAUDE.md`.
A Parte 1 explica os cinco itens do enunciado do zero. A Parte 2 traz três ideias de cenário.
Cada ideia mostra o que cada item usa e de qual arquivo da aula ele vem.

Próximo passo: você escolhe uma ideia e leva para a professora autorizar.

---

## Parte 1 — Os 5 itens, do zero

### Item 1 — Cenário com modelos 3D

**O que é.** Um cenário é um conjunto de objetos 3D posicionados no espaço.
Cada objeto é montado com peças simples.

**Três formas de modelar** (o enunciado aceita as três; Bézier conta como "mais sofisticada"):

| Forma | Funções | Exemplo da aula |
|---|---|---|
| Objetos da GLUT | `glutSolidSphere`, `glutSolidCube`, `glutSolidCone`, `glutSolidTorus`, `glutSolidTeapot` | `glutObjects.cpp` (Guha cap. 10), `spots.c` |
| Quádricas da GLU | `gluNewQuadric`, `gluCylinder`, `gluDisk`, `gluPartialDisk`, `gluSphere` | `gluQuadrics.cpp`, `torpedo.cpp` (Guha cap. 10) |
| Superfícies de Bézier | `glMap2f`, `glMapGrid2f`, `glEvalMesh2` | `bezierCanoe.cpp` (seu `Cap10/Canoa.cpp`) |

**Como juntar as peças.** `glTranslatef`, `glRotatef` e `glScalef` posicionam cada peça.
`glPushMatrix` e `glPopMatrix` isolam as transformações de cada uma.
A última transformação escrita é a primeira aplicada ao objeto.
Exemplo da aula: carro → rodas → parafusos (seu `Cap4/Carro2.0.cpp`).

**Atenção com a iluminação.** Use as versões `Solid` da GLUT, porque elas já geram normais.
As quádricas também geram normais. Na Bézier, ligue `glEnable(GL_AUTO_NORMAL)` (`litBezierCanoe.cpp`, Guha cap. 11).

**Câmera.** `gluPerspective` define a projeção. `gluLookAt` posiciona o observador.

**Seu exemplo mais completo:** `Cap10/Torpedo.cpp`. Ele junta cilindro e discos da GLU, um hemisfério e pás de Bézier.

### Item 2 — Animação

**O que é.** O programa muda uma variável (ângulo, posição) um pouco a cada quadro e redesenha a cena.
A troca rápida de quadros cria a ilusão de movimento.

| Forma | Como funciona | Exemplo |
|---|---|---|
| Interativa | a tecla muda a variável e chama `glutPostRedisplay()` | `rotatingHelix1.cpp`, seu `Robot.cpp` |
| Idle | `glutIdleFunc(f)` roda `f` sempre que o sistema está livre; a velocidade depende do computador | `rotatingHelix2.cpp` |
| Timer | `glutTimerFunc(ms, f, v)` roda `f` depois de `ms` milissegundos; `f` se agenda de novo | `rotatingHelix3.cpp`, `spots.c`, seu `Planet.cpp` |

Padrão do timer usado na aula:

```c
void animate(int value) {
    if (isAnimate) angulo += 2.0;          // muda o estado
    glutTimerFunc(16, animate, 1);         // agenda o próximo quadro (~60 FPS)
    glutPostRedisplay();                   // pede um novo desenho
}
```

**Buffer duplo.** `GLUT_DOUBLE` cria dois buffers de cor. O programa desenha no buffer de trás.
`glutSwapBuffers()` troca os dois no fim do desenho. Assim a tela não pisca entre quadros.

### Item 3 — Z-buffer

**O problema.** Sem teste de profundidade, o OpenGL pinta na ordem do código.
O último objeto desenhado fica por cima, mesmo que esteja atrás.

**O algoritmo** (slides de visibilidade):

1. Preencher o z-buffer com a profundidade máxima e o buffer de cor com a cor de fundo.
2. Rasterizar cada polígono.
3. Em cada pixel, comparar a profundidade nova com a guardada.
   Se a nova for **menor** (mais perto), pintar o pixel e guardar a nova profundidade. Se não, ignorar.

**Exemplo.** No pixel (100, 50), o z-buffer começa com 1,0.
O cubo tem z = 0,7 e a esfera tem z = 0,4 nesse pixel.

| Ordem | Passo 1 | Passo 2 | Resultado |
|---|---|---|---|
| cubo, depois esfera | 0,7 < 1,0 → pinta cubo, guarda 0,7 | 0,4 < 0,7 → pinta esfera, guarda 0,4 | esfera |
| esfera, depois cubo | 0,4 < 1,0 → pinta esfera, guarda 0,4 | 0,7 > 0,4 → ignora cubo | esfera |

A ordem do desenho deixa de importar. A lista L41 usa a convenção oposta: começa em −∞ e vence o **maior** z.

**Em OpenGL são três passos:**

```c
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);  // aloca o z-buffer
glEnable(GL_DEPTH_TEST);                                   // liga o teste
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);        // limpa a cada quadro
```

**Diferença para o back-face culling.** O culling remove as faces de trás de **um** poliedro convexo fechado.
O z-buffer resolve qualquer cena, pixel a pixel, inclusive com vários objetos.

Exemplos da aula: `triangles.c`, `planes.c` (`glDepthFunc`), `hidplanes.c` (pasta `05-visibilidade/codigos/`).

### Item 4 — Iluminação

**O modelo da aula** (somado para cada luz):

`I = Ia·Ka + Id·Kd·cos θ + Ie·Ke·cosⁿ φ`

- **Ambiente** (`Ia·Ka`): luz que vem de todo lado. Ilumina tudo por igual.
- **Difusa** (`Id·Kd·cos θ`): lei de Lambert. θ é o ângulo entre a normal N e a direção da luz L. Não depende do observador.
- **Especular** (`Ie·Ke·cosⁿ φ`): o brilho. φ é o ângulo entre a reflexão R e o observador. Quanto maior n (`GL_SHININESS`), menor e mais forte o brilho.

**Tipos de luz:**

| Tipo | Em OpenGL |
|---|---|
| Direcional (como o Sol) | `GL_POSITION` com `w = 0` |
| Pontual | `GL_POSITION` com `w = 1` |
| Spot (pontual com cone) | `w = 1` + `GL_SPOT_CUTOFF`, `GL_SPOT_DIRECTION`, `GL_SPOT_EXPONENT` |

**Os 4 passos da aula:** (1) normais em cada vértice, (2) criar e posicionar as luzes,
(3) definir o material, (4) definir a luz ambiente global com `glLightModelfv`.

**Detalhes que costumam cair na PO4:**

- A posição da luz passa pela matriz modelview do momento. Em `movelight.c`, o `glRotated` antes do `glLightfv(GL_POSITION)` faz a luz girar.
- A atenuação `1 / (kc + kl·d + kq·d²)` só vale para luz posicional (L42).
- `GL_EMISSION` faz o objeto parecer aceso (`material.c`; semáforo da L42).
- A luz é calculada nos **vértices**. `GL_SMOOTH` interpola as cores entre eles; `GL_FLAT` usa uma cor por face (`sceneflat.c`).
  Por isso um piso de 4 vértices não mostra o círculo de um spot. O piso precisa ser uma malha de muitos quadradinhos.

**Exemplo integrador:** `spots.c` tem 3 spots coloridos girando sobre esfera, cone e chaleira.
Ele já cobre os itens 1 a 4. Falta só a textura.

### Item 5 — Textura (pesquisa própria)

**O que é.** Uma imagem colada sobre a superfície.
Cada vértice recebe uma coordenada (s, t) da imagem, de 0 a 1. O OpenGL interpola essas coordenadas dentro do polígono.
Valores acima de 1 repetem a imagem quando o modo é `GL_REPEAT`.

**De onde vem a imagem:**

- **Procedural:** o programa gera a imagem num vetor. Exemplos: tabuleiro (`createChessboard`) e listras (`createStripes`). Não depende de arquivo.
- **BMP:** `getBMPData()` lê BMP de 24 bits sem compressão. Os BMPs do Guha não estão no servidor, então seria preciso uma imagem própria.

**Passos** (de `loadTextures.cpp`, Experimento 12.1):

```c
glGenTextures(1, textura);                       // cria o id
glBindTexture(GL_TEXTURE_2D, textura[0]);        // ativa
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, imagem);
glEnable(GL_TEXTURE_2D);
glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // mistura com a luz
```

**Como colar em cada tipo de modelo:**

| Modelo | Como | Fonte |
|---|---|---|
| Polígono | `glTexCoord2f(s, t)` antes de cada `glVertex3f` | `loadTextures.cpp` (Exp. 12.1) |
| Quádrica da GLU | `gluQuadricTexture(q, GL_TRUE)` | `texturedTorpedo.cpp` (Exp. 12.16) |
| Bézier | `glMap2f(GL_MAP2_TEXTURE_COORD_2, ...)` | `texturedTorpedo.cpp` (Exp. 12.16) |
| Malha própria | coordenada `(i/p, j/q)` em cada vértice | `texturedTorus.cpp` (Exp. 12.15) |

Os exemplos do Guha não mostram textura em objetos `glutSolid*`. Para texturizar, use quádricas ou malhas próprias.

**Textura com luz.** `GL_REPLACE` ignora a luz. `GL_MODULATE` multiplica a textura pela cor iluminada
(`fieldAndSkyLit.cpp`, Exp. 12.17). Nesse modo, o material deve ser branco, como no exemplo do Guha.

**Outras opções do cap. 12:** repetir × prender a imagem (`GL_REPEAT` × `GL_CLAMP`, Exp. 12.6 a 12.9)
e filtro `GL_NEAREST` × `GL_LINEAR` (Exp. 12.11).

**Textura animada.** O Exp. 12.15 faz a textura deslizar pelo toro somando um deslocamento às coordenadas.
Detalhe: na cópia de `texturedTorus.cpp` desta pasta, a variável `shift` aumenta no timer, mas não é somada às coordenadas.
Na nossa implementação, basta somar `shift` ao `s`.

Fonte de toda a parte de textura: Guha cap. 12, Experimenter p. 87–94, pasta `referencias/sumanta-guha/ExperimenterSource/Chapter12/`.

---

## Parte 2 — Três ideias de cenário

### Ideia A — Escrivaninha com luminária articulada

Um quarto escuro. Sobre a mesa, uma luminária articulada se move sozinha e o foco de luz percorre o tampo.
Na mesa há um tabuleiro de xadrez, uma chaleira e um globo que gira.

| Item | O que usa | De onde vem |
|---|---|---|
| 1. Modelos | mesa: 4 pernas com `glutSolidCube` escalado; luminária: base com `gluDisk` e `gluCylinder`, braços com cubos escalados, juntas com `glutSolidSphere`, cúpula com `gluCylinder` de raios diferentes; chaleira `glutSolidTeapot`; globo `gluSphere` numa haste `gluCylinder`. Com `glEnable(GL_NORMALIZE)`, porque `glScalef` muda o tamanho das normais e estraga a luz | seu `LookAtDesk.cpp` (mesa), seu `Robot.cpp` (braço articulado), `gluQuadrics.cpp`, `spots.c`; `GL_NORMALIZE` em `lightlab.c`, `teapots.c`, `teaambient.c` e Exp. 11.25 do Guha |
| 2. Animação | timer: ombro e cotovelo da luminária oscilam (`ângulo = A·sin(t)`); o globo gira em Y. Teclas: pausar, mover o braço na mão, girar a câmera | seu `Robot.cpp`, `rotatingHelix3.cpp`, `spots.c` |
| 3. Z-buffer | o braço passa na frente da cúpula; a chaleira esconde parte do globo | `triangles.c`, `hidplanes.c` |
| 4. Iluminação | `GL_LIGHT0` = spot dentro da cúpula; a posição e a direção são definidas dentro da hierarquia do braço, então a luz acompanha o movimento. `GL_LIGHT1` direcional fraca (`w = 0`) como luz do quarto. Lâmpada com `GL_EMISSION`. Chaleira com especular alto; mesa só difusa. Tampo como malha fina de faixas de triângulos com `glNormal3f(0, 1, 0)`, para o círculo do spot aparecer | `spots.c`, `movelight.c`, `plane.c`, `material.c`, L42 exercício 7 (lanterna spot na mão do robô); malha do tampo como o piso de `checkeredFloor.cpp` (Guha cap. 11) |
| 5. Textura | tabuleiro 8×8 procedural sobre o tampo, com `GL_MODULATE` para o spot clarear as casas. Cada vértice da malha recebe a coordenada `(i/N, j/N)`. A textura é desligada (`glDisable(GL_TEXTURE_2D)`) antes dos outros objetos | `loadTextures.cpp` (Exp. 12.1), `texturedTorus.cpp` (Exp. 12.15), `fieldAndSkyLit.cpp` (Exp. 12.17) |

- **Dificuldade:** média.
- **Ponto mais difícil:** chamar `glLightfv(GL_POSITION)` e `GL_SPOT_DIRECTION` no ponto certo da hierarquia do braço.
- **Vantagem:** reaproveita dois códigos seus e responde a um exercício da L42.

### Ideia B — Farol na praia, à noite

Uma ilha com um farol listrado. O feixe do farol gira e varre o mar.
Um barco dá voltas na ilha e balança nas ondas. A água se move.

| Item | O que usa | De onde vem |
|---|---|---|
| 1. Modelos | ilha: hemisfério achatado com `glScalef`; torre: `gluCylinder` com raio do topo menor; galeria: `glutSolidTorus`; cabine: `gluCylinder`; teto: `glutSolidCone`; mar: malha de quadradinhos; barco: casco de Bézier | seu `Cap2/Hemisfer.cpp`, `gluQuadrics.cpp`, seu `Cap10/Canoa.cpp` (`bezierCanoe.cpp`) |
| 2. Animação | timer: ângulo do feixe; barco em círculo (`x = R cos t`, `z = R sin t`) e sobe e desce (`y = A sin(kt)`); deslocamento da textura do mar | `spots.c`, seu `Planet.cpp` (órbita), seu `Cap3/circulo.cpp`, `texturedTorus.cpp` |
| 3. Z-buffer | o barco passa atrás e na frente da torre e da ilha | `triangles.c`, `hidplanes.c` |
| 4. Iluminação | `GL_LIGHT0` spot no topo, com direção `(cos a, −0,3, sin a)` atualizada a cada quadro; `GL_LIGHT1` luar direcional azulado e fraco (`w = 0`); lâmpada emissiva; casco com `GL_AUTO_NORMAL`; atenuação no spot (opcional) | `spots.c`, `plane.c`, `material.c`, `litBezierCanoe.cpp`, L42 (atenuação) |
| 5. Textura | torre: listras vermelhas e brancas procedurais (adaptadas de `createStripes`) com `gluQuadricTexture`; mar: ondas procedurais em tons de azul, repetidas com `GL_REPEAT` e deslocadas a cada quadro; `GL_MODULATE` | `texturedTorpedo.cpp` (Exp. 12.16), `texturedTorus.cpp` (Exp. 12.15), Exp. 12.6, `fieldAndSkyLit.cpp` (Exp. 12.17) |

- **Dificuldade:** média-alta.
- **Pontos mais difíceis:** casco de Bézier com luz, e a malha do mar fina o bastante para o feixe aparecer.
- **Vantagem:** usa as três formas de modelagem, inclusive a Bézier que o enunciado chama de "mais sofisticada". Textura animada.

### Ideia C — Sistema solar

O Sol no centro, planetas em órbita com luas, Saturno com anel e um fundo de estrelas.

| Item | O que usa | De onde vem |
|---|---|---|
| 1. Modelos | Sol, planetas e luas com `gluSphere`; anel de Saturno com `gluDisk` com furo; estrelas com `GL_POINTS` em posições sorteadas | `gluQuadrics.cpp`, seu `Planet.cpp`, `spaceTravel.cpp` (sorteio com `rand`) |
| 2. Animação | timer: translação (`glRotatef` em torno do Sol + `glTranslatef`) e rotação própria; luas na hierarquia do planeta. Teclas: pausar, acelerar, trocar a câmera | seu `Planet.cpp`, L22 (planeta com duas luas) |
| 3. Z-buffer | planetas passam na frente e atrás do Sol e uns dos outros; luas somem atrás do planeta | `triangles.c`, `hidplanes.c` |
| 4. Iluminação | `GL_LIGHT0` posicional (`w = 1`) no centro do Sol: cada planeta mostra lado dia e lado noite (Lambert). O Sol usa `GL_EMISSION`, porque a luz está dentro dele e a superfície de fora ficaria escura. Luz ambiente global baixa. Atenuação (opcional) | `plane.c`, `material.c`, L42 |
| 5. Textura | `gluQuadricTexture` nas esferas; Júpiter e Saturno com listras procedurais; Terra e Marte com BMP próprio (opcional) | `texturedTorpedo.cpp` (Exp. 12.16), `fieldAndSkyLit.cpp` (Exp. 12.17), `getBMPData` |

- **Dificuldade:** baixa-média.
- **Ponto de atenção:** é a ideia mais próxima de um exercício da aula. Tem menos risco de erro e menos variedade de modelos.

### Comparação

| | A — Escrivaninha | B — Farol | C — Sistema solar |
|---|---|---|---|
| Dificuldade | média | média-alta | baixa-média |
| Seus códigos reaproveitados | `LookAtDesk`, `Robot` | `Canoa`, `Hemisfer`, `Planet` | `Planet`, `SpaceTravel` |
| Modelagem | GLUT + GLU | GLUT + GLU + Bézier | GLU |
| Tipo de luz principal | spot que segue o braço | spot giratório + luar | luz pontual dentro do Sol |
| Textura | tabuleiro procedural | listras + mar animado (procedural) | listras procedurais + BMP opcional |
| Precisa de arquivo BMP? | não | não | só se quiser fotos dos planetas |
