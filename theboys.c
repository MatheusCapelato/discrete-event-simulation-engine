#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "conjunto.h"
#include "fila.h"
#include "fprio.h"

#define T_INICIO 0
#define T_FIM_DO_MUNDO 525600
#define N_TAMANHO_MUNDO 20000
#define N_HABILIDADES 10
#define N_HEROIS (N_HABILIDADES * 5)
#define N_BASES (N_HEROIS / 5)
#define N_MISSOES (T_FIM_DO_MUNDO / 100)
#define N_COMPOSTOS_V (N_HABILIDADES * 3)

#define EV_CHEGA 1
#define EV_ESPERA 2
#define EV_DESISTE 3
#define EV_AVISA 4
#define EV_ENTRA 5
#define EV_SAI 6
#define EV_VIAJA 7
#define EV_MORRE 8
#define EV_MISSAO 9
#define EV_FIM 10

struct heroi {
    int id;               // ID do herói
    int experiencia;      // Pontos de experiência ganhos em missões
    int paciencia;        // Determina se o herói espera na fila ou desiste
    int velocidade;       // Velocidade de deslocamento (metros/minuto)
    int id_base_atual;    // ID da base onde se encontra
    bool vivo;            // Controle do uso do Composto V
    struct cjto_t *habilidades; // Conjunto de habilidades
};

struct base {
    int id;               // ID da base
    int lotacao_max;      // Capacidade máxima de heróis presentes
    int local_x;          // Coordenada X
    int local_y;          // Coordenada Y
    int missoes_participadas;   // numero de missoes que a base participou
    int max_espera;         // maximo para espera
    struct cjto_t *presentes; // Heróis atualmente dentro da base
    struct fila_t *espera;      // Fila de heróis aguardando para entrar
};

struct missao {
    int id;               // ID da missão
    int local_x;          // Coordenada X
    int local_y;          // Coordenada Y
    int tentativas;       // numero de tentivas na missao
    bool cumprida;        // controle para verificar se a missao foi cumprida ou nao
    struct cjto_t *habilidades; // Habilidades exigidas para cumprimento
};

struct mundo {
    int tempo_atual;      // O relógio global da simulação
    int n_herois;         // Total de heróis
    int n_bases;          // Total de bases
    int n_missoes;        // Total de missões
    int n_habilidades;    // Habilidades distintas possíveis
    int n_compostos_v;    // Total de Compostos V
    int tamanho_mundo;    // Coordenadas máximas (plano cartesiano)
    
    struct heroi *herois; // Vetor contendo todos os heróis
    struct base *bases;   // Vetor contendo todas as bases
    struct missao *missoes; // Vetor contendo todas as missões
    
    struct fprio_t *lef;  // Lista de Eventos Futuros
};

struct evento {
    int id_heroi;   // ID do heroi
    int id_base;    //ID da base
    int id_missao;  //ID da missão
};

// Gera um número inteiro aleatório entre min e max (inclusive)
int aleatorio(int min, int max) {
    return min + rand() % (max - min + 1);
}

// Calcula a distancia cartesiana entre dois pontos do mapa
int calcula_distancia(int x1, int y1, int x2, int y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    return (int) sqrt(dx * dx + dy * dy);
}

// "evento" cria mundo
struct mundo *cria_mundo() {
    int i, hab;

    //inicia ponteiros e areas dos dados
    struct mundo *m = malloc(sizeof(struct mundo));
    if (m == NULL) return NULL;

    m->herois = malloc(sizeof(struct heroi) * N_HEROIS);
    if (m->herois == NULL) return NULL;

    m->bases = malloc(sizeof(struct base) * N_BASES);
    if (m->bases == NULL) return NULL;

    m->missoes = malloc(sizeof(struct missao) * N_MISSOES);
    if (m->missoes == NULL) return NULL;

    //coloca os dados fixos em seus respectivos lugares
    m->tempo_atual = T_INICIO;
    m->n_herois = N_HEROIS;
    m->n_bases = N_BASES;
    m->n_missoes = N_MISSOES;
    m->n_habilidades = N_HABILIDADES;
    m->n_compostos_v = N_COMPOSTOS_V;
    m->tamanho_mundo = N_TAMANHO_MUNDO;

