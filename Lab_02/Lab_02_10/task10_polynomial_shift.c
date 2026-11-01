/*
================================================================================
ENUNCIADO / ЗАДАНИЕ
================================================================================
[RUSSO]
10. Реализуйте функцию с переменным числом аргументов, находящую
переразложение многочлена со степенями значения x по степеням значения (x - a).
Входными аргументами являются точность e, значение a, указатель для записи
массива коэффициентов результата, степень многочлена и его коэффициенты.

[PORTUGUÊS]
10. Função variádica que encontra a reprecipitação de um polinômio de potências
de x para potências de (x - a). Argumentos: epsilon, valor de a, ponteiro para
o array resultante, grau do polinômio (int) e coeficientes (reais) variádicos.
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

// ============================================================================
// DEFINIÇÃO DE STATUS CODES
// ============================================================================
typedef enum {
    STATUS_OK = 0,
    STATUS_ERR_MEMORY,
    STATUS_ERR_NULL_PTR,
    STATUS_ERR_INVALID_DEG,
    STATUS_ERR_INVALID_EPSILON
} StatusCode;

// ============================================================================
// FUNÇÕES AUXILIARES MATEMÁTICAS
// ============================================================================

// Calcula o coeficiente binomial C(n, k) para o Binômio de Newton
// C(n, k) = n! / (k! * (n-k)!)
static long long binomial_coeff(int n, int k) {
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;

    // Otimização: C(n, k) == C(n, n-k)
    if (k > n - k) k = n - k;

    long long res = 1;
    for (int i = 1; i <= k; ++i) {
        res = res * (n - i + 1) / i;
    }
    return res;
}

// Calcula a potência de 'a' elevado a 'exp'
static double my_pow(double base, int exp) {
    double result = 1.0;
    for (int i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}

// ============================================================================
// FUNÇÃO PRINCIPAL DO EXERCÍCIO (Variádica)
// ============================================================================
StatusCode polynomial_shift(double epsilon, double a, double** out_coeffs, int degree, ...) {
    // 1. Validação rigorosa dos argumentos
    if (out_coeffs == NULL) return STATUS_ERR_NULL_PTR;
    if (degree < 0) return STATUS_ERR_INVALID_DEG;
    if (epsilon <= 0.0) return STATUS_ERR_INVALID_EPSILON;

    // Inicializa a saída por segurança
    *out_coeffs = NULL;

    // 2. Alocação de memória para os coeficientes originais (f) e novos (g)
    // O polinômio de grau 'n' tem 'n+1' coeficientes (do x^0 até x^n)
    double* f = (double*)malloc((degree + 1) * sizeof(double));
    if (f == NULL) return STATUS_ERR_MEMORY;

    double* g = (double*)calloc((degree + 1), sizeof(double)); // calloc zera a memória
    if (g == NULL) {
        free(f);
        return STATUS_ERR_MEMORY;
    }

    // 3. Leitura dos coeficientes variádicos (f0, f1, ..., fn)
    va_list args;
    va_start(args, degree);
    for (int i = 0; i <= degree; i++) {
        f[i] = va_arg(args, double);
    }
    va_end(args);

    // 4. Matemática: f(x) = sum( f_i * x^i )
    // Queremos f(x) em termos de (x-a).
    // Sabemos que x = (x - a) + a. Substituindo x^i:
    // ((x-a) + a)^i = sum( C(i,j) * (x-a)^j * a^(i-j) )
    // Então, distribuímos os coeficientes f_i sobre os g_j.
    for (int i = 0; i <= degree; i++) {
        for (int j = 0; j <= i; j++) {
            long long comb = binomial_coeff(i, j);
            double a_pow = my_pow(a, i - j);

            // Acumula no coeficiente g_j
            g[j] += f[i] * (double)comb * a_pow;
        }
    }

    // 5. Tratamento com Epsilon (Regra do laboratório)
    // Zera coeficientes que são "lixo" de ponto flutuante (muito próximos de zero)
    for (int i = 0; i <= degree; i++) {
        if (fabs(g[i]) < epsilon) {
            g[i] = 0.0;
        }
    }

    // 6. Limpeza e retorno
    free(f); // O array 'f' não é mais necessário
    *out_coeffs = g; // Retorna o array 'g' alocado
    return STATUS_OK;
}

// ============================================================================
// FUNÇÃO AUXILIAR DE I/O: Imprime o polinômio formatado
// ============================================================================
static void print_polynomial(const double* coeffs, int degree, const char* var_name) {
    int first = 1;
    for (int i = degree; i >= 0; i--) {
        if (coeffs[i] == 0.0) continue; // Pula termos nulos

        if (!first && coeffs[i] > 0.0) printf(" + ");
        if (coeffs[i] < 0.0) printf(" - ");

        double abs_val = fabs(coeffs[i]);

        // Se não for 1 (ou for o termo independente), imprime o coeficiente
        if (i == 0 || abs_val != 1.0) {
            printf("%.2f", abs_val);
        }

        if (i > 0) {
            printf("%s", var_name);
            if (i > 1) printf("^%d", i);
        }
        first = 0;
    }
    if (first) printf("0"); // Polinômio nulo
    printf("\n");
}

// ============================================================================
// FUNÇÃO MAIN (Demonstração)
// ============================================================================
int main(void) {
    // Exemplo: f(x) = 2 + 3x + x^2 (Grau 2)
    // Queremos f(x) em potências de (x - 2)
    int degree = 2;
    double a = 2.0;
    double epsilon = 1e-9;
    double* result_coeffs = NULL;

    printf("--- Demonstracao do Exercicio 10 ---\n");
    printf("Polinomio original: f(x) = 1.0*x^2 + 3.0*x + 2.0\n");
    printf("Mudando para base (x - %.1f):\n\n", a);

    // Chamando a função variádica: (epsilon, a, ponteiro_saida, grau, f0, f1, f2)
    StatusCode status = polynomial_shift(epsilon, a, &result_coeffs, degree, 2.0, 3.0, 1.0);

    // Tratamento de erros via switch (Regra do laboratório)
    switch (status) {
        case STATUS_OK:
            printf("Sucesso!\n");
            printf("f(x) = ");
            print_polynomial(result_coeffs, degree, "(x-2)");

            // Explicação matemática do resultado:
            // f(x) = (x-2)^2 + 7(x-2) + 12
            // Se expandirmos: (x^2 - 4x + 4) + 7x - 14 + 12 = x^2 + 3x + 2. Correto!

            free(result_coeffs); // Libera a memória alocada pela função
            break;
        case STATUS_ERR_MEMORY:
            fprintf(stderr, "Erro: Falha na alocacao de memoria.\n");
            break;
        case STATUS_ERR_INVALID_EPSILON:
            fprintf(stderr, "Erro: Epsilon invalido.\n");
            break;
        case STATUS_ERR_INVALID_DEG:
            fprintf(stderr, "Erro: Grau invalido.\n");
            break;
        default:
            fprintf(stderr, "Erro desconhecido.\n");
            break;
    }

    return 0;
}
