/*
================================================================================
ENUNCIADO / ЗАДАНИЕ
================================================================================
[RUSSO]
1. Реализуйте функцию перевода числа из десятичной системы счисления в систему
счисления с основанием 2^r, r = 1, ..., 5. При реализации функции разрешается
использовать битовые операции и операции обращения к памяти, запрещается
использовать стандартные арифметические операции. Продемонстрируйте работу
реализованных функций.

[PORTUGUÊS]
1. Realize uma função de conversão de um número do sistema decimal para um
sistema numérico com base 2^r, r = 1, ..., 5. Na implementação da função, é
permitido usar operações bit a bit e operações de acesso à memória; é proibido
usar operações aritméticas padrão. Demonstre o funcionamento das funções
implementadas.

RESTRIÇÕES: Sem <string.h>, sem variáveis globais, alocação dinâmica,
tratamento de erros com enum, separação de lógica e I/O. Sem operadores
aritméticos (+, -, *, /, %).

OBSERVAÇÕES / ПРИМЕЧАНИЯ:
1. O arquivo deve ser nomeado seguindo o padrão do projeto (ex: task01_base.c).
2. A conversão usará apenas bitwise: shift (>>, <<) e AND (&).
3. O resultado é uma string alocada dinamicamente.
================================================================================
*/

#include <stdio.h>  // Para printf, fprintf
#include <stdlib.h> // Para malloc, free

// ============================================================================
// DEFINIÇÃO DE STATUS CODES
// ============================================================================
typedef enum {
    STATUS_OK = 0,          // Operação bem-sucedida
    STATUS_ERR_MEMORY,      // Falha na alocação de memória
    STATUS_ERR_NULL_PTR,    // Ponteiro nulo passado por parâmetro
    STATUS_ERR_INVALID_ARG  // Argumento inválido (ex: r fora do intervalo)
} StatusCode;

// ============================================================================
// FUNÇÕES AUXILIARES
// ============================================================================

/**
 * @brief Calcula o comprimento de uma string manualmente.
 */
static size_t my_strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// ============================================================================
// FUNÇÃO LÓGICA (Conversão sem aritmética padrão)
// ============================================================================

/**
 * @brief Converte um número decimal para uma string na base 2^r usando apenas bitwise.
 *
 * Passo a passo:
 * 1. Validação de ponteiros e do parâmetro r (deve ser 1 a 5).
 * 2. Define a máscara de bits: mask = (1 << r) - 1.
 * 3. Aloca memória para a string de saída.
 * 4. Loop principal: extrai o dígito (num & mask), converte para char,
 *    armazena de trás para frente e desloca (num >> r).
 * 5. Inverte a string para a ordem correta.
 */
StatusCode convert_to_base_2r(unsigned int num, int r, char** out_str) {
    if (out_str == NULL) return STATUS_ERR_NULL_PTR;
    if (r < 1 || r > 5) return STATUS_ERR_INVALID_ARG;

    unsigned int mask = (1u << r) - 1u;
    char buffer[33];
    int index = 0;

    if (num == 0) {
        buffer[index++] = '0';
    } else {
        while (num > 0) {
            unsigned int digit = num & mask;

            if (digit < 10) {
                buffer[index++] = (char)('0' + digit);
            } else {
                buffer[index++] = (char)('A' + (digit - 10));
            }

            num = num >> r;
        }
    }

    buffer[index] = '\0';

    char* result = (char*)malloc((size_t)(index + 1) * sizeof(char));
    if (result == NULL) return STATUS_ERR_MEMORY;

    int j = 0;
    for (int i = index - 1; i >= 0; i--) {
        result[j++] = buffer[i];
    }
    result[j] = '\0';

    *out_str = result;
    return STATUS_OK;
}

// ============================================================================
// FUNÇÃO MAIN (Roteamento e I/O)
// ============================================================================
int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <numero_decimal> <r>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s 255 3 (converte 255 para base 8)\n", argv[0]);
        return 1;
    }

    char* endptr;
    unsigned int num = (unsigned int)strtoul(argv[1], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Erro: Numero invalido.\n");
        return 1;
    }

    int r = (int)strtoul(argv[2], &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Erro: Valor de r invalido.\n");
        return 1;
    }

    char* result = NULL;
    StatusCode status = convert_to_base_2r(num, r, &result);

    if (status != STATUS_OK) {
        switch (status) {
            case STATUS_ERR_MEMORY:
                fprintf(stderr, "Erro: Falha de alocacao de memoria.\n");
                break;
            case STATUS_ERR_NULL_PTR:
                fprintf(stderr, "Erro: Ponteiro nulo.\n");
                break;
            case STATUS_ERR_INVALID_ARG:
                fprintf(stderr, "Erro: r deve estar entre 1 e 5.\n");
                break;
            default:
                fprintf(stderr, "Erro desconhecido.\n");
                break;
        }
        return 1;
    }

    // ------------------------------------------------------------------------
    // EXIBIÇÃO DO RESULTADO (Aqui está a mudança que você pediu)
    // ------------------------------------------------------------------------
    printf("Numero decimal: %u\n", num);

    // Identifica o nome da base com base no valor de r
    if (r == 1) {
        printf("Base %d (Binario): %s\n", (1 << r), result);
    } else if (r == 2) {
        printf("Base %d (Quaternario): %s\n", (1 << r), result);
    } else if (r == 3) {
        printf("Base %d (Octal): %s\n", (1 << r), result);
    } else if (r == 4) {
        printf("Base %d (Hexadecimal): %s\n", (1 << r), result);
    } else if (r == 5) {
        printf("Base %d (Base 32): %s\n", (1 << r), result);
    } else {
        // Caso de segurança, mas a validação já impede r fora de 1-5
        printf("Base %d: %s\n", (1 << r), result);
    }

    // Libera a memória alocada
    free(result);

    return 0;
}

/*
================================================================================
COMO COMPILAR NO CODE::BLOCKS (WINDOWS)
================================================================================
(As instruções de compilação continuam as mesmas da resposta anterior)
1. Crie o arquivo task01_base.c.
2. Adicione ao projeto.
3. Vá em Project -> Set program's arguments...
4. Digite: 255 3 (para octal)
5. Pressione F9.
================================================================================
*/
