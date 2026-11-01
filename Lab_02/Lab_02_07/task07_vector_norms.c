/*
================================================================================
ENUNCIADO / ЗАДАНИЕ
================================================================================
[RUSSO]
7. Напишите функцию с переменным числом аргументов, на вход которой
передается: размерность пространства n; экземпляры структур, содержащие
координаты векторов; указатели на функции, вычисляющие нормы векторов.
Ваша функция должна для каждой переданной нормы вернуть самый длинный
вектор (если таких несколько, вернуть их все).

[PORTUGUÊS]
7. Função variádica que recebe: dimensão n; instâncias de structs com
coordenadas de vetores; ponteiros para funções de norma. Para cada função
de norma, retornar o(s) vetor(es) de maior norma.
Fórmulas: ||x||_inf = max|x_j|, ||x||_p = (sum |x_j|^p)^(1/p), ||x||_A = sqrt((Ax,x))
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

// ============================================================================
// ESTRUTURAS E TIPOS
// ============================================================================
// Tipo universal para o ponteiro de função de norma.
// Recebe o vetor (array de doubles), a dimensão n, e um parâmetro extra 'p'
// (usado para a norma p e ignorado nas outras).
typedef double (*NormFunc)(const double* vec, int n, double p);

// Estrutura que representa um vetor de n dimensões
typedef struct {
    double* coords;
} Vector;

// ============================================================================
// FUNÇÕES DE CÁLCULO DE NORMAS (Para serem usadas como ponteiros)
// ============================================================================

// Norma do Infinito: max |x_j|
double norm_infinity(const double* vec, int n, double p) {
    (void)p; // Ignorado, mas mantido para compatibilidade de assinatura
    double max_val = 0.0;
    for (int i = 0; i < n; i++) {
        double abs_val = fabs(vec[i]);
        if (abs_val > max_val) max_val = abs_val;
    }
    return max_val;
}

// Norma P: (sum |x_j|^p)^(1/p)
double norm_p(const double* vec, int n, double p) {
    if (p < 1.0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += pow(fabs(vec[i]), p);
    }
    return pow(sum, 1.0 / p);
}

// Norma A (Matriz Positiva Definida): Exemplo simplificado ||x||_A = sqrt((Ax,x))
// Para simplificar o teste, usaremos uma matriz identidade multiplicada por 2.
// Então ||x||_A = sqrt(2 * sum(x_i^2)) = sqrt(2) * ||x||_2
double norm_a(const double* vec, int n, double p) {
    (void)p;
    double sum_sq = 0.0;
    for (int i = 0; i < n; i++) {
        sum_sq += vec[i] * vec[i];
    }
    // Simulação de A = 2 * I
    return sqrt(2.0 * sum_sq);
}

// ============================================================================
// FUNÇÃO PRINCIPAL DO EXERCÍCIO (Variádica)
// ============================================================================
// O truque aqui é ler os argumentos variádicos em blocos:
// 1º bloco: 'n' e quantidade de normas.
// 2º bloco: os ponteiros de função.
// 3º bloco: os vetores (até encontrar NULL).
// ============================================================================
void find_max_norm_vectors(int n, int num_norms, ...) {
    va_list args;
    va_start(args, num_norms);

    // 1. LER AS FUNÇÕES DE NORMA
    // Aloca um array para guardar os ponteiros de função
    NormFunc* norms = (NormFunc*)malloc(num_norms * sizeof(NormFunc));
    if (!norms) { va_end(args); return; }

    for (int i = 0; i < num_norms; i++) {
        norms[i] = va_arg(args, NormFunc);
    }

    // 2. LER OS VETORES
    // Vamos guardar todos os vetores em um array dinâmico
    int capacity = 10;
    int count = 0;
    Vector* vectors = (Vector*)malloc(capacity * sizeof(Vector));
    if (!vectors) { free(norms); va_end(args); return; }

    while (1) {
        Vector* v = va_arg(args, Vector*);
        if (v == NULL) break; // Fim da lista de vetores

        if (count == capacity) {
            capacity *= 2;
            Vector* temp = (Vector*)realloc(vectors, capacity * sizeof(Vector));
            if (!temp) break;
            vectors = temp;
        }
        vectors[count++] = *v;
    }
    va_end(args);

    // 3. PROCESSAR CADA NORMA
    // Para cada função de norma, encontraremos o maior valor e guardaremos os índices
    for (int k = 0; k < num_norms; k++) {
        NormFunc func = norms[k];
        double p_param = (func == norm_p) ? 3.0 : 0.0; // Usaremos p=3 para testar a Norma P
        double max_val = -1.0;

        // Descobre o valor máximo
        for (int i = 0; i < count; i++) {
            double val = func(vectors[i].coords, n, p_param);
            if (val > max_val) max_val = val;
        }

        // Imprime o resultado para esta norma
        printf("--- Resultado para a norma %d (Maior valor: %.3f) ---\n", k+1, max_val);

        // Verifica se há empates e imprime os vetores
        for (int i = 0; i < count; i++) {
            double val = func(vectors[i].coords, n, p_param);
            // Comparação com epsilon (regra do lab) para floats
            if (fabs(val - max_val) < 1e-6) {
                printf("  Vetor [");
                for (int j = 0; j < n; j++) {
                    printf("%.1f%s", vectors[i].coords[j], j < n-1 ? ", " : "");
                }
                printf("] (Norma = %.3f)\n", val);
            }
        }
    }

    // Limpeza
    free(norms);
    free(vectors);
}

// ============================================================================
// FUNÇÃO MAIN (Demonstração)
// ============================================================================
int main(void) {
    int n = 2; // Dimensão dos vetores (2D)

    // Criando alguns vetores de teste
    double c1[] = {3.0, 4.0};
    Vector v1 = {c1};

    double c2[] = {5.0, 0.0};
    Vector v2 = {c2};

    double c3[] = {1.0, 1.0};
    Vector v3 = {c3};

    printf("Vetores de teste (Dimensao %d):\n", n);
    printf("V1 = [3, 4]\n");
    printf("V2 = [5, 0]\n");
    printf("V3 = [1, 1]\n\n");

    /*
     Chamada da função variádica.
     Parâmetros:
     - n = 2
     - num_norms = 3 (vamos testar 3 normas)
     - norm_infinity, norm_p, norm_a (ponteiros para funções)
     - v1, v2, v3 (ponteiros para as structs)
     - NULL (marcador de fim dos vetores)
    */
    find_max_norm_vectors(n, 3,
                          norm_infinity,
                          norm_p,
                          norm_a,
                          &v1, &v2, &v3,
                          (Vector*)NULL); // Cast para evitar warning do compilador

    return 0;
}
