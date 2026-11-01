/*
================================================================================
ENUNCIADO / ЗАДАНИЕ
================================================================================
[RUSSO]
2. Реализуйте функцию, определяющую максимальное количество докладов...
[PORTUGUÊS]
2. Implemente uma função que determine o número máximo de palestras...
RESTRIÇÕES: Sem <string.h>, sem variáveis globais, alocação dinâmica,
tratamento de erros com enum, separação de lógica e I/O.
OBSERVAÇÕES: O programa agora lê os dados de um arquivo de texto.
================================================================================
*/

#include <stdio.h>  // Para printf, fprintf, fscanf, fopen, fclose
#include <stdlib.h> // Para malloc, free, qsort, strtol

// ============================================================================
// ESTRUTURAS E ENUMS
// ============================================================================
typedef enum {
    STATUS_OK = 0,
    STATUS_ERR_MEMORY,
    STATUS_ERR_NULL_PTR,
    STATUS_ERR_INVALID_ARG,
    STATUS_ERR_INVALID_TIME,
    STATUS_ERR_FILE_OPEN,
    STATUS_ERR_FILE_READ
} StatusCode;

typedef struct {
    unsigned int id;
    char start_time[6];  // "HH:MM\0"
    char finish_time[6]; // "HH:MM\0"
    int start_mins;
    int finish_mins;
} Lecture;

// ============================================================================
// FUNÇÕES AUXILIARES
// ============================================================================

static int time_to_mins(const char* time_str) {
    if (time_str == NULL) return -1;
    if (time_str[2] != ':') return -1;
    if (time_str[0] < '0' || time_str[0] > '2') return -1;
    if (time_str[1] < '0' || time_str[1] > '9') return -1;
    if (time_str[3] < '0' || time_str[3] > '5') return -1;
    if (time_str[4] < '0' || time_str[4] > '9') return -1;

    int h = (time_str[0] - '0') * 10 + (time_str[1] - '0');
    int m = (time_str[3] - '0') * 10 + (time_str[4] - '0');

    if (h > 23 || m > 59) return -1;
    return (h * 60) + m;
}

static int compare_lectures(const void* a, const void* b) {
    const Lecture* l1 = (const Lecture*)a;
    const Lecture* l2 = (const Lecture*)b;
    if (l1->finish_mins < l2->finish_mins) return -1;
    if (l1->finish_mins > l2->finish_mins) return 1;
    return 0;
}

// ============================================================================
// FUNÇÃO LÓGICA
// ============================================================================

StatusCode schedule_lectures(Lecture* lectures, int count, int num_rooms, int* out_max_count) {
    if (lectures == NULL || out_max_count == NULL) return STATUS_ERR_NULL_PTR;
    if (count < 0 || num_rooms <= 0) return STATUS_ERR_INVALID_ARG;

    if (count == 0) {
        *out_max_count = 0;
        return STATUS_OK;
    }

    for (int i = 0; i < count; i++) {
        lectures[i].start_mins = time_to_mins(lectures[i].start_time);
        lectures[i].finish_mins = time_to_mins(lectures[i].finish_time);
        if (lectures[i].start_mins == -1 || lectures[i].finish_mins == -1) return STATUS_ERR_INVALID_TIME;
        if (lectures[i].start_mins >= lectures[i].finish_mins) return STATUS_ERR_INVALID_TIME;
    }

    Lecture* sorted = (Lecture*)malloc((size_t)count * sizeof(Lecture));
    if (sorted == NULL) return STATUS_ERR_MEMORY;
    for (int i = 0; i < count; i++) sorted[i] = lectures[i];

    qsort(sorted, (size_t)count, sizeof(Lecture), compare_lectures);

    int* room_free_time = (int*)malloc((size_t)num_rooms * sizeof(int));
    if (room_free_time == NULL) {
        free(sorted);
        return STATUS_ERR_MEMORY;
    }
    for (int i = 0; i < num_rooms; i++) room_free_time[i] = 0;

    int scheduled_count = 0;
    for (int i = 0; i < count; i++) {
        for (int r = 0; r < num_rooms; r++) {
            if (room_free_time[r] <= sorted[i].start_mins) {
                room_free_time[r] = sorted[i].finish_mins;
                scheduled_count++;
                break;
            }
        }
    }

    free(room_free_time);
    free(sorted);
    *out_max_count = scheduled_count;
    return STATUS_OK;
}

