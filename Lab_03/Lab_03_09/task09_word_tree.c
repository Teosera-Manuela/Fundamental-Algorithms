/*
================================================================================
ENUNCIADO / ЗАДАНИЕ
================================================================================
9. A) Реализуйте приложение для сбора статистики по заданному тексту...
[PORTUGUÊS]
9. A) Implemente um aplicativo para coletar estatísticas de um texto...
[VERSÃO CORRIGIDA: SEM LOOP INFINITO NO MENU]
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
    STATUS_ERR_FILE_OPEN
} StatusCode;

// ============================================================================
// ESTRUTURA DA ÁRVORE BINÁRIA DE BUSCA (BST)
// ============================================================================
typedef struct TreeNode {
    char* word;
    int count;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

// ============================================================================
// FUNÇÕES AUXILIARES (Sem <string.h>)
// ============================================================================
static int my_strlen(const char* str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
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

static char* my_strdup(const char* src) {
    int len = my_strlen(src);
    char* dest = (char*)malloc((len + 1) * sizeof(char));
    if (dest == NULL) return NULL;
    for (int i = 0; i <= len; i++) dest[i] = src[i];
    return dest;
}

static int is_separator(char c, const char* separators) {
    for (int i = 0; separators[i] != '\0'; i++) {
        if (c == separators[i]) return 1;
    }
    return 0;
}

// ============================================================================
// OPERAÇÕES DA ÁRVORE (BST)
// ============================================================================
StatusCode tree_insert(TreeNode** root, const char* word) {
    if (root == NULL || word == NULL) return STATUS_ERR_NULL_PTR;

    if (*root == NULL) {
        TreeNode* new_node = (TreeNode*)malloc(sizeof(TreeNode));
        if (new_node == NULL) return STATUS_ERR_MEMORY;

        new_node->word = my_strdup(word);
        if (new_node->word == NULL) {
            free(new_node);
            return STATUS_ERR_MEMORY;
        }
        new_node->count = 1;
        new_node->left = NULL;
        new_node->right = NULL;
        *root = new_node;
        return STATUS_OK;
    }

    int cmp = my_strcmp(word, (*root)->word);
    if (cmp == 0) {
        (*root)->count++;
        return STATUS_OK;
    } else if (cmp < 0) {
        return tree_insert(&((*root)->left), word);
    } else {
        return tree_insert(&((*root)->right), word);
    }
}

int tree_search(TreeNode* root, const char* word) {
    if (root == NULL) return -1;
    int cmp = my_strcmp(word, root->word);
    if (cmp == 0) return root->count;
    if (cmp < 0) return tree_search(root->left, word);
    return tree_search(root->right, word);
}

// Conta quantos nós únicos existem na árvore
int tree_count_nodes(TreeNode* root) {
    if (root == NULL) return 0;
    return 1 + tree_count_nodes(root->left) + tree_count_nodes(root->right);
}

void tree_dfs_inorder(TreeNode* root) {
    if (root == NULL) return;
    tree_dfs_inorder(root->left);
    printf("%s: %d\n", root->word, root->count);
    tree_dfs_inorder(root->right);
}

// ============================================================================
// FUNÇÕES PARA ENCONTRAR PALAVRAS EXTREMAS E TOP N
// ============================================================================
typedef struct {
    char* word;
    int count;
} WordStat;

static void collect_stats(TreeNode* root, WordStat* stats, int* index) {
    if (root == NULL) return;
    collect_stats(root->left, stats, index);
    stats[*index].word = root->word;
    stats[*index].count = root->count;
    (*index)++;
    collect_stats(root->right, stats, index);
}

static int compare_stats(const void* a, const void* b) {
    const WordStat* w1 = (const WordStat*)a;
    const WordStat* w2 = (const WordStat*)b;
    return w2->count - w1->count;
}

void tree_find_extremes(TreeNode* root, char** longest, char** shortest) {
    if (root == NULL) return;
    int len = my_strlen(root->word);
    if (*longest == NULL || len > my_strlen(*longest)) *longest = root->word;
    if (*shortest == NULL || len < my_strlen(*shortest)) *shortest = root->word;
    tree_find_extremes(root->left, longest, shortest);
    tree_find_extremes(root->right, longest, shortest);
}

// ============================================================================
// PARTE C: SALVAR E CARREGAR ÁRVORE
// ============================================================================
StatusCode tree_save_binary(TreeNode* root, FILE* file) {
    if (root == NULL) {
        int is_null = 1;
        fwrite(&is_null, sizeof(int), 1, file);
        return STATUS_OK;
    }
    int is_null = 0;
    fwrite(&is_null, sizeof(int), 1, file);
    int len = my_strlen(root->word);
    fwrite(&len, sizeof(int), 1, file);
    fwrite(root->word, sizeof(char), len, file);
    fwrite(&(root->count), sizeof(int), 1, file);

    tree_save_binary(root->left, file);
    tree_save_binary(root->right, file);
    return STATUS_OK;
}

StatusCode tree_load_binary(TreeNode** root, FILE* file) {
    int is_null;
    if (fread(&is_null, sizeof(int), 1, file) != 1) return STATUS_OK;
    if (is_null) {
        *root = NULL;
        return STATUS_OK;
    }
    TreeNode* new_node = (TreeNode*)malloc(sizeof(TreeNode));
    if (new_node == NULL) return STATUS_ERR_MEMORY;

    int len;
    fread(&len, sizeof(int), 1, file);
    new_node->word = (char*)malloc((len + 1) * sizeof(char));
    fread(new_node->word, sizeof(char), len, file);
    new_node->word[len] = '\0';

    fread(&(new_node->count), sizeof(int), 1, file);
    new_node->left = NULL;
    new_node->right = NULL;
    *root = new_node;

    tree_load_binary(&((*root)->left), file);
    tree_load_binary(&((*root)->right), file);
    return STATUS_OK;
}

void tree_free(TreeNode* root) {
    if (root == NULL) return;
    tree_free(root->left);
    tree_free(root->right);
    free(root->word);
    free(root);
}

// ============================================================================
// FUNÇÃO MAIN (COM CORREÇÃO DO LOOP DO MENU)
// ============================================================================
int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <arquivo_texto> <separadores>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s texto.txt \" .,;\"\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];
    const char* separators = argv[2];

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Erro: Nao foi possivel abrir o arquivo '%s'.\n", filename);
        return 1;
    }

    TreeNode* root = NULL;
    char buffer[100];
    int buf_idx = 0;
    int c;

    while ((c = fgetc(file)) != EOF) {
        if (is_separator((char)c, separators) || c == '\n') {
            if (buf_idx > 0) {
                buffer[buf_idx] = '\0';
                tree_insert(&root, buffer);
                buf_idx = 0;
            }
        } else {
            if (buf_idx < 99) buffer[buf_idx++] = (char)c;
        }
    }
    if (buf_idx > 0) {
        buffer[buf_idx] = '\0';
        tree_insert(&root, buffer);
    }
    fclose(file);

    printf("Arquivo processado. Palavras inseridas na Arvore.\n");

    int option = -1;
    char input_buffer[100]; // Buffer para ler a entrada do menu com segurança

    while (option != 0) {
        printf("\n=== ESTATISTICAS DE PALAVRAS ===");
        printf("\n1. Buscar palavra");
        printf("\n2. Mostrar N palavras mais frequentes");
        printf("\n3. Mostrar palavra mais longa e mais curta");
        printf("\n4. Mostrar todas as palavras (DFS In-Order)");
        printf("\n5. Salvar Arvore em arquivo binario");
        printf("\n6. Carregar Arvore de arquivo binario");
        printf("\n0. Sair");
        printf("\nEscolha: ");

        // CORREÇÃO CRÍTICA: Usar fgets em vez de scanf para o menu
        if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) break;
        if (sscanf(input_buffer, "%d", &option) != 1) {
            printf("Entrada invalida. Digite um numero.\n");
            continue;
        }

        if (option == 1) {
            char search[100];
            printf("Digite a palavra: ");
            if (fgets(search, sizeof(search), stdin) != NULL) {
                // Remove o '\n' do final da string lida
                int len = my_strlen(search);
                if (len > 0 && search[len-1] == '\n') search[len-1] = '\0';

                int freq = tree_search(root, search);
                if (freq != -1) {
                    printf("A palavra '%s' aparece %d vez(es).\n", search, freq);
                } else {
                    printf("Palavra nao encontrada.\n");
                }
            }
        }
        else if (option == 2) {
            int n;
            printf("Quantas palavras deseja ver? ");
            if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
                if (sscanf(input_buffer, "%d", &n) != 1) {
                    printf("Valor invalido.\n");
                    continue;
                }

                int total_words = tree_count_nodes(root);
                if (total_words == 0) {
                    printf("Arvore vazia.\n");
                    continue;
                }

                // ALOCAÇÃO SEGURA: Aloca exatamente o número de nós da árvore
                WordStat* stats = (WordStat*)malloc(total_words * sizeof(WordStat));
                if (stats == NULL) {
                    printf("Erro de memoria.\n");
                    continue;
                }

                int idx = 0;
                collect_stats(root, stats, &idx);

                qsort(stats, total_words, sizeof(WordStat), compare_stats);

                printf("\n--- Top %d Palavras ---\n", n);
                for (int i = 0; i < n && i < total_words; i++) {
                    printf("%d. %s (%d vezes)\n", i + 1, stats[i].word, stats[i].count);
                }
                free(stats);
            }
        }
        else if (option == 3) {
            char* longest = NULL;
            char* shortest = NULL;
            tree_find_extremes(root, &longest, &shortest);
            if (longest) printf("Mais longa: %s\n", longest);
            if (shortest) printf("Mais curta: %s\n", shortest);
        }
        else if (option == 4) {
            printf("\n--- Todas as palavras (Ordem Alfabetica) ---\n");
            tree_dfs_inorder(root);
        }
        else if (option == 5) {
            char bin_file[100];
            printf("Nome do arquivo binario para salvar: ");
            if (fgets(bin_file, sizeof(bin_file), stdin) != NULL) {
                int len = my_strlen(bin_file);
                if (len > 0 && bin_file[len-1] == '\n') bin_file[len-1] = '\0';

                FILE* out = fopen(bin_file, "wb");
                if (out) {
                    tree_save_binary(root, out);
                    fclose(out);
                    printf("Arvore salva com sucesso.\n");
                } else {
                    printf("Erro ao criar arquivo.\n");
                }
            }
        }
        else if (option == 6) {
            char bin_file[100];
            printf("Nome do arquivo binario para carregar: ");
            if (fgets(bin_file, sizeof(bin_file), stdin) != NULL) {
                int len = my_strlen(bin_file);
                if (len > 0 && bin_file[len-1] == '\n') bin_file[len-1] = '\0';

                FILE* in = fopen(bin_file, "rb");
                if (in) {
                    tree_free(root);
                    root = NULL;
                    tree_load_binary(&root, in);
                    fclose(in);
                    printf("Arvore carregada com sucesso.\n");
                } else {
                    printf("Erro ao abrir arquivo.\n");
                }
            }
        }
    }

    tree_free(root);
    printf("Saindo...\n");
    return 0;
}
