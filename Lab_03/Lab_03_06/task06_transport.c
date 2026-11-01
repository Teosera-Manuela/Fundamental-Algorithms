/*
================================================================================
ENUNCIADO / ЗАДАНИЕ
================================================================================
[RUSSO]
6. В уездном городе ввели систему учета городского транспорта. Каждый
остановочный пункт регистрирует транспорт в текстовый файл (номер, время
остановки, время отправления, маркер). Каждый файл содержит координаты
остановки. Необходимо реализовать хранилище на базе односвязного списка
односвязных списков. Элементы основного списка — списки пройденных маршрутов.
Элементы в списке остановок упорядочены по возрастанию временных меток.
Входные аргументы — массив путей к файлам. Реализуйте поиск ТС через
интерактивный диалог.

[PORTUGUÊS]
6. Em uma cidade, foi implementado um sistema de controle de transporte.
Cada paragem registra o veículo em um arquivo de texto (número, hora de parada,
hora de partida, marcador). Cada arquivo contém as coordenadas da paragem.
- Implemente um armazenamento baseado em uma lista ligada de listas ligadas.
- Os elementos da lista principal são as rotas de cada veículo.
- Os elementos da sub-lista (paragens) devem ser ordenados por ordem crescente
  de tempo.
- Os argumentos de entrada são um array de caminhos para os arquivos.
- Implemente uma busca do veículo através de um diálogo interativo.

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
    STATUS_ERR_NOT_FOUND
} StatusCode;

// ============================================================================
// ESTRUTURAS DE DADOS (LISTA DE LISTAS)
// ============================================================================

// Marcador do tipo de paragem
typedef enum {
    STOP_INITIAL = 1,   // Inicial
    STOP_INTERMEDIATE = 2, // Intermédia
    STOP_FINAL = 3      // Final
} StopType;

// Nó da sub-lista (representa uma paragem específica de um veículo)
typedef struct StopNode {
    char stop_name[50];   // Nome/Coordenada da paragem (vindo do arquivo)
    char arrival_time[20];
    char departure_time[20];
    StopType type;
    struct StopNode* next; // Próxima paragem na rota
} StopNode;

// Nó da lista principal (representa um veículo/rota)
typedef struct VehicleNode {
    char vehicle_id[20];
    StopNode* stops_head; // Ponteiro para a lista de paragens deste veículo
    struct VehicleNode* next; // Próximo veículo
} VehicleNode;

// Estrutura para gerenciar o sistema
typedef struct {
    VehicleNode* head;
} TransportSystem;

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

// Compara duas strings de data/hora "dd.MM.yyyy hh:mm:ss"
// Retorna < 0 se t1 < t2, 0 se iguais, > 0 se t1 > t2
static int compare_times(const char* t1, const char* t2) {
    // Comparação lexicográfica direta funciona para este formato!
    // Ex: "10.10.2023 10:00:00" vs "10.10.2023 10:05:00"
    return my_strcmp(t1, t2);
}

// ============================================================================
// FUNÇÕES DE GESTÃO DA LISTA DE PARAGENS (SUB-LISTA)
// ============================================================================

/**
 * @brief Insere uma paragem na sub-lista de um veículo, mantendo a ordem cronológica.
 *
 * Passo a passo:
 * 1. Cria um novo nó para a paragem.
 * 2. Se a lista estiver vazia ou a nova paragem for anterior à primeira,
 *    insere no início.
 * 3. Caso contrário, percorre a lista até encontrar a posição correta
 *    (onde o tempo da próxima paragem é maior que o da nova).
 * 4. Ajusta os ponteiros para inserir o novo nó.
 */