    // cria a fila de eventos futuros
    m->lef = fprio_cria();

    //inicia dados dos herois
    for (int i = 0; i < N_HEROIS; i++) {
        m->herois[i].id = i;
        m->herois[i].experiencia = 0;
        m->herois[i].paciencia = aleatorio(0, 100);
        m->herois[i].velocidade = aleatorio(50, 5000);
        m->herois[i].vivo = true;

        m->herois[i].id_base_atual = -1;

        m->herois[i].habilidades = cjto_cria(N_HABILIDADES);
        int quantidade_hab = aleatorio(1, 3);

        while(cjto_card(m->herois[i].habilidades) < quantidade_hab) {
            hab = aleatorio(0, N_HABILIDADES - 1);
            cjto_insere(m->herois[i].habilidades, hab);
        }
    }

    // inicia dados das bases
    for (i = 0; i < N_BASES; i++) {
        m->bases[i].id = i;
        m->bases[i].local_x = aleatorio(0, N_TAMANHO_MUNDO - 1);
        m->bases[i].local_y = aleatorio(0, N_TAMANHO_MUNDO - 1);
        m->bases[i].lotacao_max = aleatorio(3, 10);
        m->bases[i].missoes_participadas = 0;
        m->bases[i].max_espera = 0;

        m->bases[i].presentes = cjto_cria(N_HEROIS);
        m->bases[i].espera = fila_cria();
    }

    // inicia dados das missoes
    for (i = 0; i < N_MISSOES; i++) {
        m->missoes[i].id = i;
        m->missoes[i].local_x = aleatorio(0, N_MISSOES - 1);
        m->missoes[i].local_y = aleatorio(0, N_MISSOES - 1);
        m->missoes[i].habilidades = cjto_cria(N_HABILIDADES);
        m->missoes[i].tentativas = 0;

        m->missoes[i].cumprida = false;

        int quantidade_hab = aleatorio(6,10);

        while (cjto_card(m->missoes[i].habilidades) < quantidade_hab) {
            hab = aleatorio(0, N_HABILIDADES - 1);
            cjto_insere(m->missoes[i].habilidades, hab);
        }
    }

    return m;
}

// "evento" inicia evento: coloca os eventos iniciais na lef para a simulacao comecar
void inicia_evento(struct mundo *m) {
    int i;

    // ESPALHANDO OS HERÓIS PELO MAPA
    // agenda a primeira chegada de cada herói em uma base aleatória
    // ao longo dos 3 primeiros dias de simulação (3 dias * 24h * 60m = 4320)
    for (i = 0; i < m->n_herois; i++) {
        struct evento *event = malloc(sizeof(struct evento));
        if (event == NULL) return;

        event->id_heroi = i;
        event->id_base = aleatorio(0, m->n_bases - 1);
        event->id_missao = -1;

        int temp = aleatorio(0, 4320);

        fprio_insere(m->lef, event, EV_CHEGA, temp);
    }

    // O CALENDÁRIO DE MISSÕES
    // sorteia o exato minuto em que cada missão do ano vai aparecer no mapa
    // elas são distribuídas do minuto 0 até o último minuto do ano (525.600)
    for (i = 0; i < m->n_missoes; i++) {
        struct evento *ev = malloc(sizeof(struct evento));
        if (ev == NULL) return;

        ev->id_heroi = -1;
        ev->id_base = -1;
        ev->id_missao = i;

        int temp = aleatorio(0, T_FIM_DO_MUNDO);
        fprio_insere(m->lef, ev, EV_MISSAO, temp);
    }
    
    // O RELÓGIO DO FIM DA SIMULAÇÃO
    // Crava o evento FIM no final da fila para garantir que a simulação
    // pare e imprima as estatísticas exatas no minuto 525.600
    struct evento *event_fim = malloc(sizeof(struct evento));
    if (event_fim == NULL) return;
    event_fim->id_heroi = -1;
    event_fim->id_base = -1;
    event_fim->id_missao = -1;
    fprio_insere(m->lef, event_fim, EV_FIM, T_FIM_DO_MUNDO);
}

