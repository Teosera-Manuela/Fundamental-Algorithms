/*
================================================================================
ENUNCIADO / ЗАДАНИЕ
================================================================================
[RUSSO]
5. Экземпляр структуры типа Student содержит поля: id, имя, фамилия,
группа и оценки за 5 экзаменов. Через аргументы командной строки подаётся
путь к файлу и путь к трассировочному файлу. Реализуйте поиск по id, фамилии,
имени, группе, сортировку по этим же полям (через qsort) и вывод в
трассировочный файл данных студента (по id) и студентов с баллом выше среднего.

[PORTUGUÊS]
5. A estrutura Student contém: id (int), nome, sobrenome, grupo e notas
de 5 exames (array dinâmico de unsigned char). Recebe via argumentos da linha
de comando o caminho de um arquivo de entrada e o caminho de um arquivo de trace.
- O programa lê o arquivo para um array dinâmico de Student.
- Implementa busca por: id, sobrenome, nome, grupo.
- Implementa ordenação por: id, sobrenome, nome, grupo (usando qsort e
  passando funções comparadoras).
- Escreve no arquivo de trace: dados do aluno buscado por id (Nome, grupo,
  média) e nome/sobrenome de alunos com média acima da média geral da turma.
- Menu interativo para o usuário.

RESTRIÇÕES: Sem <string.h>, sem variáveis globais, alocação dinâmica,
tratamento de erros com enum, separação de lógica e I/O.
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// DEFINIÇÃO DE STATUS CODES
// ============================================================================
typedef enum {
    STATUS_OK = 0,
    STATUS_ERR_MEMORY,
    STATUS_ERR_NULL_PTR,
    STATUS_ERR_FILE_OPEN,
    STATUS_ERR_INVALID_DATA
} StatusCode;

// ============================================================================
// ESTRUTURA STUDENT
// ============================================================================
typedef struct {
    unsigned int id;
    char name[50];
    char surname[50];
    char group[10];
    unsigned char* grades; // Array dinâmico de 5 notas
} Student;

// ============================================================================
// FUNÇÕES AUXILIARES (Substituindo <string.h>)
// ============================================================================

static int my_strlen(const char* str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

static void my_strcpy(char* dest, const char* src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

static int my_strcmp(const char* s1, const char* s2) {
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] < s2[i]) return -1;
        if (s1[i] > s2[i]) return 1;
        i++;
    }
    if (s1[i] == '\0' && s2[i] == '\0') return 0;
    if (s1[i] == '\0') return -1;
    return 1;
}

// Busca substring (para busca parcial de nomes)
static int my_strstr(const char* haystack, const char* needle) {
    if (*needle == '\0') return 1;
    for (int i = 0; haystack[i] != '\0'; i++) {
        int j = 0;
        while (haystack[i + j] == needle[j] && needle[j] != '\0') {
            j++;
        }
        if (needle[j] == '\0') return 1; // Encontrou
    }
    return 0;
}

// ============================================================================
// FUNÇÕES DE LÓGICA E LEITURA
// ============================================================================

// Calcula a média de um aluno
static double calculate_student_average(const Student* s) {
    if (s == NULL || s->grades == NULL) return 0.0;
    double sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += s->grades[i];
    }
    return sum / 5.0;
}

// Lê os dados do arquivo para o array dinâmico
StatusCode load_students(const char* filename, Student** out_students, int* out_count) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) return STATUS_ERR_FILE_OPEN;

    Student* students = NULL;
    int count = 0;
    int capacity = 10;
    students = (Student*)malloc(capacity * sizeof(Student));
    if (students == NULL) {
        fclose(file);
        return STATUS_ERR_MEMORY;
    }

    // Variáveis temporárias para leitura
    unsigned int temp_id;
    char temp_name[50], temp_surname[50], temp_group[10];
    int g1, g2, g3, g4, g5;

    while (fscanf(file, "%u %49s %49s %9s %d %d %d %d %d",
                  &temp_id, temp_name, temp_surname, temp_group,
                  &g1, &g2, &g3, &g4, &g5) == 9) {

        // Validação básica das notas
        if (g1 < 0 || g1 > 255 || g2 < 0 || g2 > 255 || g3 < 0 || g3 > 255 ||
            g4 < 0 || g4 > 255 || g5 < 0 || g5 > 255) {
            free(students);
            fclose(file);
            return STATUS_ERR_INVALID_DATA;
        }

        if (count == capacity) {
            capacity *= 2;
            Student* temp = (Student*)realloc(students, capacity * sizeof(Student));
            if (temp == NULL) {
                free(students);
                fclose(file);
                return STATUS_ERR_MEMORY;
            }
            students = temp;
        }

        students[count].id = temp_id;
        my_strcpy(students[count].name, temp_name);
        my_strcpy(students[count].surname, temp_surname);
        my_strcpy(students[count].group, temp_group);

        // Aloca dinamicamente as 5 notas
        students[count].grades = (unsigned char*)malloc(5 * sizeof(unsigned char));
        if (students[count].grades == NULL) {
            // Libera o que já foi alocado em caso de erro
            for (int i = 0; i < count; i++) free(students[i].grades);
            free(students);
            fclose(file);
            return STATUS_ERR_MEMORY;
        }

        students[count].grades[0] = (unsigned char)g1;
        students[count].grades[1] = (unsigned char)g2;
        students[count].grades[2] = (unsigned char)g3;
        students[count].grades[3] = (unsigned char)g4;
        students[count].grades[4] = (unsigned char)g5;

        count++;
    }

    fclose(file);
    *out_students = students;
    *out_count = count;
    return STATUS_OK;
}

// Libera a memória dos alunos
void free_students(Student* students, int count) {
    if (students == NULL) return;
    for (int i = 0; i < count; i++) {
        free(students[i].grades);
    }
    free(students);
}

// ============================================================================
// FUNÇÕES DE COMPARAÇÃO PARA QSORT
// ============================================================================
static int cmp_by_id(const void* a, const void* b) {
    const Student* s1 = (const Student*)a;
    const Student* s2 = (const Student*)b;
    if (s1->id < s2->id) return -1;
    if (s1->id > s2->id) return 1;
    return 0;
}

static int cmp_by_surname(const void* a, const void* b) {
    const Student* s1 = (const Student*)a;
    const Student* s2 = (const Student*)b;
    return my_strcmp(s1->surname, s2->surname);
}

static int cmp_by_name(const void* a, const void* b) {
    const Student* s1 = (const Student*)a;
    const Student* s2 = (const Student*)b;
    return my_strcmp(s1->name, s2->name);
}

static int cmp_by_group(const void* a, const void* b) {
    const Student* s1 = (const Student*)a;
    const Student* s2 = (const Student*)b;
    return my_strcmp(s1->group, s2->group);
}

// ============================================================================
// FUNÇÃO MAIN (MENU INTERATIVO)
// ============================================================================
int main(int argc, char* argv[]) {
    // 1. Validação de argumentos
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <arquivo_entrada> <arquivo_trace>\n", argv[0]);
        return 1;
    }

    const char* input_file = argv[1];
    const char* trace_file = argv[2];

    // 2. Carregar dados
    Student* students = NULL;
    int count = 0;
    StatusCode status = load_students(input_file, &students, &count);

    if (status != STATUS_OK) {
        switch (status) {
            case STATUS_ERR_FILE_OPEN: fprintf(stderr, "Erro: Nao foi possivel abrir '%s'.\n", input_file); break;
            case STATUS_ERR_MEMORY: fprintf(stderr, "Erro: Falha de alocacao de memoria.\n"); break;
            case STATUS_ERR_INVALID_DATA: fprintf(stderr, "Erro: Dados invalidos no arquivo.\n"); break;
            default: fprintf(stderr, "Erro desconhecido.\n"); break;
        }
        return 1;
    }

    printf("Carregados %d alunos do arquivo.\n", count);

    int option = -1;
    while (option != 0) {
        printf("\n=== SISTEMA DE ALUNOS ===");
        printf("\n1. Buscar por ID e salvar no Trace");
        printf("\n2. Listar Alunos Acima da Media Geral (salvar no Trace)");
        printf("\n3. Ordenar e Listar por ID");
        printf("\n4. Ordenar e Listar por Sobrenome");
        printf("\n5. Buscar por Nome (Console)");
        printf("\n6. Buscar por Grupo (Console)");
        printf("\n0. Sair");
        printf("\nEscolha: ");
        scanf("%d", &option);

        if (option == 1) {
            unsigned int search_id;
            printf("Digite o ID: ");
            scanf("%u", &search_id);

            FILE* trace = fopen(trace_file, "a"); // 'a' para adicionar ao final
            if (trace == NULL) {
                fprintf(stderr, "Erro ao abrir arquivo de trace.\n");
                continue;
            }

            int found = 0;
            for (int i = 0; i < count; i++) {
                if (students[i].id == search_id) {
                    double avg = calculate_student_average(&students[i]);
                    fprintf(trace, "Aluno ID %u: %s %s, Grupo: %s, Media: %.2f\n",
                            students[i].id, students[i].name, students[i].surname,
                            students[i].group, avg);
                    printf("Encontrado e salvo no trace.\n");
                    found = 1;
                    break;
                }
            }
            if (!found) printf("Aluno nao encontrado.\n");
            fclose(trace);
        }
        else if (option == 2) {
            // Calcular média geral da turma
            double total_avg = 0;
            for (int i = 0; i < count; i++) {
                total_avg += calculate_student_average(&students[i]);
            }
            total_avg /= count;

            FILE* trace = fopen(trace_file, "a");
            if (trace == NULL) {
                fprintf(stderr, "Erro ao abrir arquivo de trace.\n");
                continue;
            }

            fprintf(trace, "\n--- Alunos com media acima da media geral (%.2f) ---\n", total_avg);
            for (int i = 0; i < count; i++) {
                double avg = calculate_student_average(&students[i]);
                if (avg > total_avg) {
                    fprintf(trace, "%s %s (Media: %.2f)\n", students[i].name, students[i].surname, avg);
                    printf("Salvo no trace: %s %s\n", students[i].name, students[i].surname);
                }
            }
            fclose(trace);
            printf("Processo concluido. Verifique o arquivo de trace.\n");
        }
        else if (option == 3) {
            qsort(students, count, sizeof(Student), cmp_by_id);
            printf("\n--- Alunos por ID ---\n");
            for (int i = 0; i < count; i++) {
                printf("ID: %u | %s %s | Grupo: %s\n", students[i].id, students[i].name, students[i].surname, students[i].group);
            }
        }
        else if (option == 4) {
            qsort(students, count, sizeof(Student), cmp_by_surname);
            printf("\n--- Alunos por Sobrenome ---\n");
            for (int i = 0; i < count; i++) {
                printf("%s, %s | ID: %u | Grupo: %s\n", students[i].surname, students[i].name, students[i].id, students[i].group);
            }
        }
        else if (option == 5) {
            char search_name[50];
            printf("Digite parte do Nome: ");
            scanf("%49s", search_name);
            printf("\n--- Resultados da Busca ---\n");
            int found = 0;
            for (int i = 0; i < count; i++) {
                if (my_strstr(students[i].name, search_name)) {
                    printf("ID: %u | %s %s | Grupo: %s\n", students[i].id, students[i].name, students[i].surname, students[i].group);
                    found = 1;
                }
            }
            if (!found) printf("Nenhum aluno encontrado.\n");
        }
        else if (option == 6) {
            char search_group[10];
            printf("Digite o Grupo: ");
            scanf("%9s", search_group);
            printf("\n--- Resultados da Busca ---\n");
            int found = 0;
            for (int i = 0; i < count; i++) {
                if (my_strcmp(students[i].group, search_group) == 0) {
                    printf("ID: %u | %s %s | Grupo: %s\n", students[i].id, students[i].name, students[i].surname, students[i].group);
                    found = 1;
                }
            }
            if (!found) printf("Nenhum aluno encontrado.\n");
        }
    }

    free_students(students, count);
    printf("Saindo do sistema...\n");
    return 0;
}