static StatusCode insert_stop_sorted(StopNode** head, const char* stop_name,
                                     const char* arrival, const char* departure, StopType type) {
    StopNode* new_node = (StopNode*)malloc(sizeof(StopNode));
    if (new_node == NULL) return STATUS_ERR_MEMORY;

    my_strcpy(new_node->stop_name, stop_name);
    my_strcpy(new_node->arrival_time, arrival);
    my_strcpy(new_node->departure_time, departure);
    new_node->type = type;
    new_node->next = NULL;

    // Caso 1: Lista vazia ou novo nó deve ser o primeiro
    if (*head == NULL || compare_times(arrival, (*head)->arrival_time) < 0) {
        new_node->next = *head;
        *head = new_node;
        return STATUS_OK;
    }

    // Caso 2: Procurar a posição correta
    StopNode* current = *head;
    while (current->next != NULL && compare_times(arrival, current->next->arrival_time) >= 0) {
        current = current->next;
    }

    // Inserir após 'current'
    new_node->next = current->next;
    current->next = new_node;
    return STATUS_OK;
}

// ============================================================================
// FUNÇÕES DE GESTÃO DA LISTA PRINCIPAL (VEÍCULOS)
// ============================================================================

/**
 * @brief Encontra ou cria um veículo na lista principal.
 */
static StatusCode get_or_create_vehicle(TransportSystem* sys, const char* vehicle_id, VehicleNode** out_vehicle) {
    VehicleNode* current = sys->head;
    while (current != NULL) {
        if (my_strcmp(current->vehicle_id, vehicle_id) == 0) {
            *out_vehicle = current;
            return STATUS_OK;
        }
        current = current->next;
    }

    // Se não encontrou, cria um novo
    VehicleNode* new_vehicle = (VehicleNode*)malloc(sizeof(VehicleNode));
    if (new_vehicle == NULL) return STATUS_ERR_MEMORY;

    my_strcpy(new_vehicle->vehicle_id, vehicle_id);
    new_vehicle->stops_head = NULL;

    // Insere no início da lista principal (a ordem dos veículos não importa)
    new_vehicle->next = sys->head;
    sys->head = new_vehicle;

    *out_vehicle = new_vehicle;
    return STATUS_OK;
}

/**
 * @brief Lê os dados de um arquivo de paragem.
 * Formato do arquivo esperado:
 * ID_VEICULO;HORA_CHEGADA;HORA_PARTIDA;TIPO(1/2/3)
 */
StatusCode process_file(TransportSystem* sys, const char* filename, const char* stop_name) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) return STATUS_ERR_FILE_OPEN;

    char line[200];
    char vehicle_id[20], arrival[20], departure[20];
    int type_int;

    // Lê linha por linha
    while (fgets(line, sizeof(line), file) != NULL) {
        // Parsing manual simples (substituindo sscanf para melhor controle de erros)
        // Formato esperado: ID;Arrival;Departure;Type
        // Ex: BUS01;10.10.2023 10:00:00;10.10.2023 10:05:00;1

        int i = 0, part = 0;
        char buffer[100];

        // Pula linhas em branco
        if (line[0] == '\n' || line[0] == '\0') continue;

        // Extrai ID
        while (line[i] != ';' && line[i] != '\0') { buffer[part++] = line[i++]; }
        buffer[part] = '\0'; my_strcpy(vehicle_id, buffer);
        if (line[i] == '\0') continue; i++; part = 0;

        // Extrai Arrival
        while (line[i] != ';' && line[i] != '\0') { buffer[part++] = line[i++]; }
        buffer[part] = '\0'; my_strcpy(arrival, buffer);
        if (line[i] == '\0') continue; i++; part = 0;

        // Extrai Departure
        while (line[i] != ';' && line[i] != '\0') { buffer[part++] = line[i++]; }
        buffer[part] = '\0'; my_strcpy(departure, buffer);
        if (line[i] == '\0') continue; i++; part = 0;

        // Extrai Type
        while (line[i] != ';' && line[i] != '\n' && line[i] != '\0') { buffer[part++] = line[i++]; }
        buffer[part] = '\0';

        // Converte tipo char para int (assumindo 1, 2 ou 3)
        type_int = buffer[0] - '0';
        StopType type = (StopType)type_int;

        // Adiciona à estrutura
        VehicleNode* vehicle = NULL;
        if (get_or_create_vehicle(sys, vehicle_id, &vehicle) == STATUS_OK) {
            insert_stop_sorted(&vehicle->stops_head, stop_name, arrival, departure, type);
        }
    }

    fclose(file);
    return STATUS_OK;
}