// ============================================================================
// FUNÇÃO MAIN (I/O)
// ============================================================================
int main(int argc, char* argv[]) {
    // 1. Validação de argumentos (Programa + Num Salas + Arquivo)
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <numero_de_salas> <arquivo_de_entrada>\n", argv[0]);
        return 1;
    }

    int num_rooms = (int)strtol(argv[1], NULL, 10);
    if (num_rooms <= 0) {
        fprintf(stderr, "Erro: O numero de salas deve ser maior que zero.\n");
        return 1;
    }

    // 2. Abertura do arquivo
    FILE* file = fopen(argv[2], "r");
    if (file == NULL) {
        fprintf(stderr, "Erro: Nao foi possivel abrir o arquivo '%s'.\n", argv[2]);
        return 1;
    }

    // 3. Leitura dos dados para alocação dinâmica
    Lecture* lectures = NULL;
    int count = 0;
    int capacity = 10;
    lectures = (Lecture*)malloc((size_t)capacity * sizeof(Lecture));
    if (lectures == NULL) {
        fclose(file);
        fprintf(stderr, "Erro de memoria.\n");
        return 1;
    }

    // Lê ID, Hora Inicio e Hora Fim de cada linha
    while (fscanf(file, "%u %5s %5s",
                  &lectures[count].id,
                  lectures[count].start_time,
                  lectures[count].finish_time) == 3) {
        count++;
        if (count == capacity) {
            capacity *= 2;
            Lecture* temp = (Lecture*)realloc(lectures, (size_t)capacity * sizeof(Lecture));
            if (temp == NULL) {
                free(lectures);
                fclose(file);
                fprintf(stderr, "Erro de memoria.\n");
                return 1;
            }
            lectures = temp;
        }
    }

    if (ferror(file)) {
        fprintf(stderr, "Erro ao ler o arquivo.\n");
        free(lectures);
        fclose(file);
        return 1;
    }
    fclose(file); // Arquivo lido, podemos fechar

    // 4. Processamento
    printf("--- Simulacao de Agenda de Conferencias ---\n");
    printf("Salas disponiveis: %d\n", num_rooms);
    printf("Palestras lidas do arquivo: %d\n\n", count);

    int max_scheduled = 0;
    StatusCode status = schedule_lectures(lectures, count, num_rooms, &max_scheduled);

    // 5. Tratamento de erros
    if (status != STATUS_OK) {
        switch (status) {
            case STATUS_ERR_MEMORY: fprintf(stderr, "Erro: Memoria.\n"); break;
            case STATUS_ERR_NULL_PTR: fprintf(stderr, "Erro: Ponteiro nulo.\n"); break;
            case STATUS_ERR_INVALID_ARG: fprintf(stderr, "Erro: Argumentos.\n"); break;
            case STATUS_ERR_INVALID_TIME: fprintf(stderr, "Erro: Horario invalido no arquivo.\n"); break;
            default: fprintf(stderr, "Erro desconhecido.\n"); break;
        }
        free(lectures);
        return 1;
    }

    // 6. Exibição do resultado
    printf("Maximo de palestras agendadas com sucesso: %d\n", max_scheduled);

    // Liberação da memória
    free(lectures);
    return 0;
}

/*
================================================================================
COMO TESTAR NO CODE::BLOCKS
================================================================================
1. Crie um arquivo de texto chamado "palestras.txt" na MESMA PASTA onde está o
   seu arquivo .c (ou na pasta bin/Debug).
   O conteúdo deve ser:
   1 09:00 10:30
   2 09:30 11:00
   3 10:30 12:00
   4 11:00 12:30
   5 13:00 14:00

2. Vá em Project -> Set program's arguments...
3. Digite: 2 palestras.txt
4. Pressione F9.
================================================================================
*/
