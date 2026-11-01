/*
================================================================================
ENUNCIADO / ЗАДАНИЕ
================================================================================
[RUSSO]
1. Через аргументы командной строки программе подаются флаг (первым
аргументом), строка (вторым аргументом); и (только для флага -c) целое число
типа unsigned int и далее произвольное количество строк. Флаг определяет
действие:
- -l: подсчёт длины строки.
- -r: перевёрнутая строка.
- -u: каждый символ на нечётной позиции (позиция 0 - чётная) в верхний регистр.
- -n: символы цифр, затем буквы, затем остальные (в исходном порядке).
- -c: конкатенация 2-й, 4-й, 5-й и т.д. строк в псевдослучайном порядке (seed).

[PORTUGUÊS]
1. Pela linha de comando, recebe-se uma flag, uma string, e (apenas para -c)
um unsigned int e mais strings. A flag define a ação:
- -l: Conta o comprimento da string.
- -r: Inverte a string.
- -u: Converte caracteres em posições ímpares (índice 0 é par) para maiúsculas.
- -n: Reorganiza a string: dígitos primeiro, depois letras, depois o resto (ordem original).
- -c: Concatena a 2ª, 4ª, 5ª... strings passadas, em ordem pseudoaleatória
      usando srand(seed).

RESTRIÇÕES: Sem <string.h>, sem variáveis globais, alocação dinâmica,
tratamento de erros com enum, separação de lógica e I/O.
================================================================================
*/

#include <stdio.h>  // Para funções de I/O (printf, fprintf)
#include <stdlib.h> // Para malloc, free, strtoul, rand, srand
#include <ctype.h>  // Para isdigit, isalpha, toupper

// ============================================================================
// DEFINIÇÃO DE STATUS CODES (Regra do laboratório)
// ============================================================================
typedef enum {
    STATUS_OK = 0,          // Operação bem-sucedida
    STATUS_ERR_MEMORY,      // Falha na alocação de memória
    STATUS_ERR_NULL_PTR,    // Ponteiro nulo passado por parâmetro
    STATUS_ERR_ARGS         // Argumentos de linha de comando inválidos
} StatusCode;

// ============================================================================
// FUNÇÕES AUXILIARES (Substituindo a biblioteca <string.h>)
// ============================================================================