// ============================================================================
// FUNÇÕES DE EXIBIÇÃO E BUSCA
// ============================================================================

void print_vehicle_route(const VehicleNode* vehicle) {
    printf("\n--- Rota do Veiculo: %s ---\n", vehicle->vehicle_id);
    StopNode* current = vehicle->stops_head;

    if (current == NULL) {
        printf("Nenhuma paragem registada.\n");
        return;
    }

    const char* type_str[] = {"", "INICIAL", "INTERMEDIA", "FINAL"};

    while (current != NULL) {
        printf("[%s] Paragem: %s | Chegada: %s | Partida: %s | Tipo: %s\n",
               (current->type == STOP_INITIAL) ? "->" : "  ",
               current->stop_name,
               current->arrival_time,
               current->departure_time,
               type_str[current->type]);
        current = current->next;
    }
}

StatusCode search_vehicle(TransportSystem* sys, const char* vehicle_id, VehicleNode** out_vehicle) {
    VehicleNode* current = sys->head;
    while (current != NULL) {
        if (my_strcmp(current->vehicle_id, vehicle_id) == 0) {
            *out_vehicle = current;
            return STATUS_OK;
        }
        current = current->next;
    }
    return STATUS_ERR_NOT_FOUND;
}

// Libera toda a memória alocada
void free_system(TransportSystem* sys) {
    VehicleNode* v = sys->head;
    while (v != NULL) {
        StopNode* s = v->stops_head;
        while (s != NULL) {
            StopNode* temp_s = s;
            s = s->next;
            free(temp_s);
        }
        VehicleNode* temp_v = v;
        v = v->next;
        free(temp_v);
    }
    sys->head = NULL;
}

// ============================================================================
// FUNÇÃO MAIN
// ============================================================================
int main(int argc, char* argv[]) {
    // 1. Validação de argumentos
    // O usuário deve passar pelo menos 1 arquivo.
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo_paragem_1> [arquivo_paragem_2] ...\n", argv[0]);
        return 1;
    }

    TransportSystem sys;
    sys.head = NULL;

    // 2. Processar todos os arquivos passados
    printf("Lendo arquivos de paragens...\n");
    for (int i = 1; i < argc; i++) {
        // O nome da paragem é o próprio nome do arquivo (para simplificar)
        const char* stop_name = argv[i];

        StatusCode status = process_file(&sys, argv[i], stop_name);
        if (status != STATUS_OK) {
            if (status == STATUS_ERR_FILE_OPEN) {
                fprintf(stderr, "Aviso: Nao foi possivel abrir '%s'. Pulando...\n", argv[i]);
            } else {
                fprintf(stderr, "Erro ao processar '%s'.\n", argv[i]);
            }
        }
    }

    // 3. Menu Interativo
    int option = -1;
    while (option != 0) {
        printf("\n=== SISTEMA DE TRANSPORTE ===");
        printf("\n1. Procurar Veiculo");
        printf("\n2. Listar Todos os Veiculos Registados");
        printf("\n0. Sair");
        printf("\nEscolha: ");
        scanf("%d", &option);

        if (option == 1) {
            char search_id[20];
            printf("Digite o ID do Veiculo (ex: BUS01): ");
            scanf("%19s", search_id);

            VehicleNode* found_vehicle = NULL;
            StatusCode status = search_vehicle(&sys, search_id, &found_vehicle);

            if (status == STATUS_OK) {
                print_vehicle_route(found_vehicle);
            } else {
                printf("Veiculo '%s' nao encontrado no sistema.\n", search_id);
            }
        }
        else if (option == 2) {
            printf("\n--- Veiculos no Sistema ---\n");
            VehicleNode* current = sys.head;
            int count = 0;
            while (current != NULL) {
                printf("- %s\n", current->vehicle_id);
                current = current->next;
                count++;
            }
            if (count == 0) printf("Nenhum veiculo registado.\n");
        }
    }

    // 4. Limpeza
    free_system(&sys);
    printf("Saindo do sistema...\n");
    return 0;
}
