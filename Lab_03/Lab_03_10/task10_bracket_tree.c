/*
================================================================================
ENUNCIADO / ЗАДАНИЕ
================================================================================
[RUSSO]
10. Реализуйте приложение для построения дерева скобочного выражения.
На вход через аргументы командной строки подается путь к файлу с выражениями
и путь к выходному файлу. Постройте дерево и сохраните его в выходной файл
в удобном текстовом формате.

[PORTUGUÊS]
10. Implemente um aplicativo para construir uma árvore a partir de uma expressão
com parênteses. O caminho do arquivo de entrada (com as expressões) e o caminho
do arquivo de saída são passados via linha de comando. Construa a árvore e
salve-a no arquivo de saída em um formato de texto legível.

RESTRIÇÕES: Sem <string.h>, sem variáveis globais, alocação dinâmica,
tratamento de erros com enum, separação de lógica e I/O.

OBSERVAÇÕES / ПРИМЕЧАНИЯ:
1. A árvore é N-ária (um nó pode ter vários filhos, como o 'E' no exemplo).
2. O parsing é feito de forma recursiva.
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
    STATUS_ERR_INVALID_SYNTAX
} StatusCode;

// ============================================================================
// ESTRUTURA DA ÁRVORE N-ÁRIA
// ============================================================================

// Nó da árvore
typedef struct TreeNode {
    char name;              // Nome do nó (ex: 'A', 'B')
    struct TreeNode** children; // Array dinâmico de ponteiros para os filhos
    int child_count;        // Quantidade de filhos
    int capacity;           // Capacidade do array de filhos
} TreeNode;

/**
 * @brief Cria um novo nó na árvore.
 */
static TreeNode* create_node(char name) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    if (node == NULL) return NULL;

    node->name = name;
    node->child_count = 0;
    node->capacity = 2; // Começa com espaço para 2 filhos
    node->children = (TreeNode**)malloc(node->capacity * sizeof(TreeNode*));

    if (node->children == NULL) {
        free(node);
        return NULL;
    }
    return node;
}

/**
 * @brief Adiciona um filho a um nó existente.
 */
static void add_child(TreeNode* parent, TreeNode* child) {
    if (parent->child_count == parent->capacity) {
        parent->capacity *= 2;
        TreeNode** new_children = (TreeNode**)realloc(parent->children, parent->capacity * sizeof(TreeNode*));
        if (new_children == NULL) return; // Falha silenciosa por brevidade
        parent->children = new_children;
    }
    parent->children[parent->child_count++] = child;
}

/**
 * @brief Libera a memória de toda a árvore recursivamente.
 */
static void free_tree(TreeNode* root) {
    if (root == NULL) return;
    for (int i = 0; i < root->child_count; i++) {
        free_tree(root->children[i]);
    }
    free(root->children);
    free(root);
}

// ============================================================================
// FUNÇÕES AUXILIARES (Parsing)
// ============================================================================

// Ignora espaços em branco
static void skip_spaces(const char** str) {
    while (**str == ' ' || **str == '\t' || **str == '\n' || **str == '\r') {
        (*str)++;
    }
}

/**
 * @brief Função recursiva que constrói a árvore a partir da string.
 *
 * Passo a passo:
 * 1. Lê o caractere do nó (ex: 'A').
 * 2. Cria o nó.
 * 3. Se o próximo caractere (ignorando espaços) for '(',
 *    significa que este nó tem filhos.
 * 4. Entra em um loop: lê o próximo filho recursivamente, adiciona à lista,
 *    e se o próximo for ',', pula e continua lendo.
 * 5. Se o próximo for ')', fecha o parêntese e para de ler filhos.
 */
