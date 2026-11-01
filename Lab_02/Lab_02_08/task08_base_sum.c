/*
================================================================================
ENUNCIADO / ЗАДАНИЕ
================================================================================
[RUSSO]
8. Реализуйте функцию с переменным числом аргументов, вычисляющую сумму
переданных целых неотрицательных чисел в заданной системе счисления с
основанием [2..36]. Реализуйте функцию сложения в столбик двух чисел.

[PORTUGUÊS]
8. Função variádica que calcula a soma de números inteiros não-negativos
em uma base [2..36] fornecida. Os números vêm como strings. Implemente uma
função de "soma em coluna" (escola) de dois números na base dada.
O resultado não deve ter zeros à esquerda.
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

// ============================================================================
// DEFINIÇÃO DE STATUS CODES
// ============================================================================
typedef enum {
    STATUS_OK = 0,
    STATUS_ERR_MEMORY,
    STATUS_ERR_NULL_PTR,
    STATUS_ERR_INVALID_BASE
} StatusCode;

// ============================================================================
// FUNÇÕES AUXILIARES
// ============================================================================

// Converte um caractere (ex: 'A' ou '5') para seu valor inteiro
static int char_to_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'z') return c - 'a' + 10;
    if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
    return 0;
}

// Converte um valor inteiro para o caractere correspondente
static char val_to_char(int v) {
    if (v >= 0 && v <= 9) return '0' + v;
    if (v >= 10 && v <= 35) return 'a' + (v - 10);
    return '?';
}

// Inverte uma string no lugar (in-place)
static void reverse_str(char* str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
}

// ============================================================================
// FUNÇÃO OBRIGATÓRIA: Soma em coluna de 2 números em uma base
// ============================================================================
// Retorna uma string alocada dinamicamente com o resultado.
StatusCode add_two_in_base(const char* num1, const char* num2, int base, char** out_result) {
    if (num1 == NULL || num2 == NULL || out_result == NULL) return STATUS_ERR_NULL_PTR;
    if (base < 2 || base > 36) return STATUS_ERR_INVALID_BASE;

    int len1 = strlen(num1);
    int len2 = strlen(num2);
    int max_len = (len1 > len2 ? len1 : len2) + 1; // +1 para o possível "vai um"

    // Aloca memória para o resultado (+1 para o '\0')
    char* result = (char*)malloc((max_len + 1) * sizeof(char));
    if (result == NULL) return STATUS_ERR_MEMORY;

    int i = len1 - 1;
    int j = len2 - 1;
    int k = 0;
    int carry = 0; // O famoso "vai um"

    // Enquanto houver dígitos ou o carry for maior que zero
    while (i >= 0 || j >= 0 || carry > 0) {
        int val1 = (i >= 0) ? char_to_val(num1[i]) : 0;
        int val2 = (j >= 0) ? char_to_val(num2[j]) : 0;

        int sum = val1 + val2 + carry;
        carry = sum / base; // Quanto vai para a próxima coluna
        int current_digit = sum % base; // O que fica na coluna atual

        result[k++] = val_to_char(current_digit);

        i--;
        j--;
    }
    result[k] = '\0';

    // Como somamos da direita pra esquerda, a string gerada está invertida.
    // Vamos inverter para ficar na ordem correta.
    reverse_str(result);

    *out_result = result;
    return STATUS_OK;
}

// ============================================================================
// FUNÇÃO PRINCIPAL VARIÁDICA: Soma N números
// ============================================================================
// Recebe a base, a quantidade de números, e as strings.
StatusCode sum_multiple_numbers(int base, int count, char** out_result, ...) {
    if (out_result == NULL) return STATUS_ERR_NULL_PTR;
    if (base < 2 || base > 36) return STATUS_ERR_INVALID_BASE;

    if (count <= 0) {
        // Se não há números, a soma é "0"
        char* zero = (char*)malloc(2);
        if (!zero) return STATUS_ERR_MEMORY;
        strcpy(zero, "0");
        *out_result = zero;
        return STATUS_OK;
    }

    va_list args;
    va_start(args, out_result);

    // Começamos com a string "0" como acumulador
    char* accumulator = (char*)malloc(2);
    if (!accumulator) { va_end(args); return STATUS_ERR_MEMORY; }
    strcpy(accumulator, "0");

    for (int i = 0; i < count; i++) {
        char* next_num = va_arg(args, char*);
        if (next_num == NULL) continue;

        char* new_sum = NULL;
        StatusCode st = add_two_in_base(accumulator, next_num, base, &new_sum);

        free(accumulator); // Libera o acumulador antigo
        if (st != STATUS_OK) {
            va_end(args);
            return st; // Erro ao somar
        }
        accumulator = new_sum; // Atualiza o acumulador
    }

    va_end(args);

    // Remove zeros à esquerda que possam ter sobrado (ex: "000123" -> "123")
    char* final_result = accumulator;
    while (*final_result == '0' && *(final_result + 1) != '\0') {
        final_result++;
    }

    // Copia o resultado final limpo para uma nova alocação exata
    char* clean_result = (char*)malloc(strlen(final_result) + 1);
    if (!clean_result) {
        free(accumulator);
        return STATUS_ERR_MEMORY;
    }
    strcpy(clean_result, final_result);

    free(accumulator);
    *out_result = clean_result;
    return STATUS_OK;
}

// ============================================================================
// FUNÇÃO MAIN (Demonstração)
// ============================================================================
int main(void) {
    char* result = NULL;

    // Teste 1: Soma na base 10 (Decimal)
    // 125 + 975 + 50 = 1150
    printf("--- Teste 1: Base 10 ---\n");
    StatusCode st = sum_multiple_numbers(10, 3, &result, "125", "975", "50");
    if (st == STATUS_OK) {
        printf("125 + 975 + 50 = %s\n", result);
        free(result);
    }

    // Teste 2: Soma na base 16 (Hexadecimal)
    // FF + 1 = 100 (em hex)
    printf("\n--- Teste 2: Base 16 ---\n");
    st = sum_multiple_numbers(16, 2, &result, "FF", "1");
    if (st == STATUS_OK) {
        printf("FF + 1 = %s\n", result);
        free(result);
    }

    // Teste 3: Big Numbers (Números Gigantes na Base 2)
    // 1111 (15) + 1 = 10000 (16)
    printf("\n--- Teste 3: Base 2 (Gigantes) ---\n");
    st = sum_multiple_numbers(2, 2, &result, "1111", "1");
    if (st == STATUS_OK) {
        printf("1111 + 1 = %s\n", result);
        free(result);
    }

    // Teste 4: Número gigante na base 10
    // 99999999999999999999 + 1 = 100000000000000000000
    printf("\n--- Teste 4: Big Numbers (Base 10) ---\n");
    st = sum_multiple_numbers(10, 2, &result, "99999999999999999999", "1");
    if (st == STATUS_OK) {
        printf("Gigante + 1 = %s\n", result);
        free(result);
    }

    return 0;
}