// Retorna o comprimento da string (equivalente ao strlen)
static size_t my_strlen(const char* str) {
    size_t len = 0;
    // Percorre a string até encontrar o caractere nulo '\0'
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Copia a string (equivalente ao strcpy)
static void my_strcpy(char* dest, const char* src) {
    size_t i = 0;
    // Copia caractere por caractere
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0'; // Adiciona o terminador nulo no final
}

// ============================================================================
// FUNÇÕES LÓGICAS (Uma para cada flag)
// ============================================================================

// Flag -l: Apenas conta o comprimento da string
StatusCode process_flag_l(const char* str, size_t* out_len) {
    // Validação de ponteiros
    if (str == NULL || out_len == NULL) return STATUS_ERR_NULL_PTR;

    *out_len = my_strlen(str); // Armazena o resultado via ponteiro
    return STATUS_OK;
}

// Flag -r: Retorna uma nova string invertida
StatusCode process_flag_r(const char* str, char** out_str) {
    if (str == NULL || out_str == NULL) return STATUS_ERR_NULL_PTR;

    size_t len = my_strlen(str);
    // Aloca memória para a nova string (len + 1 para o terminador '\0')
    char* rev = (char*)malloc((len + 1) * sizeof(char));
    if (rev == NULL) return STATUS_ERR_MEMORY; // Falha de alocação

    // Copia os caracteres de trás para frente
    for (size_t i = 0; i < len; i++) {
        rev[i] = str[len - 1 - i];
    }
    rev[len] = '\0'; // Finaliza a string

    *out_str = rev; // Retorna o ponteiro alocado
    return STATUS_OK;
}

// Flag -u: Converte caracteres em posições ímpares para maiúsculas
StatusCode process_flag_u(const char* str, char** out_str) {
    if (str == NULL || out_str == NULL) return STATUS_ERR_NULL_PTR;

    size_t len = my_strlen(str);
    char* u_str = (char*)malloc((len + 1) * sizeof(char));
    if (u_str == NULL) return STATUS_ERR_MEMORY;

    my_strcpy(u_str, str); // Copia a string original primeiro

    // Percorre a string. Índices ímpares: 1, 3, 5...
    for (size_t i = 0; i < len; i++) {
        if (i % 2 != 0) { // Se for índice ímpar
            // Converte para maiúscula (cast para unsigned char evita problemas com acentos)
            u_str[i] = toupper((unsigned char)u_str[i]);
        }
    }

    *out_str = u_str;
    return STATUS_OK;
}

// Flag -n: Reorganiza (dígitos -> letras -> outros)
StatusCode process_flag_n(const char* str, char** out_str) {
    if (str == NULL || out_str == NULL) return STATUS_ERR_NULL_PTR;

    size_t len = my_strlen(str);
    char* n_str = (char*)malloc((len + 1) * sizeof(char));
    if (n_str == NULL) return STATUS_ERR_MEMORY;

    size_t index = 0;

    // 1. Copia todos os dígitos primeiro
    for (size_t i = 0; i < len; i++) {
        if (isdigit((unsigned char)str[i])) {
            n_str[index++] = str[i];
        }
    }
    // 2. Copia todas as letras depois
    for (size_t i = 0; i < len; i++) {
        if (isalpha((unsigned char)str[i])) {
            n_str[index++] = str[i];
        }
    }
    // 3. Copia o restante (nem dígito, nem letra)
    for (size_t i = 0; i < len; i++) {
        if (!isdigit((unsigned char)str[i]) && !isalpha((unsigned char)str[i])) {
            n_str[index++] = str[i];
        }
    }
    n_str[len] = '\0'; // Terminador nulo

    *out_str = n_str;
    return STATUS_OK;
}

// Flag -c: Concatenação pseudoaleatória
StatusCode process_flag_c(unsigned int seed, int count, char** strings, char** out_str) {
    if (strings == NULL || out_str == NULL) return STATUS_ERR_NULL_PTR;

    if (count <= 0) {
        // Se não houver strings, retorna uma string vazia válida
        char* empty = (char*)malloc(1);
        if (!empty) return STATUS_ERR_MEMORY;
        empty[0] = '\0';
        *out_str = empty;
        return STATUS_OK;
    }

    // 1. Calcula o tamanho total necessário para a string final
    size_t total_len = 0;
    for (int i = 0; i < count; i++) {
        total_len += my_strlen(strings[i]);
    }

    // 2. Aloca memória para o resultado
    char* result = (char*)malloc((total_len + 1) * sizeof(char));
    if (result == NULL) return STATUS_ERR_MEMORY;

    // 3. Cria um array de índices para embaralhar
    int* indices = (int*)malloc(count * sizeof(int));
    if (indices == NULL) {
        free(result); // Libera o que já foi alocado em caso de erro
        return STATUS_ERR_MEMORY;
    }

    for (int i = 0; i < count; i++) {
        indices[i] = i; // Preenche [0, 1, 2, ...]
    }

    // 4. Embaralha os índices usando a seed fornecida (Algoritmo Fisher-Yates)
    srand(seed);
    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    // 5. Concatena as strings na nova ordem pseudoaleatória
    size_t current_pos = 0;
    for (int i = 0; i < count; i++) {
        int idx = indices[i];
        size_t str_len = my_strlen(strings[idx]);
        for (size_t k = 0; k < str_len; k++) {
            result[current_pos++] = strings[idx][k];
        }
    }
    result[current_pos] = '\0'; // Finaliza a string

    free(indices); // Libera o array temporário
    *out_str = result;
    return STATUS_OK;
}

// ============================================================================
// FUNÇÃO MAIN (Roteamento de argumentos e I/O)
// ============================================================================
int main(int argc, char* argv[]) {

    // Validação básica: Precisa de pelo menos o programa + flag + string
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <flag> <string> [seed] [strings...]\n", argv[0]);
        return 1;
    }

    const char* flag = argv[1];
    const char* input_str = argv[2];
    StatusCode status = STATUS_OK;

    // Roteamento das flags (Lógica do programa)
    if (flag[0] == '-' && flag[1] == 'l' && flag[2] == '\0') {
        size_t len = 0;
        status = process_flag_l(input_str, &len);
        if (status == STATUS_OK) {
            printf("Comprimento: %zu\n", len);
        }
    }
    else if (flag[0] == '-' && flag[1] == 'r' && flag[2] == '\0') {
        char* res = NULL;
        status = process_flag_r(input_str, &res);
        if (status == STATUS_OK) {
            printf("Invertida: %s\n", res);
            free(res); // Libera a memória alocada pela função
        }
    }
    else if (flag[0] == '-' && flag[1] == 'u' && flag[2] == '\0') {
        char* res = NULL;
        status = process_flag_u(input_str, &res);
        if (status == STATUS_OK) {
            printf("Maiusculas impares: %s\n", res);
            free(res);
        }
    }
    else if (flag[0] == '-' && flag[1] == 'n' && flag[2] == '\0') {
        char* res = NULL;
        status = process_flag_n(input_str, &res);
        if (status == STATUS_OK) {
            printf("Reordenada: %s\n", res);
            free(res);
        }
    }
    else if (flag[0] == '-' && flag[1] == 'c' && flag[2] == '\0') {
        // Para -c, precisamos validar os argumentos extras
        if (argc < 5) {
            fprintf(stderr, "Erro: Flag -c requer <seed> e pelo menos 2 strings.\n");
            return 1;
        }

        // Converte o 3º argumento (seed) para unsigned int
        unsigned int seed = (unsigned int)strtoul(argv[2], NULL, 10);

        // O exercício pede a partir da 2ª string.
        // O argv[3] é a 1ª string. O argv[4] é a 2ª. Então começamos do índice 4.
        int num_valid_strings = argc - 4;

        // Aloca um array temporário de ponteiros para passar para a função
        char** valid_strings = (char**)malloc(num_valid_strings * sizeof(char*));
        if (valid_strings == NULL) {
            fprintf(stderr, "Erro de memoria.\n");
            return 1;
        }

        // Preenche o array com os ponteiros das strings válidas
        for (int i = 0; i < num_valid_strings; i++) {
            valid_strings[i] = argv[i + 4];
        }

        char* res = NULL;
        status = process_flag_c(seed, num_valid_strings, valid_strings, &res);

        if (status == STATUS_OK) {
            printf("Concatenacao pseudoaleatoria: %s\n", res);
            free(res);
        }
        free(valid_strings); // Libera o array temporário
    }
    else {
        fprintf(stderr, "Flag desconhecida: %s\n", flag);
        return 1;
    }

    // Tratamento de erros baseado no status retornado
    if (status != STATUS_OK) {
        switch (status) {
            case STATUS_ERR_MEMORY:
                fprintf(stderr, "Erro: Falha de alocacao de memoria.\n");
                break;
            case STATUS_ERR_NULL_PTR:
                fprintf(stderr, "Erro: Ponteiro nulo.\n");
                break;
            case STATUS_ERR_ARGS:
                fprintf(stderr, "Erro: Argumentos invalidos.\n");
                break;
            default:
                fprintf(stderr, "Erro desconhecido.\n");
                break;
        }
        return 1;
    }

    return 0;
}