// evento chega
void chega(int tempo, int id_heroi, int id_base, struct mundo *m) {
    // O Terreno
    // Cria atalhos para os vetores para ficar mais facil de ler
    struct heroi *h = &m->herois[id_heroi];
    struct base *b = &m->bases[id_base];
    
    // Atualiza a base atual do heroi
    h->id_base_atual = id_base;
    
    // Variaveis que usadas para pensar
    bool espera = false; // espera inicia como false
    int tamanho_fila = fila_tamanho(b->espera); // tamanho da fila de espera da base
    int presentes_na_base = cjto_card(b->presentes); // conjunto de herois presentes na base
    int vagas = b->lotacao_max - presentes_na_base; // vagas que tem na base
    
    // A Inteligencia Artificial do Heroi
    // heroi verifica tamanho da fila
    // se nao tiver fila e tiver vaga na base o heroi entra direto na base
    if (vagas > 0 && tamanho_fila == 0) {
        espera = true;
    } else {
        // senao a paciencia do heroi deve ser pelo menos 10 vezes o tamanho da fila
        espera = ((h->paciencia) > (10 * tamanho_fila));
    }
    
    //A Encruzilhada do Destino
    // Cria a struct do evento futuro
    struct evento *event = malloc(sizeof(struct evento));
    if (event == NULL) return;
    event->id_heroi = id_heroi;
    event->id_base = id_base;
    event->id_missao = -1; // <-- Não é uma missao
    
    if (espera) {
        // Imprime que decidiu esperar e joga na LEF
        printf("%6d: CHEGA HEROI %2d BASE %d (%2d/%2d) ESPERA\n", tempo, id_heroi, id_base, presentes_na_base, b->lotacao_max);
        fprio_insere(m->lef, event, EV_ESPERA, tempo);
    } else {
        printf("%6d: CHEGA HEROI %2d BASE %d (%2d/%2d) DESISTE\n", tempo, id_heroi, id_base, presentes_na_base, b->lotacao_max);
        fprio_insere(m->lef, event, EV_DESISTE, tempo); // <-- senão imprime que o heroi desistiu e inicia evento desiste
    }
}

void espera(int tempo, int id_heroi, int id_base, struct mundo *m) {
    // Achando a base
    struct base *b = &m->bases[id_base];
    
    // Coleta o tamanho da fila antes do heroi entrar para o print
    int tam_fila = fila_tamanho(b->espera);
    
    // Imprime a acao e insere o heroi fisicamente na Fila
    printf("%6d: ESPERA HEROI %2d BASE %d (%2d)\n", tempo, id_heroi, id_base, tam_fila);
    fila_insere(b->espera, id_heroi);

    int tam_novo = fila_tamanho(b->espera);
    if (tam_novo > b->max_espera) {
        b->max_espera = tam_novo;
    }
    
    // inicia ponteiro da base
    struct evento *event = malloc(sizeof(struct evento));
    if (event == NULL) return;
    event->id_heroi = -1; // <-- Porteiro não tem ID de herói
    event->id_base = id_base;
    event->id_missao = -1;
    
    // Insere o evento do porteiro na LEF para acontecer agora
    fprio_insere(m->lef, event, EV_AVISA, tempo);
}

