/*
================================================================================
ENUNCIADO / ЗАДАНИЕ
================================================================================
[RUSSO]
8. Реализуйте программу для работы с многочленами (коэффициенты - целые
числа). Реализуйте функции сложения, вычитания, умножения, целочисленного
деления, остатка от деления, вычисления в точке, производной и композиции.
Программа должна обрабатывать текстовый файл с инструкциями (Add, Sub, Mult,
Div, Mod, Eval, Diff, Cmps). Если параметр один, используется значение
сумматора. Начальное значение сумматора — 0.

[PORTUGUÊS]
8. Implemente um programa para trabalhar com polinômios (coeficientes inteiros).
- Funções: soma, subtração, multiplicação, divisão inteira, resto, avaliação,
  derivada e composição.
- O programa deve processar um arquivo de texto com instruções (Add, Sub, Mult,
  Div, Mod, Eval, Diff, Cmps).
- Se a instrução tiver apenas um parâmetro, o primeiro termo é o valor do
  acumulador. O valor inicial do acumulador é 0.

RESTRIÇÕES: Sem <string.h>, sem variáveis globais, alocação dinâmica,
tratamento de erros com enum, separação de lógica e I/O.
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// DEFINIÇÃO DE STATUS CODES E ESTRUTURAS
// ============================================================================
typedef enum {
    STATUS_OK = 0,
    STATUS_ERR_MEMORY,
    STATUS_ERR_NULL_PTR,
    STATUS_ERR_FILE_OPEN,
    STATUS_ERR_INVALID_POLY,
    STATUS_ERR_DIV_BY_ZERO
} StatusCode;

typedef struct {
    int* coeffs; // Array de coeficientes (índice = expoente)
    int degree;  // Grau do polinômio (maior expoente com coeficiente != 0)
} Polynomial;

// ============================================================================
// FUNÇÕES AUXILIARES (String e Polinômios)
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

// Cria um polinômio vazio (grau -1 significa zero)
StatusCode poly_init(Polynomial* p, int max_degree) {
    if (p == NULL) return STATUS_ERR_NULL_PTR;
    p->degree = max_degree;
    p->coeffs = (int*)calloc(max_degree + 1, sizeof(int));
    if (p->coeffs == NULL) return STATUS_ERR_MEMORY;
    return STATUS_OK;
}

void poly_free(Polynomial* p) {
    if (p && p->coeffs) {
        free(p->coeffs);
        p->coeffs = NULL;
        p->degree = -1;
    }
}

// Imprime o polinômio de forma legível
void poly_print(const Polynomial* p) {
    if (p == NULL || p->degree == -1) {
        printf("0");
        return;
    }
    int first = 1;
    for (int i = p->degree; i >= 0; i--) {
        if (p->coeffs[i] == 0) continue;

        if (!first && p->coeffs[i] > 0) printf("+");
        if (p->coeffs[i] == -1 && i != 0) printf("-");
        else if (p->coeffs[i] != 1 || i == 0) printf("%d", p->coeffs[i]);

        if (i > 0) printf("x");
        if (i > 1) printf("^%d", i);
        first = 0;
    }
    if (first) printf("0");
}

// Ajusta o grau após operações (remove zeros à esquerda)
static void poly_trim(Polynomial* p) {
    while (p->degree >= 0 && p->coeffs[p->degree] == 0) {
        p->degree--;
    }
}

// Parsing manual de string para polinômio (ex: "2x^2-x+2")
StatusCode poly_parse(const char* str, Polynomial* out_p) {
    if (str == NULL || out_p == NULL) return STATUS_ERR_NULL_PTR;

    // Encontra o maior expoente (procurando por '^')
    int max_deg = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '^') {
            int d = 0; int j = i + 1;
            while (str[j] >= '0' && str[j] <= '9') { d = d * 10 + (str[j] - '0'); j++; }
            if (d > max_deg) max_deg = d;
        } else if (str[i] == 'x' && max_deg < 1) {
            max_deg = 1;
        }
    }

    poly_init(out_p, max_deg);
    out_p->degree = max_deg;

    int i = 0;
    int sign = 1;
    while (str[i] != '\0') {
        if (str[i] == '+') { sign = 1; i++; }
        else if (str[i] == '-') { sign = -1; i++; }

        int coeff = 0;
        int has_coeff = 0;
        while (str[i] >= '0' && str[i] <= '9') {
            coeff = coeff * 10 + (str[i] - '0');
            i++; has_coeff = 1;
        }

        int exp = 0;
        if (str[i] == 'x') {
            i++;
            if (str[i] == '^') {
                i++;
                while (str[i] >= '0' && str[i] <= '9') { exp = exp * 10 + (str[i] - '0'); i++; }
            } else {
                exp = 1;
            }
        }

        if (has_coeff || str[i-1] == 'x') { // Se tinha número ou era apenas 'x'
            if (!has_coeff) coeff = 1;
            out_p->coeffs[exp] += sign * coeff;
        }
        if (str[i] == '\0') break;
    }
    poly_trim(out_p);
    return STATUS_OK;
}

// ============================================================================
// OPERAÇÕES MATEMÁTICAS
// ============================================================================

// Add: Soma
StatusCode poly_add(const Polynomial* a, const Polynomial* b, Polynomial* res) {
    int max_deg = (a->degree > b->degree) ? a->degree : b->degree;
    poly_init(res, max_deg);
    res->degree = max_deg;
    for (int i = 0; i <= max_deg; i++) {
        int val_a = (i <= a->degree) ? a->coeffs[i] : 0;
        int val_b = (i <= b->degree) ? b->coeffs[i] : 0;
        res->coeffs[i] = val_a + val_b;
    }
    poly_trim(res);
    return STATUS_OK;
}

// Sub: Subtração
StatusCode poly_sub(const Polynomial* a, const Polynomial* b, Polynomial* res) {
    int max_deg = (a->degree > b->degree) ? a->degree : b->degree;
    poly_init(res, max_deg);
    res->degree = max_deg;
    for (int i = 0; i <= max_deg; i++) {
        int val_a = (i <= a->degree) ? a->coeffs[i] : 0;
        int val_b = (i <= b->degree) ? b->coeffs[i] : 0;
        res->coeffs[i] = val_a - val_b;
    }
    poly_trim(res);
    return STATUS_OK;
}

// Mult: Multiplicação
StatusCode poly_mult(const Polynomial* a, const Polynomial* b, Polynomial* res) {
    if (a->degree == -1 || b->degree == -1) {
        poly_init(res, 0); res->degree = -1; return STATUS_OK;
    }
    int max_deg = a->degree + b->degree;
    poly_init(res, max_deg);
    res->degree = max_deg;
    for (int i = 0; i <= a->degree; i++) {
        for (int j = 0; j <= b->degree; j++) {
            res->coeffs[i + j] += a->coeffs[i] * b->coeffs[j];
        }
    }
    poly_trim(res);
    return STATUS_OK;
}

// Eval: Avaliação em um ponto (retorna int)
int poly_eval(const Polynomial* p, int x) {
    if (p->degree == -1) return 0;
    int result = 0;
    for (int i = p->degree; i >= 0; i--) {
        result = result * x + p->coeffs[i];
    }
    return result;
}

// Diff: Derivada
StatusCode poly_diff(const Polynomial* p, Polynomial* res) {
    if (p->degree <= 0) {
        poly_init(res, 0); res->degree = -1; return STATUS_OK;
    }
    poly_init(res, p->degree - 1);
    res->degree = p->degree - 1;
    for (int i = 1; i <= p->degree; i++) {
        res->coeffs[i - 1] = p->coeffs[i] * i;
    }
    poly_trim(res);
    return STATUS_OK;
}

// Div e Mod: Divisão Inteira e Resto (Algoritmo de Divisão Longa)
StatusCode poly_divmod(const Polynomial* a, const Polynomial* b, Polynomial* quotient, Polynomial* remainder) {
    if (b->degree == -1) return STATUS_ERR_DIV_BY_ZERO;

    // Copia 'a' para o resto
    poly_init(remainder, a->degree);
    remainder->degree = a->degree;
    for (int i = 0; i <= a->degree; i++) remainder->coeffs[i] = a->coeffs[i];

    // Se o grau de 'a' for menor que 'b', quociente é 0
    if (a->degree < b->degree) {
        poly_init(quotient, 0); quotient->degree = -1;
        return STATUS_OK;
    }

    int q_deg = a->degree - b->degree;
    poly_init(quotient, q_deg);
    quotient->degree = q_deg;

    int b_lead = b->coeffs[b->degree]; // Coeficiente líder de B

    while (remainder->degree >= b->degree && remainder->degree != -1) {
        int r_lead = remainder->coeffs[remainder->degree];
        if (r_lead % b_lead != 0) break; // Divisão inteira: para se não for divisível

        int coef = r_lead / b_lead;
        int exp = remainder->degree - b->degree;

        quotient->coeffs[exp] = coef;

        for (int i = 0; i <= b->degree; i++) {
            remainder->coeffs[i + exp] -= coef * b->coeffs[i];
        }
        poly_trim(remainder);
    }
    poly_trim(quotient);
    return STATUS_OK;
}

// Cmps: Composição (a(b(x)))
StatusCode poly_compose(const Polynomial* a, const Polynomial* b, Polynomial* res) {
    // Se a é constante
    if (a->degree == 0) {
        poly_init(res, 0);
        res->coeffs[0] = a->coeffs[0];
        return STATUS_OK;
    }

    // Começa com o termo de maior grau de 'a'
    Polynomial result;
    poly_init(&result, 0);
    result.coeffs[0] = a->coeffs[a->degree];

    // Loop de Horner: result = result * b + a[i]
    for (int i = a->degree - 1; i >= 0; i--) {
        Polynomial temp;
        poly_mult(&result, b, &temp);
        poly_free(&result);

        // Soma o coeficiente a[i]
        temp.coeffs[0] += a->coeffs[i];
        poly_trim(&temp);
        result = temp;
    }

    *res = result;
    return STATUS_OK;
}

// ============================================================================
// FUNÇÃO MAIN
// ============================================================================
int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo_instrucoes.txt>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "Erro ao abrir arquivo.\n");
        return 1;
    }

    // Acumulador inicial = 0
    Polynomial accumulator;
    poly_init(&accumulator, 0);
    accumulator.degree = -1;

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        // Ignora comentários e linhas vazias
        if (line[0] == '%' || line[0] == '\n' || line[0] == '[' || line[0] == ']') continue;

        // Encontra a instrução e os parâmetros
        char cmd[10] = {0};
        int i = 0;
        while (line[i] != '(' && line[i] != '\0' && line[i] != ' ') {
            cmd[i] = line[i]; i++;
        }
        if (line[i] != '(') continue;
        i++; // Pula '('

        char arg1[100] = {0}, arg2[100] = {0};
        int arg_idx = 0, part = 0;

        while (line[i] != ')' && line[i] != '\0') {
            if (line[i] == ',') { part = 1; arg_idx = 0; i++; continue; }
            if (part == 0) arg1[arg_idx++] = line[i];
            else arg2[arg_idx++] = line[i];
            i++;
        }

        // Remove espaços em branco dos argumentos (simplificado)
        char clean1[100] = {0}, clean2[100] = {0};
        int k = 0; for(int j=0; arg1[j]!='\0'; j++) if(arg1[j]!=' ') clean1[k++]=arg1[j];
        k = 0; for(int j=0; arg2[j]!='\0'; j++) if(arg2[j]!=' ') clean2[k++]=arg2[j];

        Polynomial p1, p2, res;

        // Lógica do Acumulador: se arg2 está vazio, p1 = accumulator
        if (clean2[0] == '\0') {
            p1 = accumulator;
            // Cria uma cópia para não modificar o original se a função alterar
            poly_init(&p2, 0);
            p2.degree = -1; // p2 será o único argumento
            if (clean1[0] != '\0') poly_parse(clean1, &p2);
        } else {
            poly_parse(clean1, &p1);
            poly_parse(clean2, &p2);
        }

        // Executa o comando
        if (cmd[0] == 'A' && cmd[1] == 'd' && cmd[2] == 'd') { // Add
            poly_add(&p1, &p2, &res);
            poly_free(&accumulator);
            accumulator = res;
            printf("Add: "); poly_print(&accumulator); printf("\n");
        }
        else if (cmd[0] == 'S' && cmd[1] == 'u' && cmd[2] == 'b') { // Sub
            poly_sub(&p1, &p2, &res);
            poly_free(&accumulator);
            accumulator = res;
            printf("Sub: "); poly_print(&accumulator); printf("\n");
        }
        else if (cmd[0] == 'M' && cmd[1] == 'u' && cmd[2] == 'l') { // Mult
            poly_mult(&p1, &p2, &res);
            poly_free(&accumulator);
            accumulator = res;
            printf("Mult: "); poly_print(&accumulator); printf("\n");
        }
        else if (cmd[0] == 'E' && cmd[1] == 'v') { // Eval
            int x = 0; int j = 0;
            while (clean2[j] >= '0' && clean2[j] <= '9') { x = x * 10 + (clean2[j] - '0'); j++; }
            // Se Eval tiver só 1 argumento, o ponto está em clean1
            if (clean2[0] == '\0') { x = 0; j = 0; while(clean1[j]>='0'&&clean1[j]<='9'){x=x*10+(clean1[j]-'0');j++;} }

            int val = poly_eval(&accumulator, x);
            printf("Eval em %d: %d\n", x, val);
        }
        else if (cmd[0] == 'D' && cmd[1] == 'i') { // Div
            Polynomial q, r;
            if (poly_divmod(&p1, &p2, &q, &r) == STATUS_OK) {
                poly_free(&accumulator);
                accumulator = q;
                printf("Div: "); poly_print(&accumulator); printf("\n");
                poly_free(&q); poly_free(&r);
            }
        }
        else if (cmd[0] == 'D' && cmd[1] == 'i' && cmd[2] == 'f') { // Diff
            poly_diff(&p1, &res);
            poly_free(&accumulator);
            accumulator = res;
            printf("Diff: "); poly_print(&accumulator); printf("\n");
        }

        // Limpeza dos temporários
        if (clean2[0] != '\0') { poly_free(&p1); poly_free(&p2); }
        else { poly_free(&p2); }
    }

    poly_free(&accumulator);
    fclose(file);
    return 0;
}
