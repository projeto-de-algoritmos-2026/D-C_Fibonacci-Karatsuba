# Fibonacci-Karatsuba

**Número da Lista:** X<br>
**Conteúdo da Disciplina:** Dividir e Conquistar<br>

## Alunos

| Matrícula | Aluno           |
| --------- | --------------- |
| xx/xxxxxx | xxxx xxxx xxxxx |
| xx/xxxxxx | xxxx xxxx xxxxx |

## Sobre

O projeto calcula números de Fibonacci gigantes, com milhões de dígitos, e usa esse cálculo para comparar dois algoritmos de multiplicação de inteiros grandes:

- **Multiplicação escolar:** o método do papel, com "dígitos" de 32 bits (base 2³²). Complexidade O(n²).
- **Multiplicação de Karatsuba:** algoritmo de dividir e conquistar que troca 4 multiplicações de metade do tamanho por 3. Complexidade O(n^log₂3) ≈ O(n^1,585).

### O algoritmo de Karatsuba

Cada fator é dividido em uma metade alta e uma metade baixa, com m limbs (B = 2³²):

```
x = x1·B^m + x0        y = y1·B^m + y0

z0 = x0·y0
z2 = x1·y1
z1 = (x0 + x1)·(y0 + y1) − z0 − z2

x·y = z2·B^(2m) + z1·B^m + z0
```

- **Dividir:** separar cada fator nas metades `x1, x0` e `y1, y0`.
- **Conquistar:** calcular `z0`, `z2` e `(x0 + x1)·(y0 + y1)` recursivamente, com 3 multiplicações em vez de 4.
- **Combinar:** obter `z1` por subtração e somar as três partes deslocadas, em tempo O(n).

Recorrência e solução pelo Teorema Mestre (a = 3, b = 2, f(n) = O(n)):

```
T(n) = 3·T(n/2) + O(n)   ⇒   T(n) = O(n^log₂3) ≈ O(n^1,585)
```

Abaixo de 32 limbs, a recursão para e usa a multiplicação escolar, que é mais rápida para números pequenos. O limite foi escolhido por medição.

### Fibonacci por fast doubling

```
F(2k)   = F(k)·(2·F(k+1) − F(k))
F(2k+1) = F(k)² + F(k+1)²
```

A partir de (F(k), F(k+1)), o algoritmo percorre os bits de n do mais alto para o mais baixo, dobrando k a cada bit. São 3 multiplicações por bit, e os números dobram de tamanho a cada passo, então o tempo é dominado pelas últimas multiplicações. Por isso o custo de F(n) segue o custo da multiplicação: O(n²) com o escolar e O(n^1,585) com o Karatsuba.

### Arquitetura

```
include/bignum/   API da biblioteca (domínio)
  BigInt.hpp                  inteiro não negativo de precisão arbitrária
  Multiplier.hpp              interface com duas implementações
  SchoolbookMultiplier.hpp    multiplicação escolar
  KaratsubaMultiplier.hpp     multiplicação de Karatsuba
  Fibonacci.hpp               F(n) por fast doubling
src/              implementações
app/              CLI e benchmark (somente entrada e saída)
tests/            testes com Google Test
cmake/            opções de compilação e alvos de formatação e análise
```

O `Fibonacci` recebe o `Multiplier` pelo construtor. Assim, o mesmo cálculo roda com qualquer um dos dois algoritmos, e a comparação entre eles não depende de mais nada.

### Resultados

Medidos com `fibonacci_benchmark 3`, em Release (`-O3`). O expoente empírico é a inclinação da reta de log(tempo) × log(tamanho), calculada por regressão linear sobre as 4 maiores medições.

|                                                              | Escolar | Karatsuba | Teoria          |
| ------------------------------------------------------------ | ------- | --------- | --------------- |
| Expoente empírico na multiplicação                           | 2,02    | 1,58      | 2 e 1,585       |
| Expoente empírico em F(n)                                    | 2,01    | 1,61      | 2 e 1,585       |
| Multiplicar dois números de 65.536 limbs (2 milhões de bits) | 3,33 s  | 0,25 s    | 13× mais rápido |
| F(4.096.000), com 855 mil dígitos                            | 6,36 s  | 0,61 s    | 10× mais rápido |

Até cerca de n = 16.000 os dois tempos coincidem, porque os números ainda estão abaixo do limite de 32 limbs e o Karatsuba usa a multiplicação escolar.

## Screenshots

Adicione 3 ou mais screenshots do projeto em funcionamento.

## Instalação

**Linguagem:** C++17<br>
**Framework:** nenhum. Build com CMake, testes com Google Test (baixado automaticamente pelo CMake).<br>

Pré-requisitos, em Linux ou WSL:

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build
```

Compilação para uso e benchmark (Release):

```bash
cmake --preset release
cmake --build --preset release
```

Compilação e execução dos testes (Debug):

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

Na primeira configuração com testes, o CMake baixa o Google Test, então essa etapa precisa de internet.

## Uso

### Calcular F(n)

```bash
./build/release/fibonacci <n> [escolar|karatsuba] [--completo]
```

| Argumento                | Significado                                                                                               |
| ------------------------ | --------------------------------------------------------------------------------------------------------- |
| `n`                      | Índice do termo                                                                                           |
| `escolar` ou `karatsuba` | Algoritmo de multiplicação. O padrão é `karatsuba`                                                        |
| `--completo`             | Mostra todos os dígitos. Sem ele, números grandes aparecem só com os 30 primeiros e os 30 últimos dígitos |

Exemplo:

```
$ ./build/release/fibonacci 1000000
F(1000000) = 195328212870775773163201494759...719893411568996526838242546875
Digitos:        208988
Limbs:          21696
Algoritmo:      karatsuba
Tempo do F(n):  73.0553 ms
Tempo decimal:  1111.83 ms
```

Para comparar os algoritmos, rode o mesmo `n` com `escolar` e com `karatsuba` e compare o **Tempo do F(n)**. O **Tempo decimal** é a conversão do número para texto, igual nos dois casos.

### Benchmark

```bash
./build/release/fibonacci_benchmark 3 | tee resultados.csv
```

O argumento é o tempo máximo, em segundos, de uma medição: quando uma medição passa desse valor, a série daquele algoritmo termina. Com 3, o benchmark leva alguns minutos.

A saída é um CSV com duas partes: a multiplicação isolada (de 16 a 65.536 limbs) e o cálculo de F(n) (de n = 1.000 a 64 milhões). Cada série termina com o expoente empírico. Para gerar gráficos, use escala logarítmica nos dois eixos: cada algoritmo vira uma reta cuja inclinação é o expoente.

### Formatação e análise estática

```bash
cmake --build --preset debug --target check   # verifica a formatação
cmake --build --preset debug --target format  # formata os arquivos
cmake --build --preset debug --target lint    # formatação + clang-tidy
```

## Outros

- **Testes:** 112 testes cobrem o `BigInt` (construção, comparação e aritmética), os dois multiplicadores e o Fibonacci. O Karatsuba é comparado com a multiplicação escolar (oráculo) com vários limites de recursão, números aleatórios de até 3.000 dígitos e o pior caso de carry. O Fibonacci é conferido com valores conhecidos, com uma versão que só usa somas e com a identidade de Cassini.
- **Limitações:** o `toString()` é O(n²), então converter F(n) para decimal fica lento para n muito grande. O CLI mostra esse tempo separado do cálculo, e o benchmark mede só o cálculo. A divisão do `BigInt` (`/` e `%`) é feita bit a bit; o Fibonacci não usa divisão, então isso não afeta os resultados.