// evento avisa
void avisa(int tempo, int id_base, struct mundo *m) {
    // Achando a base
    struct base *b = &m->bases[id_base];
    
    // Coleta os dados de lotação
    int presentes = cjto_card(b->presentes);
    int vagas = b->lotacao_max - presentes;
    
    // Imprime o status inicial do porteiro e a fila inteira
    printf("%6d: AVISA PORTEIRO BASE %d (%2d/%2d) FILA ", tempo, id_base, presentes, b->lotacao_max);
    fila_imprime(b->espera);
    
    // laço que deixa os herois entrarem na base
    // enquanto tiver vaga na base e tiver alguem na fila
    while (vagas > 0 && fila_tamanho(b->espera) > 0) {
        
        int id_heroi_admitido;
        
        // Tira o primeiro da fila e salva o ID dele na variavel
        fila_retira(b->espera, &id_heroi_admitido);
        
        // Imprime que ele foi admitido
        printf("%6d: AVISA PORTEIRO BASE %d ADMITE %2d\n", tempo, id_base, id_heroi_admitido);
        
        // coloca o heroi dentro da base
        cjto_insere(b->presentes, id_heroi_admitido);
        
        // Prepara a caixinha do evento ENTRA para esse heroi
        struct evento *ev = malloc(sizeof(struct evento));
        if (ev == NULL) return;
        ev->id_heroi = id_heroi_admitido;
        ev->id_base = id_base;
        ev->id_missao = -1;
        fprio_insere(m->lef, ev, EV_ENTRA, tempo);
        
        // Atualiza a contagem de vagas para o laço while saber se ainda pode puxar o próximo
        presentes = cjto_card(b->presentes);
        vagas = b->lotacao_max - presentes;
    }
}

// evento sai
void entra(int tempo, int id_heroi, int id_base, struct mundo *m) {
    // O Terreno
    struct heroi *h = &m->herois[id_heroi];
    struct base *b = &m->bases[id_base];
    
    // Coleta os dados para o print
    int presentes = cjto_card(b->presentes);
    
    // Calcula o tempo que ele vai ficar na base (TPB)
    int TPB = 15 + h->paciencia * aleatorio(1, 20);
    int tempo_saida = tempo + TPB;
    
    // Imprime que ele entrou e já avisa quando vai sair
    printf("%6d: ENTRA HEROI %2d BASE %d (%2d/%2d) SAI %d\n", tempo, id_heroi, id_base, presentes, b->lotacao_max, tempo_saida);
    
    // agenda o evento de SAIDA dele
    struct evento *event = malloc(sizeof(struct evento));
    if (event == NULL) return;
    event->id_heroi = id_heroi;
    event->id_base = id_base;
    event->id_missao = -1;
    
    fprio_insere(m->lef, event, EV_SAI, tempo_saida);
}

// evento sai
void sai(int tempo, int id_heroi, int id_base, struct mundo *m) {
    // O Terreno
    struct base *b = &m->bases[id_base];
    
    // Tira o heroi de dentro da base (libera a vaga)
    cjto_retira(b->presentes, id_heroi);
    
    // Coleta os dados novos para o print
    int presentes = cjto_card(b->presentes);
    
    // Imprime a saida do heroi: [tempo]: sai heroi [heroi] base [base do heroi] ([quantidade de heroi na base]/[locação maxima da base])
    printf("%6d: SAI HEROI %2d BASE %d (%2d/%2d)\n", tempo, id_heroi, id_base, presentes, b->lotacao_max);
    
    // escolhe a viagem de forma aleatoria
    int base_destino = aleatorio(0, m->n_bases - 1);
    
    struct evento *event_viaja = malloc(sizeof(struct evento));
    if (event_viaja == NULL) return;
    event_viaja->id_heroi = id_heroi;
    event_viaja->id_base = base_destino;
    event_viaja->id_missao = -1;
    fprio_insere(m->lef, event_viaja, EV_VIAJA, tempo);
    
    // avisa ponteiro da nova vaga
    struct evento *event_avisa = malloc(sizeof(struct evento));
    if (event_avisa == NULL) return;
    event_avisa->id_heroi = -1;
    event_avisa->id_base = id_base;
    event_avisa->id_missao = -1;
    fprio_insere(m->lef, event_avisa, EV_AVISA, tempo);
}

