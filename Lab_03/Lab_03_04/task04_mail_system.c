/*
================================================================================
ENUNCIADO / ЗАДАНИЕ
================================================================================
[RUSSO]
4. 1. Опишите тип структуры String... (создание, удаление, сравнение, копирование,
конкатенация).
2. Экземпляр структуры Mail содержит Address получателя, вес, идентификатор,
время создания и вручения. Экземпляр Post содержит указатель на Address
и динамический массив Mail. Реализуйте интерактивный диалог для добавления,
удаления, поиска (по ID), сортировки (по провинции, затем по ID) и вывода
доставленных/просроченных отправлений.

[PORTUGUÊS]
4. 1. Descreva a estrutura String (ponteiro char* e int len). Implemente funções:
   - Criar a partir de char*.
   - Deletar conteúdo interno.
   - Relação de ordem (tamanho, depois lexicográfico).
   - Relação de equivalência (lexicográfico).
   - Copiar para existente e copiar para novo (dinâmico).
   - Concatenar.
2. A estrutura Mail contém: Address do destinatário (Província, Município, Bairro,
   Rua, Número da Casa), peso (float), ID de Rastreio (ex: 14 dígitos),
   data de criação e data de entrega (dd:MM:yyyy hh:mm:ss).
   A estrutura Post contém: Address da agência e um array dinâmico de Mail.
   Implemente um menu interativo para: adicionar, remover, buscar por ID,
   ordenar (por Província e depois por ID), buscar todas as entregues e todas as
   atrasadas (comparando com o tempo do sistema). Use a estrutura String do item 1.

RESTRIÇÕES: Sem <string.h>, sem variáveis globais, alocação dinâmica,
tratamento de erros com enum, separação de lógica e I/O.

OBSERVAÇÕES / ПРИМЕЧАНИЯ:
1. Contexto adaptado para Angola (Províncias, Municípios, Ruas).
2. O campo "CEP" foi substituído por "Província" como chave primária de ordenação.
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ============================================================================
// DEFINIÇÃO DE STATUS CODES
// ============================================================================
typedef enum {
    STATUS_OK = 0,
    STATUS_ERR_MEMORY,
    STATUS_ERR_NULL_PTR,
    STATUS_ERR_INVALID_ARG,
    STATUS_ERR_NOT_FOUND
} StatusCode;

// ============================================================================
// PARTE 1: ESTRUTURA STRING E FUNÇÕES BÁSICAS
// ============================================================================
typedef struct {
    char* data;
    int length;
} String;

static int my_strlen(const char* str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

StatusCode string_create(String* out_str, const char* src) {
    if (out_str == NULL || src == NULL) return STATUS_ERR_NULL_PTR;

    int len = my_strlen(src);
    out_str->data = (char*)malloc((len + 1) * sizeof(char));
    if (out_str->data == NULL) return STATUS_ERR_MEMORY;

    for (int i = 0; i <= len; i++) {
        out_str->data[i] = src[i];
    }
    out_str->length = len;
    return STATUS_OK;
}

void string_destroy(String* str) {
    if (str != NULL && str->data != NULL) {
        free(str->data);
        str->data = NULL;
        str->length = 0;
    }
}

int string_compare_order(const String* s1, const String* s2) {
    if (s1->length < s2->length) return -1;
    if (s1->length > s2->length) return 1;
    for (int i = 0; i < s1->length; i++) {
        if (s1->data[i] < s2->data[i]) return -1;
        if (s1->data[i] > s2->data[i]) return 1;
    }
    return 0;
}

int string_equals(const String* s1, const String* s2) {
    if (s1->length != s2->length) return 0;
    for (int i = 0; i < s1->length; i++) {
        if (s1->data[i] != s2->data[i]) return 0;
    }
    return 1;
}

StatusCode string_copy(String* dest, const String* src) {
    if (dest == NULL || src == NULL) return STATUS_ERR_NULL_PTR;

    char* new_data = (char*)realloc(dest->data, (src->length + 1) * sizeof(char));
    if (new_data == NULL) return STATUS_ERR_MEMORY;

    dest->data = new_data;
    for (int i = 0; i <= src->length; i++) {
        dest->data[i] = src->data[i];
    }
    dest->length = src->length;
    return STATUS_OK;
}

StatusCode string_clone(const String* src, String* out_dest) {
    return string_create(out_dest, src->data);
}

StatusCode string_concat(String* s1, const String* s2) {
    if (s1 == NULL || s2 == NULL) return STATUS_ERR_NULL_PTR;

    int new_len = s1->length + s2->length;
    char* new_data = (char*)realloc(s1->data, (new_len + 1) * sizeof(char));
    if (new_data == NULL) return STATUS_ERR_MEMORY;

    s1->data = new_data;
    for (int i = 0; i <= s2->length; i++) {
        s1->data[s1->length + i] = s2->data[i];
    }
    s1->length = new_len;
    return STATUS_OK;
}

// ============================================================================
// PARTE 2: ESTRUTURAS DE ENCOMENDA (CONTEXTO ANGOLA)
// ============================================================================

typedef struct {
    String province; // Ex: Luanda, Benguela
    String municipality; // Ex: Belas, Lobito
    String neighborhood; // Ex: Talatona, Restinga
    String street;
    int house_num;
} Address;

typedef struct {
    Address recipient;
    double weight;
    String mail_id;    // 14 dígitos (Simulando código de rastreio)
    String create_time; // dd:MM:yyyy hh:mm:ss
    String delivery_time;
} Mail;

typedef struct {
    Address current_office;
    Mail* mails;
    int count;
    int capacity;
} Post;

// Converte string de data para time_t
static time_t parse_time(const String* time_str) {
    struct tm tm = {0};
    if (time_str->length < 19) return 0;

    tm.tm_mday = (time_str->data[0] - '0') * 10 + (time_str->data[1] - '0');
    tm.tm_mon  = ((time_str->data[3] - '0') * 10 + (time_str->data[4] - '0')) - 1;
    tm.tm_year = ((time_str->data[6] - '0') * 1000 + (time_str->data[7] - '0') * 100 +
                  (time_str->data[8] - '0') * 10 + (time_str->data[9] - '0')) - 1900;
    tm.tm_hour = (time_str->data[11] - '0') * 10 + (time_str->data[12] - '0');
    tm.tm_min  = (time_str->data[14] - '0') * 10 + (time_str->data[15] - '0');
    tm.tm_sec  = (time_str->data[17] - '0') * 10 + (time_str->data[18] - '0');

    return mktime(&tm);
}

// ============================================================================
// FUNÇÕES DE LÓGICA DO POST
// ============================================================================

StatusCode post_init(Post* post) {
    if (post == NULL) return STATUS_ERR_NULL_PTR;

    // Agência Central em Luanda
    string_create(&post->current_office.province, "Luanda");
    string_create(&post->current_office.municipality, "Luanda");
    string_create(&post->current_office.neighborhood, "Maianga");
    string_create(&post->current_office.street, "Rua Rainha Ginga");
    post->current_office.house_num = 100;

    post->capacity = 10;
    post->mails = (Mail*)malloc(post->capacity * sizeof(Mail));
    if (post->mails == NULL) return STATUS_ERR_MEMORY;
    post->count = 0;

    return STATUS_OK;
}

void post_destroy(Post* post) {
    if (post == NULL) return;
    string_destroy(&post->current_office.province);
    string_destroy(&post->current_office.municipality);
    string_destroy(&post->current_office.neighborhood);
    string_destroy(&post->current_office.street);

    for (int i = 0; i < post->count; i++) {
        string_destroy(&post->mails[i].recipient.province);
        string_destroy(&post->mails[i].recipient.municipality);
        string_destroy(&post->mails[i].recipient.neighborhood);
        string_destroy(&post->mails[i].recipient.street);
        string_destroy(&post->mails[i].mail_id);
        string_destroy(&post->mails[i].create_time);
        string_destroy(&post->mails[i].delivery_time);
    }
    free(post->mails);
}

StatusCode post_add_mail(Post* post, Mail* mail) {
    if (post == NULL || mail == NULL) return STATUS_ERR_NULL_PTR;

    if (post->count == post->capacity) {
        int new_cap = post->capacity * 2;
        Mail* new_mails = (Mail*)realloc(post->mails, new_cap * sizeof(Mail));
        if (new_mails == NULL) return STATUS_ERR_MEMORY;
        post->mails = new_mails;
        post->capacity = new_cap;
    }

    post->mails[post->count] = *mail;
    post->count++;
    return STATUS_OK;
}

// Comparador: Província (primário), ID (secundário)
static int compare_mails(const void* a, const void* b) {
    const Mail* m1 = (const Mail*)a;
    const Mail* m2 = (const Mail*)b;

    int cmp_prov = string_compare_order(&m1->recipient.province, &m2->recipient.province);
    if (cmp_prov != 0) return cmp_prov;

    return string_compare_order(&m1->mail_id, &m2->mail_id);
}

static int compare_time_asc(const void* a, const void* b) {
    const Mail* m1 = (const Mail*)a;
    const Mail* m2 = (const Mail*)b;
    time_t t1 = parse_time(&m1->create_time);
    time_t t2 = parse_time(&m2->create_time);
    if (t1 < t2) return -1;
    if (t1 > t2) return 1;
    return 0;
}

static void print_mail(const Mail* m) {
    printf("ID: %s | Peso: %.2f kg | Criado: %s | Entrega: %s\n",
           m->mail_id.data, m->weight, m->create_time.data,
           m->delivery_time.length > 0 ? m->delivery_time.data : "Pendente");
    printf("   Destino: %s, %s, Bairro %s, %s Nº %d\n",
           m->recipient.province.data, m->recipient.municipality.data,
           m->recipient.neighborhood.data, m->recipient.street.data, m->recipient.house_num);
}

// ============================================================================
// FUNÇÃO MAIN (MENU INTERATIVO)
// ============================================================================
int main() {
    Post post;
    StatusCode status = post_init(&post);
    if (status != STATUS_OK) {
        fprintf(stderr, "Erro ao inicializar os correios.\n");
        return 1;
    }

    // --- DADOS DE TESTE (CONTEXTO ANGOLANO) ---
    Mail m1;
    string_create(&m1.recipient.province, "Benguela");
    string_create(&m1.recipient.municipality, "Lobito");
    string_create(&m1.recipient.neighborhood, "Restinga");
    string_create(&m1.recipient.street, "Av. 10 de Fevereiro");
    m1.recipient.house_num = 45;
    m1.weight = 2.3;
    string_create(&m1.mail_id, "AO123456789012");
    string_create(&m1.create_time, "10:10:2023 10:00:00");
    string_create(&m1.delivery_time, "15:10:2023 14:00:00"); // Já entregue
    post_add_mail(&post, &m1);

    Mail m2;
    string_create(&m2.recipient.province, "Luanda");
    string_create(&m2.recipient.municipality, "Belas");
    string_create(&m2.recipient.neighborhood, "Talatona");
    string_create(&m2.recipient.street, "Rua do Cruzeiro");
    m2.recipient.house_num = 12;
    m2.weight = 0.8;
    string_create(&m2.mail_id, "AO987654321098");
    string_create(&m2.create_time, "20:10:2023 08:30:00");
    string_create(&m2.delivery_time, ""); // Pendente
    post_add_mail(&post, &m2);

    int option = -1;
    while (option != 0) {
        printf("\n=== SISTEMA DE ENCOMENDAS DE ANGOLA (CORREIOS) ===");
        printf("\n1. Registar Encomenda");
        printf("\n2. Remover Encomenda (por ID)");
        printf("\n3. Buscar por ID");
        printf("\n4. Listar Todas (Ordenadas por Provincia e ID)");
        printf("\n5. Listar Entregues (por data de criacao)");
        printf("\n6. Listar Atrasadas / Pendentes (por data de criacao)");
        printf("\n0. Sair");
        printf("\nEscolha: ");
        scanf("%d", &option);

        if (option == 1) {
            Mail new_mail;
            char buf[100];
            printf("Provincia (ex: Luanda): "); scanf("%99s", buf); string_create(&new_mail.recipient.province, buf);
            printf("Municipio (ex: Belas): "); scanf("%99s", buf); string_create(&new_mail.recipient.municipality, buf);
            printf("Bairro (ex: Talatona): "); scanf("%99s", buf); string_create(&new_mail.recipient.neighborhood, buf);
            printf("Rua: "); scanf("%99s", buf); string_create(&new_mail.recipient.street, buf);
            printf("Numero da Casa: "); scanf("%d", &new_mail.recipient.house_num);
            printf("Peso (kg): "); scanf("%lf", &new_mail.weight);
            printf("Codigo de Rastreio: "); scanf("%99s", buf); string_create(&new_mail.mail_id, buf);

            char time_buf[30];
            printf("Data/Hora Criacao (dd:MM:yyyy hh:mm:ss): ");
            scanf(" %[^\n]", time_buf);
            string_create(&new_mail.create_time, time_buf);
            string_create(&new_mail.delivery_time, "");

            if (post_add_mail(&post, &new_mail) == STATUS_OK) {
                printf("Encomenda registada com sucesso!\n");
            }
        }
        else if (option == 2) {
            char search_id[20];
            printf("Digite o ID a remover: ");
            scanf("%19s", search_id);
            String search_str;
            string_create(&search_str, search_id);

            int found = -1;
            for (int i = 0; i < post.count; i++) {
                if (string_equals(&post.mails[i].mail_id, &search_str)) {
                    found = i;
                    break;
                }
            }

            if (found != -1) {
                string_destroy(&post.mails[found].recipient.province);
                string_destroy(&post.mails[found].recipient.municipality);
                string_destroy(&post.mails[found].recipient.neighborhood);
                string_destroy(&post.mails[found].recipient.street);
                string_destroy(&post.mails[found].mail_id);
                string_destroy(&post.mails[found].create_time);
                string_destroy(&post.mails[found].delivery_time);

                for (int i = found; i < post.count - 1; i++) {
                    post.mails[i] = post.mails[i + 1];
                }
                post.count--;
                printf("Removido com sucesso.\n");
            } else {
                printf("Nao encontrado.\n");
            }
            string_destroy(&search_str);
        }
        else if (option == 3) {
            char search_id[20];
            printf("Digite o ID: ");
            scanf("%19s", search_id);
            String search_str;
            string_create(&search_str, search_id);

            int found = 0;
            for (int i = 0; i < post.count; i++) {
                if (string_equals(&post.mails[i].mail_id, &search_str)) {
                    print_mail(&post.mails[i]);
                    found = 1;
                }
            }
            if (!found) printf("Nao encontrado.\n");
            string_destroy(&search_str);
        }
        else if (option == 4) {
            qsort(post.mails, post.count, sizeof(Mail), compare_mails);
            for (int i = 0; i < post.count; i++) {
                print_mail(&post.mails[i]);
            }
        }
        else if (option == 5) {
            Mail* temp = (Mail*)malloc(post.count * sizeof(Mail));
            int count_temp = 0;
            for (int i = 0; i < post.count; i++) {
                if (post.mails[i].delivery_time.length > 0) {
                    temp[count_temp++] = post.mails[i];
                }
            }
            qsort(temp, count_temp, sizeof(Mail), compare_time_asc);
            for (int i = 0; i < count_temp; i++) print_mail(&temp[i]);
            free(temp);
        }
        else if (option == 6) {
            time_t now = time(NULL);
            Mail* temp = (Mail*)malloc(post.count * sizeof(Mail));
            int count_temp = 0;
            for (int i = 0; i < post.count; i++) {
                if (post.mails[i].delivery_time.length == 0) {
                    time_t create_t = parse_time(&post.mails[i].create_time);
                    if (create_t < now) {
                        temp[count_temp++] = post.mails[i];
                    }
                }
            }
            qsort(temp, count_temp, sizeof(Mail), compare_time_asc);
            for (int i = 0; i < count_temp; i++) print_mail(&temp[i]);
            free(temp);
        }
    }

    post_destroy(&post);
    printf("Saindo do sistema...\n");
    return 0;
}
