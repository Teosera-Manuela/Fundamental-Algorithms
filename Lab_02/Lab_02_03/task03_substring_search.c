/*
================================================================================
ENUNCIADO / ЗАДАНИЕ
================================================================================
[RUSSO]
3. Реализуйте функцию с переменным числом аргументов, принимающую в качестве
входных параметров подстроку, количество файлов и пути к этим файлам.
Необходимо чтобы для каждого файла функция производила поиск всех вхождений
переданной подстроки в содержимом этого файла. При реализации запрещается
использование функций strstr и strncmp из заголовочного файла string.h.

[PORTUGUÊS]
3. Implemente uma função com número variável de argumentos que recebe uma
substring, a quantidade de arquivos e os caminhos desses arquivos. Para cada
arquivo, a função deve buscar todas as ocorrências da substring no conteúdo.
É proibido usar strstr e strncmp da biblioteca string.h.
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Permitido, exceto strstr e strncmp
#include <stdarg.h> // Necessário para va_list

// ============================================================================
// DEFINIÇÃO DE STATUS CODES
// ============================================================================
typedef enum {
    STATUS_OK = 0,
    STATUS_ERR_ARGS,
    STATUS_ERR_MEMORY,
    STATUS_ERR_FILE_OPEN
} StatusCode;

// ============================================================================
// FUNÇÃO AUXILIAR: Busca substring sem strstr/strncmp
// ============================================================================
// Retorna o número de ocorrências da substring na string de texto
static int count_substring(const char* text, const char* sub) {
    if (text == NULL || sub == NULL) return 0;

    size_t text_len = strlen(text);
    size_t sub_len = strlen(sub);

    // Se a substring for maior que o texto ou vazia, não há como achar
    if (sub_len == 0 || text_len < sub_len) return 0;

    int count = 0;

    // Percorre o texto. O limite é (text_len - sub_len) para não estourar a memória
    for (size_t i = 0; i <= text_len - sub_len; i++) {
        int match = 1; // Assume que vai dar match

        // Compara caractere por caractere manualmente (substituindo strncmp)
        for (size_t j = 0; j < sub_len; j++) {
            if (text[i + j] != sub[j]) {
                match = 0; // Falhou, não é igual
                break;
            }
        }

        if (match) {
            count++; // Encontrou uma ocorrência
        }
    }
    return count;
}

// ============================================================================
// FUNÇÃO PRINCIPAL: Variádica
// ============================================================================
StatusCode search_in_files(const char* substring, int file_count, ...) {
    // Validação inicial
    if (substring == NULL || file_count <= 0) return STATUS_ERR_ARGS;

    va_list args;
    va_start(args, file_count); // Inicializa a lista variádica

    StatusCode overall_status = STATUS_OK;

    // Itera sobre a quantidade de arquivos passada
    for (int i = 0; i < file_count; i++) {
        const char* filepath = va_arg(args, const char*); // Pega o próximo caminho

        if (filepath == NULL) {
            fprintf(stderr, "Aviso: Caminho de arquivo nulo ignorado.\n");
            continue;
        }

        FILE* file = fopen(filepath, "r");
        if (file == NULL) {
            fprintf(stderr, "Erro: Nao foi possivel abrir o arquivo '%s'.\n", filepath);
            overall_status = STATUS_ERR_FILE_OPEN;
            continue; // Pula para o próximo arquivo
        }

        // Lógica para ler o arquivo inteiro:
        // Vamos descobrir o tamanho do arquivo
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (file_size <= 0) {
            printf("Arquivo '%s': vazio.\n", filepath);
            fclose(file);
            continue;
        }

        // Aloca memória para ler o conteúdo (+1 para o '\0')
        char* buffer = (char*)malloc((file_size + 1) * sizeof(char));
        if (buffer == NULL) {
            fclose(file);
            va_end(args);
            return STATUS_ERR_MEMORY;
        }

        // Lê o conteúdo
        size_t bytes_read = fread(buffer, 1, file_size, file);
        buffer[bytes_read] = '\0'; // Finaliza a string

        // Realiza a busca
        int occurrences = count_substring(buffer, substring);
        printf("Arquivo '%s': %d ocorrencia(s) encontrada(s).\n", filepath, occurrences);

        // Limpeza deste arquivo
        free(buffer);
        fclose(file);
    }

    va_end(args); // Finaliza a lista variádica
    return overall_status;
}

// ============================================================================
// FUNÇÃO MAIN (Demonstração)
// ============================================================================
int main(int argc, char* argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <substring> <qtd_arquivos> <arquivo1> [arquivo2...]\n", argv[0]);
        return 1;
    }

    const char* substring = argv[1];
    int file_count = atoi(argv[2]);

    // Validação da quantidade de argumentos passados na linha de comando
    // Precisamos de: prog + substring + file_count + file_count arquivos
    if (argc != 3 + file_count) {
        fprintf(stderr, "Erro: A quantidade de arquivos passada nao corresponde ao numero indicado.\n");
        return 1;
    }

    // O laboratório exige chamada da função variádica.
    // Como a quantidade de arquivos é dinâmica, precisamos chamar a função
    // de acordo com o número. Como C não permite passar um array para "...",
    // vamos usar um switch para os testes comuns.

    StatusCode status;
    if (file_count == 1) {
        status = search_in_files(substring, 1, argv[3]);
    } else if (file_count == 2) {
        status = search_in_files(substring, 2, argv[3], argv[4]);
    } else if (file_count == 3) {
        status = search_in_files(substring, 3, argv[3], argv[4], argv[5]);
    } else {
        fprintf(stderr, "Para esta demonstracao, o limite e de 3 arquivos.\n");
        return 1;
    }

    // Tratamento de erros
    if (status != STATUS_OK) {
        switch (status) {
            case STATUS_ERR_ARGS: fprintf(stderr, "Erro nos argumentos.\n"); break;
            case STATUS_ERR_MEMORY: fprintf(stderr, "Erro de memoria.\n"); break;
            case STATUS_ERR_FILE_OPEN: fprintf(stderr, "Alguns arquivos nao puderam ser abertos.\n"); break;
            default: fprintf(stderr, "Erro desconhecido.\n"); break;
        }
        return 1;
    }

    return 0;
}