// evento viaja
void viaja(int tempo, int id_heroi, int id_base_destino, struct mundo *m) {

    // O Terreno
    struct heroi *h = &m->herois[id_heroi];
    struct base *b_origem = &m->bases[h->id_base_atual];
    struct base *b_destino = &m->bases[id_base_destino];
    
    // Calcula a distância cartesiana (Teorema de Pitágoras)
    double dx = b_destino->local_x - b_origem->local_x;
    double dy = b_destino->local_y - b_origem->local_y;
    int distancia = (int) sqrt(dx * dx + dy * dy);
    
    // Calcula a duração da viagem e a hora de chegada
    int duracao = distancia / h->velocidade;
    int tempo_chegada = tempo + duracao;
    
    // Imprime os dados da viagem na tela: [temoo]: viaja heroi [heroi] base [base de origem] base [base de destino] dist [distancia] vel [velocidade] chega [tempo de chegada]
    printf("%6d: VIAJA HEROI %2d BASE %d BASE %d DIST %d VEL %d CHEGA %d\n", 
        tempo, id_heroi, b_origem->id, b_destino->id, distancia, h->velocidade, tempo_chegada);
        
    // agenda a chegada dele na nova base
    struct evento *ev = malloc(sizeof(struct evento));
    if (ev == NULL) return;
    ev->id_heroi = id_heroi;
    ev->id_base = id_base_destino;
    ev->id_missao = -1;
    
    fprio_insere(m->lef, ev, EV_CHEGA, tempo_chegada);
}

// evento desiste
void desiste(int tempo, int id_heroi, int id_base, struct mundo *m) {
    // Imprime na tela tempo X: desiste heroi [id do heroi] base [id da base]
    printf("%6d: DESISTE HEROI %2d BASE %d\n", tempo, id_heroi, id_base);
    
    // escolhe de forma aleatoria
    int base_destino = aleatorio(0, m->n_bases - 1);
    
    // Agenda a viagem dele no mesmo minuto
    struct evento *event = malloc(sizeof(struct evento));
    if (event == NULL) return;
    event->id_heroi = id_heroi;
    event->id_base = base_destino;
    event->id_missao = -1;
    
    fprio_insere(m->lef, event, EV_VIAJA, tempo);
}

