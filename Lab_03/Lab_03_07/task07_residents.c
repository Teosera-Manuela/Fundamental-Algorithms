/*
================================================================================
ENUNCIADO / ЗАДАНИЕ
================================================================================
[RUSSO]
7. В текстовом файле находится информация о жителях (тип структуры Liver)...
Напишите программу, которая считывает эту информацию из файла в односвязный
упорядоченный список (в порядке увеличения возраста). Реализуйте возможности
поиска, изменения, удаления/добавления и выгрузки данных в файл. Добавьте
возможность отменить последние [N/2] введённых модификаций (аналог Undo).

[PORTUGUÊS]
7. Em um arquivo de texto há informações sobre os moradores (struct Liver):
sobrenome, nome, patronímico, data de nascimento, sexo ('M'/'W'), renda média.
Leia para uma lista simplesmente ligada ordenada (por idade). Implemente busca,
modificação, remoção/adição e exportação para arquivo. Adicione a possibilidade
de desfazer as últimas [N/2] modificações (Undo).

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
    STATUS_ERR_NOT_FOUND,
    STATUS_ERR_INVALID_DATA
} StatusCode;

// ============================================================================
// ESTRUTURAS DE DADOS
// ============================================================================

typedef struct {
    char surname[50];
    char name[50];
    char patronymic[50];
    int birth_day, birth_month, birth_year;
    char gender; // 'M' ou 'W'
    double average_income;
} Liver;

// Nó da lista ligada
typedef struct ListNode {
    Liver data;
    struct ListNode* next;
} ListNode;

// Estrutura para gerenciar a lista e o histórico
typedef struct {
    ListNode* head;
    int modification_count; // Contador de modificações
} ResidentList;

// Tipo de ação para o Undo
typedef enum {
    ACTION_ADD,
    ACTION_REMOVE,
    ACTION_MODIFY
} ActionType;

// Nó da pilha de Undo
typedef struct UndoNode {
    ActionType type;
    Liver data; // Cópia dos dados (para adicionar ou restaurar)
    struct UndoNode* next;
} UndoNode;

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
    while (src[i] != '\0') { dest[i] = src[i]; i++; }
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

// Compara duas datas de nascimento (retorna > 0 se a for mais velho que b)
static int compare_age(const Liver* a, const Liver* b) {
    if (a->birth_year != b->birth_year) return b->birth_year - a->birth_year; // Mais velho primeiro (ano menor)
    if (a->birth_month != b->birth_month) return b->birth_month - a->birth_month;
    return b->birth_day - a->birth_day;
}

// ============================================================================
// FUNÇÕES DA LISTA LIGADA
// ============================================================================

/**
 * @brief Insere um habitante na lista mantendo a ordem por idade.
 */
static StatusCode insert_sorted(ResidentList* list, const Liver* liver) {
    ListNode* new_node = (ListNode*)malloc(sizeof(ListNode));
    if (new_node == NULL) return STATUS_ERR_MEMORY;
    new_node->data = *liver;
    new_node->next = NULL;

    // Caso 1: Lista vazia ou novo é mais velho que o primeiro
    if (list->head == NULL || compare_age(liver, &list->head->data) < 0) {
        new_node->next = list->head;
        list->head = new_node;
        return STATUS_OK;
    }

    // Caso 2: Procurar posição
    ListNode* current = list->head;
    while (current->next != NULL && compare_age(liver, &current->next->data) >= 0) {
        current = current->next;
    }
    new_node->next = current->next;
    current->next = new_node;
    return STATUS_OK;
}

/**
 * @brief Busca um habitante pelo nome completo.
 */
