#include "fprio.h"
#include <stdio.h>
#include <stdlib.h>

struct fprio_t *fprio_cria () {
      struct fprio_t *primeiro = malloc(sizeof(struct fprio_t));

      if (primeiro == NULL)
        return NULL;

      primeiro->num = 0;
      primeiro->prim = NULL;

      return primeiro;
}


struct fprio_t *fprio_destroi (struct fprio_t *f) {
      if (f == NULL)
        return NULL;
      
      struct fpnodo_t *aux = f->prim;
      struct fpnodo_t *nodo_destruir;

      while (aux != NULL) {
        nodo_destruir = aux;
        aux = aux->prox;

        free(nodo_destruir->item);
        free(nodo_destruir);
      }

      free(f);
      return NULL;
}


int fprio_insere (struct fprio_t *f, void *item, int tipo, int prio) {
      if (f == NULL || item == NULL)
        return -1;
      
      struct fpnodo_t *aux = f->prim;

      while (aux != NULL) {
        if (aux->item == item)
          return -1;
        aux = aux->prox;
      }

      struct fpnodo_t *nodo_insere = malloc(sizeof(struct fpnodo_t));

      if (nodo_insere == NULL)
        return -1;
      
      nodo_insere->item = item;
      nodo_insere->prio = prio;
      nodo_insere->tipo = tipo;

      if (f->prim == NULL || prio < f->prim->prio) {
        nodo_insere->prox = f->prim;
        f->prim = nodo_insere;
        
        f->num++;
        return f->num;
      }

      aux = f->prim;
      
      while (aux->prox != NULL && aux->prox->prio <= prio)
        aux = aux->prox;

      nodo_insere->prox = aux->prox;
      aux->prox = nodo_insere;

      f->num++;
      return f->num;
}


void *fprio_retira (struct fprio_t *f, int *tipo, int *prio) {
      if (f == NULL || tipo == NULL || prio == NULL || f->prim == NULL)
        return NULL;

      struct fpnodo_t *nodo_retirado = f->prim;

      void *item_salvo = nodo_retirado->item;
      *tipo = nodo_retirado->tipo;
      *prio = nodo_retirado->prio;

      f->prim = nodo_retirado->prox;
      f->num--;

      free(nodo_retirado);
      return item_salvo;
}


int fprio_tamanho (struct fprio_t *f) {
      if (f == NULL)
        return -1;
      
      return f->num;
}


void fprio_imprime (struct fprio_t *f) {
      if (f == NULL || f->prim == NULL)
        return;
      
      struct fpnodo_t *aux = f->prim;
      
      while (aux != NULL) {
          printf("(%d %d)", aux->tipo, aux->prio);
          
          /* Só imprime o espaço se não for o último vagão */
          if (aux->prox != NULL)
              printf(" ");
          aux = aux->prox;
      }
}