// evento missao
void missao(int tempo, int id_missao, struct mundo *m) {
    struct missao *mis = &m->missoes[id_missao];
    
    // Aumenta o contador de tentativas dessa missao
    mis->tentativas++;
    
    // Imprime o anuncio da missao com a moldura de colchetes
    printf("%6d: MISSAO %d TENT %d HAB REQ: [ ", tempo, id_missao, mis->tentativas);
    cjto_imprime(mis->habilidades);
    printf(" ]\n");
    
    int id_base_apta = -1;
    int dist_base_apta = -1;
    
    int id_bmp = -1; // BMP = Base Mais Proxima (Com qualquer quantidade de herois)
    int dist_bmp = -1;
    
    // O Escaneamento do Mapa (Busca pela melhor base)
    for (int i = 0; i < m->n_bases; i++) {
        struct base *b = &m->bases[i];
        int dist = calcula_distancia(mis->local_x, mis->local_y, b->local_x, b->local_y);
        int qtd_presentes = cjto_card(b->presentes);
        
        if (qtd_presentes == 0) continue; // Base sem ninguem é inutil
        
        // Atualiza a Base Mais Proxima global (Necessario para a regra do Composto V)
        if (id_bmp == -1 || dist < dist_bmp) {
            id_bmp = b->id;
            dist_bmp = dist;
        }
        
        // Junta as habilidades de todo mundo da base em um balaio so
        struct cjto_t *hab_base = cjto_cria(m->n_habilidades);
        for (int h = 0; h < m->n_herois; h++) {
            if (cjto_pertence(b->presentes, h)) {
                struct cjto_t *uniao = cjto_uniao(hab_base, m->herois[h].habilidades);
                cjto_destroi(hab_base);
                hab_base = uniao;
            }
        }
        
        // a regra de Ouro: O balaio da base contem TODAS as habilidades da missao
        if (cjto_contem(hab_base, mis->habilidades)) {
            if (id_base_apta == -1 || dist < dist_base_apta) {
                id_base_apta = b->id;
                dist_base_apta = dist;
            }
        }
        cjto_destroi(hab_base); // Limpa a memoria do balaio temporario
    }
    
    // resultado da Batalha
    if (id_base_apta != -1) {
        // VITORIA NORMAL: Achamos uma equipe forte o suficiente
        struct base *b_vencedora = &m->bases[id_base_apta];

        mis->cumprida = true;
        b_vencedora->missoes_participadas++;
        
        // Recalcula as habilidades unidas SÓ para poder imprimir bonito agora
        struct cjto_t *hab_vencedora = cjto_cria(m->n_habilidades);
        for (int h = 0; h < m->n_herois; h++) {
            if (cjto_pertence(b_vencedora->presentes, h)) {
                struct cjto_t *uniao = cjto_uniao(hab_vencedora, m->herois[h].habilidades);
                cjto_destroi(hab_vencedora);
                hab_vencedora = uniao;
            }
        }
        
        printf("%6d: MISSAO %d CUMPRIDA BASE %d HABS: [ ", tempo, id_missao, id_base_apta);
        cjto_imprime(hab_vencedora);
        printf(" ]\n");
        cjto_destroi(hab_vencedora);
        
        // Da os pontos de XP para a equipe
        for (int h = 0; h < m->n_herois; h++) {
            if (cjto_pertence(b_vencedora->presentes, h)) {
                m->herois[h].experiencia++;
            }
        }
        
    } else {
        // FRACASSO: Nenhuma equipe foi capaz de cumprir a missão.
        //  heroi tenta usar o Composto V
        // O sistema avalia se é possível apelar para o Composto V. São 3 regras simultâneas:
        // primeiro if: Ainda temos Composto V no estoque global?
        // segundo if: O tempo atual é um múltiplo de 2500? (Regra de restrição do uso)
        // terceito if: Existe alguma base no mapa que tenha pelo menos 1 herói para se sacrificar? (id_bmp != -1)
        if (m->n_compostos_v > 0 && tempo % 2500 == 0 && id_bmp != -1) {
            m->n_compostos_v--;

            struct base *bmp = &m->bases[id_bmp];

            mis->cumprida = true;
            bmp->missoes_participadas++;
            
            // Como o Composto V da "todas as habilidades" ao heroi, imprimi as da missao para a saida ficar correta
            printf("%6d: MISSAO %d CUMPRIDA BASE %d HABS: [ ", tempo, id_missao, id_bmp);
            cjto_imprime(mis->habilidades);
            printf(" ]\n");
            
            // Acha o heroi mais experiente da BMP para se sacrificar
            // Busca o herói com maior XP para usar o composto.
            // Varre todos os heróis do mundo e filtra apenas os que pertencem a base mais próxima (BMP).
            int id_sacrificio = -1;
            int max_exp = -1;
            
            for (int h = 0; h < m->n_herois; h++) {
                if (cjto_pertence(bmp->presentes, h)) {
                    // Se é o primeiro herói que olhamos (id_sacrificio == -1) 
                    // OU se a XP dele bateu o recorde atual (max_exp), ele vira o novo candidato a sacrifício.
                    if (id_sacrificio == -1 || m->herois[h].experiencia > max_exp) {
                        id_sacrificio = h;
                        max_exp = m->herois[h].experiencia;
                    }
                }
            }
            
            // confirma a morte do heroi
            struct evento *heroi_morto = malloc(sizeof(struct evento));
            if (heroi_morto == NULL) return;
            heroi_morto->id_heroi = id_sacrificio;
            heroi_morto->id_base = id_bmp;
            heroi_morto->id_missao = id_missao;
            fprio_insere(m->lef, heroi_morto, EV_MORRE, tempo);
            
            // Da o XP para a equipe que sobreviveu
            for (int h = 0; h < m->n_herois; h++) {
                if (cjto_pertence(bmp->presentes, h) && h != id_sacrificio) {
                    m->herois[h].experiencia++;
                }
            }
            
        } else {
            // missao deu errado: Missao adiada para o dia seguinte (1440 minutos = 24h * 60) */
            printf("%6d: MISSAO %d IMPOSSIVEL\n", tempo, id_missao);
            struct evento *event_adia = malloc(sizeof(struct evento));
            if (event_adia == NULL) return;
            event_adia->id_heroi = -1;
            event_adia->id_base = -1;
            event_adia->id_missao = id_missao;
            fprio_insere(m->lef, event_adia, EV_MISSAO, tempo + 1440);
        }
    }
}