static ListNode* find_resident(ResidentList* list, const char* surname, const char* name) {
    ListNode* current = list->head;
    while (current != NULL) {
        if (my_strcmp(current->data.surname, surname) == 0 &&
            my_strcmp(current->data.name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * @brief Remove um habitante da lista.
 */
static StatusCode remove_resident(ResidentList* list, ListNode* target, Liver* out_removed_data) {
    if (target == NULL) return STATUS_ERR_NULL_PTR;

    *out_removed_data = target->data; // Salva para o Undo

    if (list->head == target) {
        list->head = target->next;
    } else {
        ListNode* current = list->head;
        while (current->next != target) {
            current = current->next;
        }
        current->next = target->next;
    }
    free(target);
    return STATUS_OK;
}

// ============================================================================
// PILHA DE UNDO
// ============================================================================

static void push_undo(UndoNode** stack, ActionType type, const Liver* data) {
    UndoNode* new_node = (UndoNode*)malloc(sizeof(UndoNode));
    if (new_node != NULL) {
        new_node->type = type;
        new_node->data = *data;
        new_node->next = *stack;
        *stack = new_node;
    }
}

static int pop_undo(UndoNode** stack, ActionType* out_type, Liver* out_data) {
    if (*stack == NULL) return 0;
    UndoNode* temp = *stack;
    *out_type = temp->type;
    *out_data = temp->data;
    *stack = temp->next;
    free(temp);
    return 1;
}

static void free_undo_stack(UndoNode** stack) {
    while (*stack != NULL) {
        UndoNode* temp = *stack;
        *stack = (*stack)->next;
        free(temp);
    }
}

// ============================================================================
// FUNÇÕES DE I/O
// ============================================================================

StatusCode load_from_file(ResidentList* list, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) return STATUS_ERR_FILE_OPEN;

    Liver temp;
    // Formato: Sobrenome Nome Patronimico DD MM YYYY Genero Renda
    while (fscanf(file, "%49s %49s %49s %d %d %d %c %lf",
                  temp.surname, temp.name, temp.patronymic,
                  &temp.birth_day, &temp.birth_month, &temp.birth_year,
                  &temp.gender, &temp.average_income) == 8) {
        insert_sorted(list, &temp);
    }
    fclose(file);
    return STATUS_OK;
}

void print_resident(const Liver* l) {
    printf("%s %s %s | Nasc: %02d/%02d/%04d | Sexo: %c | Renda: %.2f\n",
           l->surname, l->name, l->patronymic,
           l->birth_day, l->birth_month, l->birth_year,
           l->gender, l->average_income);
}

void export_to_file(ResidentList* list, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Erro ao criar arquivo.\n");
        return;
    }
    ListNode* current = list->head;
    while (current != NULL) {
        fprintf(file, "%s %s %s %d %d %d %c %.2f\n",
                current->data.surname, current->data.name, current->data.patronymic,
                current->data.birth_day, current->data.birth_month, current->data.birth_year,
                current->data.gender, current->data.average_income);
        current = current->next;
    }
    fclose(file);
    printf("Dados exportados para '%s'.\n", filename);
}

// ============================================================================
// FUNÇÃO MAIN
// ============================================================================
int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo_entrada>\n", argv[0]);
        return 1;
    }

    ResidentList list = {NULL, 0};
    UndoNode* undo_stack = NULL;

    if (load_from_file(&list, argv[1]) != STATUS_OK) {
        fprintf(stderr, "Erro ao carregar o arquivo.\n");
        return 1;
    }
    printf("Arquivo carregado. Habitantes ordenados por idade.\n");

    int option = -1;
    while (option != 0) {
        printf("\n=== SISTEMA DE HABITANTES ===");
        printf("\n1. Listar Todos");
        printf("\n2. Adicionar Habitante");
        printf("\n3. Remover Habitante");
        printf("\n4. Modificar Renda");
        printf("\n5. Desfazer Ultima Modificacao (Undo)");
        printf("\n6. Exportar para Arquivo");
        printf("\n0. Sair");
        printf("\nEscolha: ");
        scanf("%d", &option);

        if (option == 1) {
            ListNode* curr = list.head;
            while (curr) { print_resident(&curr->data); curr = curr->next; }
        }
        else if (option == 2) {
            Liver new_liver;
            printf("Sobrenome: "); scanf("%49s", new_liver.surname);
            printf("Nome: "); scanf("%49s", new_liver.name);
            printf("Patronimico: "); scanf("%49s", new_liver.patronymic);
            printf("Dia Nasc: "); scanf("%d", &new_liver.birth_day);
            printf("Mes Nasc: "); scanf("%d", &new_liver.birth_month);
            printf("Ano Nasc: "); scanf("%d", &new_liver.birth_year);
            printf("Sexo (M/W): "); scanf(" %c", &new_liver.gender);
            printf("Renda: "); scanf("%lf", &new_liver.average_income);

            if (insert_sorted(&list, &new_liver) == STATUS_OK) {
                list.modification_count++;
                push_undo(&undo_stack, ACTION_ADD, &new_liver);
                printf("Adicionado.\n");
            }
        }
        else if (option == 3) {
            char s[50], n[50];
            printf("Sobrenome: "); scanf("%49s", s);
            printf("Nome: "); scanf("%49s", n);

            ListNode* target = find_resident(&list, s, n);
            if (target) {
                Liver removed_data;
                remove_resident(&list, target, &removed_data);
                list.modification_count++;
                push_undo(&undo_stack, ACTION_REMOVE, &removed_data);
                printf("Removido.\n");
            } else {
                printf("Nao encontrado.\n");
            }
        }
        else if (option == 4) {
            char s[50], n[50];
            printf("Sobrenome: "); scanf("%49s", s);
            printf("Nome: "); scanf("%49s", n);

            ListNode* target = find_resident(&list, s, n);
            if (target) {
                Liver old_data = target->data; // Salva estado antigo
                printf("Nova Renda: ");
                scanf("%lf", &target->data.average_income);
                list.modification_count++;
                push_undo(&undo_stack, ACTION_MODIFY, &old_data); // Empilha o estado antigo
                printf("Modificado.\n");
            } else {
                printf("Nao encontrado.\n");
            }
        }
        else if (option == 5) {
            ActionType type;
            Liver data;
            if (pop_undo(&undo_stack, &type, &data)) {
                if (type == ACTION_ADD) {
                    // Desfazer adição = remover
                    ListNode* target = find_resident(&list, data.surname, data.name);
                    if (target) {
                        Liver dummy;
                        remove_resident(&list, target, &dummy);
                    }
                } else if (type == ACTION_REMOVE) {
                    // Desfazer remoção = adicionar de volta
                    insert_sorted(&list, &data);
                } else if (type == ACTION_MODIFY) {
                    // Desfazer modificação = restaurar dados antigos
                    ListNode* target = find_resident(&list, data.surname, data.name);
                    if (target) target->data = data;
                }
                list.modification_count--;
                printf("Undo realizado com sucesso.\n");
            } else {
                printf("Nada para desfazer.\n");
            }
        }
        else if (option == 6) {
            char filename[100];
            printf("Nome do arquivo de saida: ");
            scanf("%99s", filename);
            export_to_file(&list, filename);
        }
    }

    // Limpeza final
    ListNode* curr = list.head;
    while (curr) {
        ListNode* temp = curr;
        curr = curr->next;
        free(temp);
    }
    free_undo_stack(&undo_stack);

    printf("Saindo...\n");
    return 0;
}
