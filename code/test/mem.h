#ifndef __MEM_H
#define __MEM_H

#ifndef MEM_STRATEGY
#define MEM_STRATEGY mem_fit_first
#endif

#ifndef __BIGGEST_ALIGNEMENT__
#define  __BIGGEST_ALIGNEMENT__ 4
#endif

#define DO_ALIGN(val, align) ((val+(align-1))&~(align-1))
#define FREEBLK_HEADSIZE (DO_ALIGN(sizeof(struct fb), __BIGGEST_ALIGNMENT__))
#define BUSYBLK_HEADSIZE (DO_ALIGN(sizeof(struct bb), __BIGGEST_ALIGNMENT__))
#define FREEBLK_MINSIZE FREEBLK_HEADSIZE + __BIGGEST_ALIGNMENT__

#define size_t unsigned

typedef struct fb fb;
typedef fb* pfb;
struct fb
{
    size_t size;
    pfb next;
};

typedef struct bb bb;
typedef bb* pbb;
struct bb
{
    size_t size;
};

struct mem_state
{
    unsigned int nb_free;
    unsigned int nb_busy;

    size_t free;
    size_t min_free;
    size_t max_free;

    size_t busy;
    size_t min_busy;
    size_t max_busy;

    size_t segment_used;
};

void mem_init(size_t taille);
void* mem_alloc(size_t);
void mem_free(void*);
size_t mem_get_size(void *);

/* Itérateur sur le contenu de l'allocateur */
void mem_show(void (*print)(void *, size_t, int free));

/* Choix de la strategie et strategies usuelles */
typedef struct fb* (mem_fit_function_t)(struct fb *, size_t);

void mem_fit(mem_fit_function_t*);
struct fb* mem_fit_first(struct fb*, size_t);

/* Facultatif */
struct fb* mem_fit_best(struct fb*, size_t);
struct fb* mem_fit_worst(struct fb*, size_t);


/* Fonctions ajoutés */
pfb get_zone_libre_prec(void* zone);
pfb get_zone_libre_suiv(void* zone);

/* Retourne l'état de la mémoire */
struct mem_state mem_get_state();

//void fprint_mem_state( pid_t pid, pthread_t tid, int is_alloc, size_t asked_size, size_t size, char* addr, struct mem_state state);

#endif