// evento heroi morto
void morre(int tempo, int id_heroi, int id_missao, struct mundo *m) {
    struct heroi *h = &m->herois[id_heroi];
    
    // O heroi morreu
    h->vivo = false;
    printf("%6d: MORRE HEROI %2d MISSAO %d\n", tempo, id_heroi, id_missao);
    
    // Se ele estava dentro de alguma base, libera a vaga e avisa o porteiro
    int id_base = h->id_base_atual;
    if (id_base != -1) {
        struct base *b = &m->bases[id_base];
        
        // verifica se conjunto dos herois presentes na base, com o id do heroi, se sim retira ele
        if (cjto_pertence(b->presentes, id_heroi)) {
            cjto_retira(b->presentes, id_heroi);
            
            struct evento *event_avisa = malloc(sizeof(struct evento));
            if (event_avisa == NULL) return;
            event_avisa->id_heroi = -1;
            event_avisa->id_base = id_base;
            event_avisa->id_missao = -1;
            fprio_insere(m->lef, event_avisa, EV_AVISA, tempo);
        }
    }
}
// evento fim
void fim(int tempo, struct mundo *m, int total_eventos) {
    printf("%6d: FIM\n", tempo);
    
    // Estatisticas dos Herois 
    int mortos = 0;
    for (int i = 0; i < m->n_herois; i++) {
        struct heroi h = m->herois[i];
        if (!h.vivo) mortos++;
        printf("HEROI %2d %s  PAC %3d VEL %4d EXP %4d HABS [ ", 
            h.id, h.vivo ? "VIVO " : "MORTO", h.paciencia, h.velocidade, h.experiencia);
        cjto_imprime(h.habilidades);
        printf(" ]\n");
    }
    
    // Estatisticas das Bases
    for (int i = 0; i < m->n_bases; i++) {
        struct base b = m->bases[i];
        printf("BASE %2d LOT %2d FILA MAX %2d MISSOES %d\n", 
            b.id, b.lotacao_max, b.max_espera, b.missoes_participadas);
    }
    
    // Estatisticas das Missoes e do Mundo
    int cumpridas = 0;
    int min_tent = -1, max_tent = -1, total_tent = 0;
    
    for (int i = 0; i < m->n_missoes; i++) {
        struct missao mis = m->missoes[i];
        if (mis.cumprida) cumpridas++;
        
        if (min_tent == -1 || mis.tentativas < min_tent) min_tent = mis.tentativas;
        if (max_tent == -1 || mis.tentativas > max_tent) max_tent = mis.tentativas;
        total_tent += mis.tentativas;
    }
    
    float pct_cumpridas = (float) cumpridas / m->n_missoes * 100.0;
    float media_tent = (float) total_tent / m->n_missoes;
    float taxa_mort = (float) mortos / m->n_herois * 100.0;
    
    printf("EVENTOS TRATADOS: %d\n", total_eventos);
    printf("MISSOES CUMPRIDAS: %d/%d (%.1f%%)\n", cumpridas, m->n_missoes, pct_cumpridas);
    printf("TENTATIVAS/MISSAO: MIN %d, MAX %d, MEDIA %.1f\n", min_tent, max_tent, media_tent);
    printf("TAXA MORTALIDADE: %.1f%%\n", taxa_mort);
}