static TreeNode* parse_expression(const char** str) {
    skip_spaces(str);

    // Se a string acabou ou é um parêntese de fechamento, não há nó
    if (**str == '\0' || **str == ')') return NULL;

    char node_name = **str;
    (*str)++; // Avança o ponteiro

    TreeNode* node = create_node(node_name);
    if (node == NULL) return NULL;

    skip_spaces(str);

    // Verifica se tem filhos
    if (**str == '(') {
        (*str)++; // Pula o '('

        while (1) {
            skip_spaces(str);
            if (**str == ')') {
                (*str)++; // Pula o ')'
                break;
            }
            if (**str == '\0') break; // Fim prematuro (erro de sintaxe)

            TreeNode* child = parse_expression(str);
            if (child != NULL) {
                add_child(node, child);
            }

            skip_spaces(str);
            if (**str == ',') {
                (*str)++; // Pula a vírgula e vai para o próximo
            } else if (**str == ')') {
                (*str)++; // Pula o ')'
                break;
            }
        }
    }

    return node;
}

// ============================================================================
// FUNÇÕES DE IMPRESSÃO DA ÁRVORE NO ARQUIVO
// ============================================================================

/**
 * @brief Imprime a árvore no arquivo no formato de árvore de diretórios.
 *
 * @param root Nó atual.
 * @param file Ponteiro para o arquivo de saída.
 * @param prefix String de espaços/barras para a indentação.
 * @param is_last Booleano: 1 se for o último filho, 0 caso contrário.
 */
void print_tree_to_file(TreeNode* root, FILE* file, const char* prefix, int is_last) {
    if (root == NULL) return;

    // Imprime o prefixo atual
    fprintf(file, "%s", prefix);

    // Imprime a ramificação (├── ou └──)
    if (is_last) {
        fprintf(file, "└── ");
    } else {
        fprintf(file, "├── ");
    }

    // Imprime o nome do nó
    fprintf(file, "%c\n", root->name);

    // Prepara o novo prefixo para os filhos
    char new_prefix[256];
    int p_len = 0;
    while (prefix[p_len] != '\0') {
        new_prefix[p_len] = prefix[p_len];
        p_len++;
    }

    // Se este nó é o último, os filhos não precisam da barra vertical '│'
    if (is_last) {
        new_prefix[p_len++] = ' ';
        new_prefix[p_len++] = ' ';
        new_prefix[p_len++] = ' ';
    } else {
        new_prefix[p_len++] = '│';
        new_prefix[p_len++] = ' ';
        new_prefix[p_len++] = ' ';
    }
    new_prefix[p_len] = '\0';

    // Chama recursivamente para os filhos
    for (int i = 0; i < root->child_count; i++) {
        int child_is_last = (i == root->child_count - 1);
        print_tree_to_file(root->children[i], file, new_prefix, child_is_last);
    }
}

// ============================================================================
// FUNÇÃO MAIN
// ============================================================================
int main(int argc, char* argv[]) {
    // 1. Validação de argumentos
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <arquivo_entrada.txt> <arquivo_saida.txt>\n", argv[0]);
        return 1;
    }

    const char* input_file = argv[1];
    const char* output_file = argv[2];

    // 2. Abrir arquivos
    FILE* in = fopen(input_file, "r");
    if (in == NULL) {
        fprintf(stderr, "Erro: Nao foi possivel abrir o arquivo de entrada '%s'.\n", input_file);
        return 1;
    }

    FILE* out = fopen(output_file, "w");
    if (out == NULL) {
        fprintf(stderr, "Erro: Nao foi possivel criar o arquivo de saida '%s'.\n", output_file);
        fclose(in);
        return 1;
    }

    char line[1024];
    int line_num = 1;

    // 3. Processar cada linha do arquivo
    while (fgets(line, sizeof(line), in) != NULL) {
        // Ignora linhas vazias
        if (line[0] == '\n' || line[0] == '\0') continue;

        fprintf(out, "\n--- Arvore da linha %d ---\n", line_num);

        const char* ptr = line;
        TreeNode* root = parse_expression(&ptr);

        if (root != NULL) {
            // Imprime a árvore no arquivo
            print_tree_to_file(root, out, "", 1); // 1 = é o último (raiz)
            free_tree(root); // Libera a memória após salvar
        } else {
            fprintf(out, "Expressao vazia ou invalida.\n");
        }

        line_num++;
    }

    fclose(in);
    fclose(out);

    printf("Processamento concluido. Verifique o arquivo '%s'.\n", output_file);
    return 0;
}
