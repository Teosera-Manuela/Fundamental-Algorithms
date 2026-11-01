/*
================================================================================
ENUNCIADO / ЗАДАНИЕ
================================================================================
[RUSSO]
9. Реализуйте функцию с переменным числом аргументов, определяющую для
каждой из переданных десятичных дробей (в виде значения вещественного типа
в диапазоне (0; 1)), имеет ли она в системе счисления с переданным как
параметр функции основанием k конечное представление.
Продемонстрируйте работу функции.

[PORTUGUÊS]
9. Função variádica que, para cada fração decimal passada (real em (0;1)),
determina se ela tem representação finita na base 'k' fornecida.
Use epsilon para comparação de floats.
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
    STATUS_ERR_INVALID_BASE,
    STATUS_ERR_INVALID_EPSILON
} StatusCode;

// ============================================================================
// FUNÇÃO LÓGICA: Verifica se a fração tem representação finita
// ============================================================================
// Lógica: Multiplicamos a fração pela base repetidamente.
// Se em algum momento o resultado for um número inteiro (ou muito próximo disso),
// significa que a fração "terminou".
// Se ela começar a repetir os mesmos valores (loop infinito), não é finita.
// Para evitar loops infinitos verdadeiros, limitamos a 100 iterações.
// ============================================================================
static int has_finite_representation(double fraction, int base, double epsilon) {
    // O valor deve estar em (0, 1)
    if (fraction <= 0.0 || fraction >= 1.0) return 0;

    double current = fraction;

    // Testa até 100 vezes (para bases grandes ou frações longas)
    for (int i = 0; i < 100; i++) {
        // Multiplica pela base
        current *= base;

        // Se o valor atual for muito próximo de um inteiro (usando epsilon)
        // Ex: 0.5 * 2 = 1.0 (é inteiro!)
        double rounded = round(current);
        if (fabs(current - rounded) < epsilon) {
            return 1; // Representação finita!
        }

        // Remove a parte inteira para a próxima iteração
        current = current - floor(current);
    }

    return 0; // Após 100 iterações, consideramos infinita/periódica
}

// ============================================================================
// FUNÇÃO PRINCIPAL (Variádica)
// ============================================================================
StatusCode check_fractions(int base, double epsilon, int count, ...) {
    // Validações iniciais
    if (base < 2 || base > 36) return STATUS_ERR_INVALID_BASE;
    if (epsilon <= 0.0) return STATUS_ERR_INVALID_EPSILON;

    va_list args;
    va_start(args, count); // Inicializa a lista variádica

    printf("--- Verificando %d fracoes na base %d (epsilon=%.1e) ---\n", count, base, epsilon);

    for (int i = 0; i < count; i++) {
        // Lê o próximo double da lista de argumentos
        double fraction = va_arg(args, double);

        // Chama a lógica pura de verificação
        int is_finite = has_finite_representation(fraction, base, epsilon);

        // I/O: Imprime o resultado
        if (is_finite) {
            printf("Fracao %.4f: TEM representacao finita na base %d.\n", fraction, base);
        } else {
            printf("Fracao %.4f: NAO TEM representacao finita na base %d.\n", fraction, base);
        }
    }

    va_end(args); // Limpa a lista
    return STATUS_OK;
}

// ============================================================================
// FUNÇÃO MAIN (Demonstração)
// ============================================================================
int main(void) {
    double epsilon = 1e-9; // Precisão para comparação

    printf("--- TESTE 1: Base 2 ---\n");
    // Na base 2:
    // 0.5 = 0.1 (Finito)
    // 0.25 = 0.01 (Finito)
    // 0.1 = 0.000110011... (Infinito, pois 1/10 não tem só fatores 2)
    // 0.3 = 0.010011... (Infinito)
    StatusCode st = check_fractions(2, epsilon, 4, 0.5, 0.25, 0.1, 0.3);
    if (st != STATUS_OK) printf("Erro na execucao.\n");


    printf("\n--- TESTE 2: Base 10 ---\n");
    // Na base 10:
    // 0.5 (Finito)
    // 0.25 (Finito)
    // 0.1 (Finito)
    // 0.3333... (Infinito)
    st = check_fractions(10, epsilon, 4, 0.5, 0.25, 0.1, 1.0/3.0);
    if (st != STATUS_OK) printf("Erro na execucao.\n");


    printf("\n--- TESTE 3: Base 16 ---\n");
    // Na base 16:
    // 0.5 (Finito: 0.8)
    // 0.0625 (Finito: 0.1)
    // 0.2 (Infinito: 1/5 não tem fatores 2 ou 4)
    st = check_fractions(16, epsilon, 3, 0.5, 0.0625, 0.2);
    if (st != STATUS_OK) printf("Erro na execucao.\n");

    return 0;
}