void destroi_mundo(struct mundo *m) {
    // Destroi o interior dos Herois
    for (int i = 0; i < m->n_herois; i++) {
        cjto_destroi(m->herois[i].habilidades);
    }
    free(m->herois);

    // Destroi o interior das Bases
    for (int i = 0; i < m->n_bases; i++) {
        cjto_destroi(m->bases[i].presentes);
        fila_destroi(m->bases[i].espera);
    }
    free(m->bases);

    // Destroi o interior das Missoes
    for (int i = 0; i < m->n_missoes; i++) {
        cjto_destroi(m->missoes[i].habilidades);
    }
    free(m->missoes);

    // Destroi a fila de prioridades (A LEF limpa os eventos que sobraram)
    fprio_destroi(m->lef);

    // destroi o mundo
    free(m);
}

// programa principal
int main ()
{
    // iniciar o mundo
    srand(0);
    
    struct mundo *mundo = cria_mundo();
    if (mundo == NULL) return 1;
    inicia_evento(mundo);

    // inicia dados de evento
    int tipo_evento;
    int temp_evento;
    struct evento *dados_d_evento;

    int total_eventos = 0;

    int fim_simulacao = 0;

    while (fprio_tamanho(mundo->lef) > 0 && fim_simulacao == 0) {
        dados_d_evento = (struct evento *) fprio_retira(mundo->lef, &tipo_evento, &temp_evento);
        if (dados_d_evento == NULL) {
            break;
        }

        total_eventos++;

        // Se o evento é de um heroi, e ele está morto, ignora:
        // Como o Composto V mata heróis instantaneamente, um herói morto pode ter deixado 
        // eventos "órfãos" agendados no futuro (como viagens ou saídas).
        // Se este evento pertence a um herói específico (!= -1) e ele não está mais vivo, descartamos a ação.
        if (dados_d_evento->id_heroi != -1 && !mundo->herois[dados_d_evento->id_heroi].vivo) {
            free(dados_d_evento);
            continue;
        }

        mundo->tempo_atual = temp_evento;
        
        // O Cérebro do Jogo (A central de comando)
        switch (tipo_evento) {
            
            case EV_CHEGA:
                // O heroi chega na base e decide se espera ou desiste
                chega(temp_evento, dados_d_evento->id_heroi, dados_d_evento->id_base, mundo);
                break;

            case EV_ESPERA:
                // O heroi entra na fila da base e avisa o porteiro
                espera(temp_evento, dados_d_evento->id_heroi, dados_d_evento->id_base, mundo);
                break;

            case EV_AVISA:
                // O porteiro checa a fila e deixa entrar se tiver vaga
                avisa(temp_evento, dados_d_evento->id_base, mundo);
                break;

            case EV_DESISTE:
                // O heroi desiste da fila e escolhe outra base para viajar
                desiste(temp_evento, dados_d_evento->id_heroi, dados_d_evento->id_base, mundo);
                break;

            case EV_ENTRA:
                // O heroi entra na base e agenda a sua saida
                entra(temp_evento, dados_d_evento->id_heroi, dados_d_evento->id_base, mundo);
                break;

            case EV_SAI:
                // O heroi sai da base e agenda uma viagem
                sai(temp_evento, dados_d_evento->id_heroi, dados_d_evento->id_base, mundo);
                break;

            case EV_VIAJA:
                // O heroi se desloca no mapa e agenda a sua chegada
                viaja(temp_evento, dados_d_evento->id_heroi, dados_d_evento->id_base, mundo);
                break;

            case EV_MORRE:
                // O heroi usou o Composto V e morreu
                morre(temp_evento, dados_d_evento->id_heroi, dados_d_evento->id_missao, mundo);
                break;

            case EV_MISSAO:
                // Uma missao aparece no mapa procurando uma equipe
                missao(temp_evento, dados_d_evento->id_missao, mundo);
                break;

            case EV_FIM:
                // Fim do mundo
                fim(temp_evento, mundo, total_eventos);
                fim_simulacao = 1; // <--- PARA O LAÇO IMEDIATAMENTE
                break;

            default:
                printf("ERRO: Evento desconhecido!\n");
                break;
        }

        //destroi os dados de eventos
        free(dados_d_evento);
    }

    // destroi o mundo
    destroi_mundo(mundo);

    return (0) ;
}