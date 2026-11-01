/*
================================================================================
ENUNCIADO / ЗАДАНИЕ
================================================================================
[RUSSO]
2. На вход программе через аргументы командной строки подается путь ко
входному файлу, флаг (начинается с '-' или '/', второй символ - 'a' или 'd')
и путь к выходному файлу. В файле в каждой строчке: id (целое >= 0), имя
(только буквы), фамилия (только буквы), зарплата (вещ. >= 0).
Считать в динамический массив, отсортировать с помощью qsort (по возрастанию
'-a' или убыванию '-d'): сначала по зарплате, потом по фамилии, потом по
имени, потом по id. Вывести в выходной файл.

[PORTUGUÊS]
2. Recebe caminho de entrada, flag e caminho de saída. Lê registros de
funcionários (id, nome, sobrenome, salário). Guarda em array dinâmico e ordena
usando qsort. Ordenação: salário -> sobrenome -> nome -> id. A flag '-a' é
crescente e '-d' é decrescente. Salva no arquivo de saída.
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Permitido nesta tarefa (proibido apenas na Task 01)
#include <ctype.h>

// ============================================================================
// DEFINIÇÃO DE STATUS CODES
// ============================================================================
typedef enum {
    STATUS_OK = 0,
    STATUS_ERR_ARGS,
    STATUS_ERR_MEMORY,
    STATUS_ERR_FILE_OPEN,
    STATUS_ERR_INVALID_DATA
} StatusCode;

// ============================================================================
// ESTRUTURA DO FUNCIONÁRIO
// ============================================================================
typedef struct {
    int id;
    char name[64];     // Tamanho fixo para simplificar (ou poderia ser dinâmico)
    char surname[64];
    double salary;
} Employee;

// ============================================================================
// FUNÇÕES AUXILIARES DE VALIDAÇÃO
// ============================================================================

// Verifica se a string contém apenas letras latinas
static int is_valid_name(const char* str) {
    if (str == NULL || str[0] == '\0') return 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isalpha((unsigned char)str[i])) return 0;
    }
    return 1;
}

// ============================================================================
// FUNÇÕES DE COMPARAÇÃO PARA O QSORT
// ============================================================================
// Precisamos de duas funções: uma crescente e uma decrescente.
// Se precisarmos de decrescente, invertemos o retorno da crescente.

static int compare_employees_asc(const void* a, const void* b) {
    const Employee* emp1 = (const Employee*)a;
    const Employee* emp2 = (const Employee*)b;

    // 1º Critério: Salário
    if (emp1->salary < emp2->salary) return -1;
    if (emp1->salary > emp2->salary) return 1;

    // 2º Critério: Sobrenome (usando strcmp)
    int cmp_surname = strcmp(emp1->surname, emp2->surname);
    if (cmp_surname != 0) return cmp_surname;

    // 3º Critério: Nome
    int cmp_name = strcmp(emp1->name, emp2->name);
    if (cmp_name != 0) return cmp_name;

    // 4º Critério: ID
    if (emp1->id < emp2->id) return -1;
    if (emp1->id > emp2->id) return 1;

    return 0; // São iguais em tudo
}

static int compare_employees_desc(const void* a, const void* b) {
    // Para decrescente, basta inverter o resultado da função crescente
    return -compare_employees_asc(a, b);
}

// ============================================================================
// FUNÇÃO DE I/O: Ler o arquivo
// ============================================================================
StatusCode read_employees(const char* filename, Employee** out_arr, int* out_count) {
    if (filename == NULL || out_arr == NULL || out_count == NULL) return STATUS_ERR_ARGS;

    FILE* file = fopen(filename, "r");
    if (file == NULL) return STATUS_ERR_FILE_OPEN;

    int capacity = 10; // Capacidade inicial
    int count = 0;
    Employee* arr = (Employee*)malloc(capacity * sizeof(Employee));
    if (arr == NULL) {
        fclose(file);
        return STATUS_ERR_MEMORY;
    }

    int temp_id;
    char temp_name[64];
    char temp_surname[64];
    double temp_salary;

    // Lê os dados do arquivo. Assumimos que estão bem formatados, mas vamos validar.
    while (fscanf(file, "%d %63s %63s %lf", &temp_id, temp_name, temp_surname, &temp_salary) == 4) {

        // Validação dos dados lidos
        if (temp_id < 0 || temp_salary < 0.0 ||
            !is_valid_name(temp_name) || !is_valid_name(temp_surname)) {
            free(arr);
            fclose(file);
            return STATUS_ERR_INVALID_DATA;
        }

        // Se o array estiver cheio, aumenta a capacidade (realocação dinâmica)
        if (count == capacity) {
            capacity *= 2;
            Employee* temp_arr = (Employee*)realloc(arr, capacity * sizeof(Employee));
            if (temp_arr == NULL) {
                free(arr);
                fclose(file);
                return STATUS_ERR_MEMORY;
            }
            arr = temp_arr;
        }

        // Armazena os dados no array
        arr[count].id = temp_id;
        strcpy(arr[count].name, temp_name);
        strcpy(arr[count].surname, temp_surname);
        arr[count].salary = temp_salary;
        count++;
    }

    fclose(file);
    *out_arr = arr;
    *out_count = count;
    return STATUS_OK;
}

// ============================================================================
// FUNÇÃO DE I/O: Escrever o arquivo
// ============================================================================
StatusCode write_employees(const char* filename, const Employee* arr, int count) {
    if (filename == NULL || arr == NULL) return STATUS_ERR_ARGS;

    FILE* file = fopen(filename, "w");
    if (file == NULL) return STATUS_ERR_FILE_OPEN;

    for (int i = 0; i < count; i++) {
        // Formato: id nome sobrenome salario
        fprintf(file, "%d %s %s %.2f\n", arr[i].id, arr[i].name, arr[i].surname, arr[i].salary);
    }

    fclose(file);
    return STATUS_OK;
}

// ============================================================================
// FUNÇÃO MAIN
// ============================================================================
int main(int argc, char* argv[]) {
    // prog caminho_entrada -a caminho_saida (argc == 4)
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <arquivo_entrada> <-a|-d> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    const char* input_path = argv[1];
    const char* flag = argv[2];
    const char* output_path = argv[3];

    // Validação da flag (deve ter 2 caracteres: '-' ou '/' seguido de 'a' ou 'd')
    if ((flag[0] != '-' && flag[0] != '/') ||
        (flag[1] != 'a' && flag[1] != 'd') ||
        flag[2] != '\0') {
        fprintf(stderr, "Erro: Flag invalida. Use -a, -d, /a ou /d.\n");
        return 1;
    }

    int is_descending = (flag[1] == 'd');
    Employee* employees = NULL;
    int count = 0;

    // 1. Ler os dados
    StatusCode status = read_employees(input_path, &employees, &count);

    if (status != STATUS_OK) {
        switch (status) {
            case STATUS_ERR_FILE_OPEN: fprintf(stderr, "Erro ao abrir arquivo de entrada.\n"); break;
            case STATUS_ERR_MEMORY: fprintf(stderr, "Erro de alocacao de memoria.\n"); break;
            case STATUS_ERR_INVALID_DATA: fprintf(stderr, "Erro: Dados invalidos no arquivo.\n"); break;
            default: fprintf(stderr, "Erro desconhecido ao ler arquivo.\n"); break;
        }
        return 1;
    }

    // 2. Ordenar usando qsort
    if (count > 0) {
        if (is_descending) {
            qsort(employees, count, sizeof(Employee), compare_employees_desc);
        } else {
            qsort(employees, count, sizeof(Employee), compare_employees_asc);
        }
    }

    // 3. Escrever no arquivo de saída
    status = write_employees(output_path, employees, count);

    if (status != STATUS_OK) {
        if (status == STATUS_ERR_FILE_OPEN) fprintf(stderr, "Erro ao abrir arquivo de saida.\n");
        else fprintf(stderr, "Erro ao escrever no arquivo.\n");
        free(employees); // Importante liberar a memória antes de sair com erro
        return 1;
    }

    // Limpeza final
    free(employees);
    printf("Ordenacao concluida com sucesso! Verifique o arquivo: %s\n", output_path);

    return 0;
}
