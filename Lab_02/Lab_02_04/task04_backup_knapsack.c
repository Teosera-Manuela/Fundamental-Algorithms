/*
================================================================================
ENUNCIADO / ЗАДАНИЕ
================================================================================
[RUSSO]
4. Реализуйте функцию выбора файлов для резервного копирования на накопитель
ограниченной вместимости. На сервере хранится набор файлов; для каждого
файла известны его имя, размер и важность. Входные параметры: массив описаний
файлов, количество элементов и вместимость. Функция определяет набор файлов,
суммарный размер которых не превышает вместимость, а суммарная важность
максимально возможна (задача о рюкзаке 0/1). Результат - структура с массивом
выбранных файлов, их суммарным размером и важностью.
Формат ввода: n capacity, затем n строк "filename size importance".
Формат вывода: TOTAL_SIZE S TOTAL_IMPORTANCE I FILES k, затем k строк с именами.

[PORTUGUÊS]
4. Implemente uma função para selecionar arquivos para backup (mochila 0/1).
Recebe um array de arquivos (nome, tamanho, importância), a quantidade e a
capacidade. Retorna uma estrutura com os arquivos escolhidos, tamanho total e
importância total. Lê da entrada padrão e imprime no formato especificado.
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// DEFINIÇÃO DE STATUS CODES E ESTRUTURAS
// ============================================================================
typedef enum {
    STATUS_OK = 0,
    STATUS_ERR_MEMORY,
    STATUS_ERR_NULL_PTR,
    STATUS_ERR_INVALID_DATA
} StatusCode;

// Estrutura de um arquivo individual
typedef struct {
    char name[64];
    int size;
    int importance;
} FileItem;

// Estrutura de retorno do resultado do backup
typedef struct {
    FileItem* selected_files;
    int count;
    int total_size;
    int total_importance;
} BackupResult;

// ============================================================================
// FUNÇÃO LÓGICA PRINCIPAL (Algoritmo da Mochila 0/1)
// ============================================================================
// Usamos Programação Dinâmica para resolver o problema da mochila.
// A tabela DP guarda a máxima importância possível para cada capacidade.
// ============================================================================
StatusCode find_best_backup(const FileItem* files, int n, int capacity, BackupResult* result) {
    if (files == NULL || result == NULL) return STATUS_ERR_NULL_PTR;
    if (capacity < 0 || n < 0) return STATUS_ERR_INVALID_DATA;

    result->selected_files = NULL;
    result->count = 0;
    result->total_size = 0;
    result->total_importance = 0;

    if (n == 0 || capacity == 0) return STATUS_OK; // Nada a fazer

    // Aloca a tabela DP: (n + 1) linhas por (capacity + 1) colunas
    // dp[i][w] = máxima importância usando os primeiros i itens com capacidade w
    int** dp = (int**)malloc((n + 1) * sizeof(int*));
    if (dp == NULL) return STATUS_ERR_MEMORY;

    for (int i = 0; i <= n; i++) {
        dp[i] = (int*)calloc((capacity + 1), sizeof(int)); // calloc preenche com 0
        if (dp[i] == NULL) {
            // Em caso de erro, libera o que já foi alocado
            for (int k = 0; k < i; k++) free(dp[k]);
            free(dp);
            return STATUS_ERR_MEMORY;
        }
    }

    // Preenchendo a tabela DP
    for (int i = 1; i <= n; i++) {
        int current_size = files[i - 1].size;
        int current_imp = files[i - 1].importance;

        for (int w = 0; w <= capacity; w++) {
            if (current_size > w) {
                // Se o arquivo é maior que a capacidade atual, não podemos pegá-lo
                dp[i][w] = dp[i - 1][w];
            } else {
                // Máximo entre NÃO pegar o arquivo ou PEGAR o arquivo
                int dont_take = dp[i - 1][w];
                int take = current_imp + dp[i - 1][w - current_size];
                dp[i][w] = (take > dont_take) ? take : dont_take;
            }
        }
    }

    // O valor máximo estará em dp[n][capacity]
    result->total_importance = dp[n][capacity];

    // Agora precisamos reconstruir quais itens foram escolhidos (Backtracking)
    // Alocamos memória para o array de escolhidos (no pior caso, todos)
    result->selected_files = (FileItem*)malloc(n * sizeof(FileItem));
    if (result->selected_files == NULL) {
        for (int i = 0; i <= n; i++) free(dp[i]);
        free(dp);
        return STATUS_ERR_MEMORY;
    }

    int w = capacity;
    for (int i = n; i > 0 && result->total_importance > 0; i--) {
        // Se o valor mudou da linha de cima para a linha atual, o item foi incluído
        if (dp[i][w] != dp[i - 1][w]) {
            // Armazena o item. (Guardamos de trás pra frente temporariamente)
            result->selected_files[result->count++] = files[i - 1];
            w -= files[i - 1].size;
            result->total_size += files[i - 1].size;
        }
    }

    // Como fizemos backtracking de trás pra frente, precisamos inverter o array
    // para manter a ordem original em que apareceram na entrada.
    for (int i = 0; i < result->count / 2; i++) {
        FileItem temp = result->selected_files[i];
        result->selected_files[i] = result->selected_files[result->count - 1 - i];
        result->selected_files[result->count - 1 - i] = temp;
    }

    // Limpeza da tabela DP
    for (int i = 0; i <= n; i++) free(dp[i]);
    free(dp);

    return STATUS_OK;
}

// ============================================================================
// FUNÇÃO MAIN (I/O e Demonstração)
// ============================================================================
int main(void) {
    int n, capacity;

    // 1. Leitura dos dados do usuário (I/O)
    printf("Digite n e capacity: ");
    if (scanf("%d %d", &n, &capacity) != 2 || n < 0 || capacity < 0) {
        fprintf(stderr, "Erro: Entrada invalida para n ou capacity.\n");
        return 1;
    }

    // Aloca o array de arquivos
    FileItem* files = (FileItem*)malloc(n * sizeof(FileItem));
    if (files == NULL) {
        fprintf(stderr, "Erro de memoria.\n");
        return 1;
    }

    printf("Digite os %d arquivos (nome tamanho importancia):\n", n);
    for (int i = 0; i < n; i++) {
        // O %63s evita buffer overflow
        if (scanf("%63s %d %d", files[i].name, &files[i].size, &files[i].importance) != 3) {
            fprintf(stderr, "Erro na leitura do arquivo %d.\n", i + 1);
            free(files);
            return 1;
        }
        // Validação dos dados
        if (files[i].size <= 0 || files[i].importance < 0) {
            fprintf(stderr, "Erro: tamanho deve ser > 0 e importancia >= 0.\n");
            free(files);
            return 1;
        }
    }

    // 2. Chamada da função lógica
    BackupResult result;
    StatusCode status = find_best_backup(files, n, capacity, &result);

    // 3. Tratamento do resultado (I/O)
    if (status == STATUS_OK) {
        // Formato de saída exigido pelo laboratório
        printf("TOTAL_SIZE %d TOTAL_IMPORTANCE %d FILES %d\n",
               result.total_size, result.total_importance, result.count);

        // Imprime os nomes dos arquivos escolhidos
        for (int i = 0; i < result.count; i++) {
            printf("%s\n", result.selected_files[i].name);
        }
    } else {
        switch (status) {
            case STATUS_ERR_MEMORY: fprintf(stderr, "Erro: Memoria insuficiente.\n"); break;
            case STATUS_ERR_INVALID_DATA: fprintf(stderr, "Erro: Dados invalidos.\n"); break;
            default: fprintf(stderr, "Erro desconhecido.\n"); break;
        }
        free(files);
        return 1;
    }

    // Liberação de toda a memória alocada
    free(result.selected_files);
    free(files);

    return 0;
}
