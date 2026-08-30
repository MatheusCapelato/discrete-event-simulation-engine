#include <stdio.h>
#include <stdlib.h>
#include "fila.h"

#include <stdio.h>
#include <stdlib.h>
#include "fila.h"

struct fila_t *fila_cria () {
        struct fila_t *f = malloc(sizeof(struct fila_t));
        if (f == NULL) return NULL;
    
        f->prim = NULL;
        f->ult = NULL;
        f->num = 0;
    
        return f;
}

int fila_insere (struct fila_t *f, int item) {
    if (f == NULL) return 0;
    
    /* Cria o novo nodo (o novo herói na fila) */
    struct fila_nodo_t *novo = malloc(sizeof(struct fila_nodo_t));
    if (novo == NULL) return 0;
    
    novo->item = item;
    novo->prox = NULL; /* Como ele vai para o final, não tem ninguém atrás dele */
    
    /* CASO A: A fila estava vazia */
    if (f->prim == NULL) {
        f->prim = novo;
    } 
    /* CASO B: Já tinha gente na fila */
    else {
        f->ult->prox = novo; /* O antigo último agora aponta para o recém-chegado */
    }
    
    /* O painel da fila atualiza quem é o novo "último" */
    f->ult = novo;
    f->num++;
    
    return 1;
}

int fila_retira (struct fila_t *f, int *item) {
    if (f == NULL || f->prim == NULL || item == NULL) return 0;
    
    /* Isola o primeiro da fila para ser retirado */
    struct fila_nodo_t *lixo = f->prim;
    
    /* Salva o valor do item (o ID do herói) no ponteiro fornecido */
    *item = lixo->item;
    
    /* O painel da fila agora aponta para o segundo da fila */
    f->prim = lixo->prox;
    
    /* ATENÇÃO (Para a apresentação): Se a fila ficou vazia após retirar, 
       o ponteiro 'ult' também precisa ser resetado! */
    if (f->prim == NULL) {
        f->ult = NULL;
    }
    
    free(lixo);
    f->num--;
    
    return 1;
}

int fila_tamanho (struct fila_t *f) {
    if (f == NULL) return -1;
    return f->num;
}

void fila_imprime (struct fila_t *f) {
    if (f == NULL) return;
    
    struct fila_nodo_t *aux = f->prim;
    
    printf("[ ");
    while (aux != NULL) {
        printf("%2d ", aux->item);
        aux = aux->prox;
    }
    printf("]\n");
}

struct fila_t *fila_destroi (struct fila_t *f) {
    if (f == NULL) return NULL;
    
    struct fila_nodo_t *aux = f->prim;
    struct fila_nodo_t *lixo;
    
    /* Explode todos os nodos um por um */
    while (aux != NULL) {
        lixo = aux;
        aux = aux->prox;
        free(lixo);
    }
    
    /* Explode a estrutura principal da fila */
    free(f);
    
    return NULL;
}