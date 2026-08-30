#ifndef FILA
#define FILA

// descreve um nodo da fila 
struct fila_nodo_t
{
	int item;					// item associado ao nodo
	struct fila_nodo_t *prox;	// próximo nodo
};

// descreve uma fila 
struct fila_t
{
	struct fila_nodo_t *prim ;	// primeiro nodo da fila
	struct fila_nodo_t *ult ;	// último nodo da fila
	int num ;					// número de itens na fila
};

// Cria uma fila vazia.
// Retorno: ponteiro para a fila criada ou NULL se erro.
struct fila_t *fila_cria ();

// Libera todas as estruturas de dados da fila, inclusive os itens.
// Retorno: NULL.
struct fila_t *fila_destroi (struct fila_t *f);

// Insere um item no final da fila (politica FIFO).
// Retorno: 1 se tiver sucesso ou 0 se falhar.
int fila_insere (struct fila_t *f, int item);

// Retira o primeiro item da fila e o devolve
// Retorno 1 se a operação foi bem sucedida e 0 caso contrário
int fila_retira (struct fila_t *f, int *item);

// Informa o número de itens na fila.
// Retorno: N >= 0 ou -1 se erro.
int fila_tamanho (struct fila_t *f);

// Imprime o conteúdo da fila 
void fila_imprime (struct fila_t *f);

#endif
