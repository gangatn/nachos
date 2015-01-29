#include "stdlib.h"
#include "syscall.h"

#ifndef MEM_STRATEGY
#define MEM_STRATEGY mem_fit_first
#endif

#ifndef __BIGGEST_ALIGNEMENT__
#define  __BIGGEST_ALIGNEMENT__ 4
#endif

#define MEM_INIT_SIZE 524288

#define DO_ALIGN(val, align) ((val+(align-1))&~(align-1))
#define FREEBLK_HEADSIZE (DO_ALIGN(sizeof(struct fb), __BIGGEST_ALIGNMENT__))
#define BUSYBLK_HEADSIZE (DO_ALIGN(sizeof(struct bb), __BIGGEST_ALIGNMENT__))
#define FREEBLK_MINSIZE FREEBLK_HEADSIZE + __BIGGEST_ALIGNMENT__

#define exit Exit

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

/* Choix de la strategie et strategies usuelles */
typedef struct fb* (mem_fit_function_t)(struct fb *, size_t);

struct fb* mem_fit_first(struct fb*, size_t);
struct fb* mem_fit_best(struct fb*, size_t);
struct fb* mem_fit_worst(struct fb*, size_t);

/* Fonctions ajoutés */
pfb get_zone_libre_prec(void* zone);
pfb get_zone_libre_suiv(void* zone);

pfb fb_init;
char* mem_start = NULL;
mem_fit_function_t* mem_fit_function;
size_t mem_size;
char * mem = NULL;

int mem_num_alloc = 0, mem_num_free = 0;

static void mem_init()
{
	size_t taille = MEM_INIT_SIZE;

    if (taille < 1 || taille < sizeof(fb))
        exit(EXIT_FAILURE);
    mem = (char *) Sbrk(taille);
    if(mem ==(char*) -1) {
		PutString("Sbrk fail to init\n");
		exit(EXIT_FAILURE);
    }
    fb_init = (struct fb*)mem;
    fb_init->size = taille;
    fb_init->next = NULL;

    mem_size = taille;
    mem_start = mem;
    mem_fit_function = MEM_STRATEGY;

}

static void* mem_alloc(size_t taille) {
    struct fb *prev, *next, *pfba;
    struct bb *pbba;
    size_t bb_size = BUSYBLK_HEADSIZE + taille;
    char* ptr;


	if(mem == NULL)
	{
		mem_init();
	}

    /* on cherche un freeblock de la taille du busyblock */
    pfba = mem_fit_function(fb_init, bb_size);
    if (!pfba) {
        return NULL;
	}

    prev = get_zone_libre_prec(pfba);
    next = get_zone_libre_suiv(pfba);

	mem_num_alloc++;

    if (pfba->size - bb_size < FREEBLK_MINSIZE) {
        /* il n'y a pas suffisamment d'espace dans le freeblock,
         * on alloue tout le freeblock au busyblock
         */
        pbba = (struct bb*)pfba;
        pbba->size = pfba->size;

        /* puis mise à jour du chaînage */
        if ((char*)pbba == mem_start )
            fb_init = NULL;
        else if ((char*)pbba == (char*)fb_init)
            fb_init = next;
        else if (prev)
            prev->next = next;

        return ((char*)pbba) + BUSYBLK_HEADSIZE;
    }

    /* on place le busyblock à la fin du freeblock
     * en l'alignant correctement (en dessous) puis en lui allouant
     * la taille restant en fin de block
     * le chaînage est inchangé
     */
    ptr = ((char*)pfba) + pfba->size - bb_size;
    pbba = (struct bb*)(DO_ALIGN((long)ptr, __BIGGEST_ALIGNMENT__));
    pbba = (struct bb*)((char*)pbba - __BIGGEST_ALIGNMENT__);
    pbba->size = bb_size + (ptr - (char*)pbba);
    pfba->size -= pbba->size;

    return ((char*)pbba) + BUSYBLK_HEADSIZE;
}

static void mem_free(void* ptr) {
    struct fb *next, *prev, *p;
    struct bb* pbba = (struct bb*)(((char*)ptr) - BUSYBLK_HEADSIZE);
    struct fb* pfba = (struct fb*)pbba;

    /*  prev = get_zone_libre_prec(pfb); */
    prev = NULL;
    p = fb_init;
    while ( p && p < pfba) {
        prev = p;
        p = p->next;
    }

    /* next = get_zone_libre_suiv(pfb); */
    p = fb_init;
    while (p && p < pfba)
        p = p->next;
    next = p;

	mem_num_free++;

    if (!prev) {
        /* il n'existe pas de block libre précédent */
        pfba->size = pbba->size;
        pfba->next = next;
        if (next && ((char*)pfba) + pfba->size == (char*)next) {
            /* le prochain block adjacent est un free : merge du courant et du suivant*/
            pfba->size += next->size;
            pfba->next = next->next;
        }

        /* mise à jour du premier freeblock */
        fb_init = pfba;
        return;
    }

    if (((char*)prev) + prev->size == (char*)pfba) {
        /* le précédent block adjacent est un free : merge du précédent et du courant */
        prev->size += pbba->size;
        if (next && ((char*)pfba) + pbba->size == (char*)next) {
            /* le prochain block adjacent est un free : merge du précédent et du suivant */
            prev->size += next->size;
            prev->next = next->next;
        }
        return;
    }

    /* le précédent block adjacent n'est pas un free mais il y a quand même un précédent free */
    prev->next = pfba;
    pfba->size = pbba->size;
    pfba->next = next;
    if (next && ((char*)pfba) + pfba->size == (char*)next) {
        /* le prochain block adjacent est un free : merge du courant et du suivant*/
        pfba->size += next->size;
        pfba->next = next->next;
    }
}

static size_t mem_get_size(void * ptr) { // pour le realloc
    pbb pbusy = (pbb)((char*)ptr - BUSYBLK_HEADSIZE);
    return pbusy->size - sizeof(bb);
}

struct fb* mem_fit_first(struct fb* pfba, size_t size) {
	char* growSize;
	struct fb* pfbprec = fb_init;
    if (pfba->size >= size)
        return pfba;
	else if(!pfba) { // Pas d'espace suffisament grand
		// On tente d'augmenter l'espace dispo
		growSize = (char*) Sbrk(size); // valeur arbitraire >= size
		if(growSize == (char*)-1)
			return NULL;

		mem_size += size;

		if(fb_init == NULL) { // Tout l'espace est alloué
			fb_init = (struct fb*) growSize;
			fb_init->size = size;
			fb_init->next = NULL;
		} else if(((char*)pfbprec + pfbprec->size) + 1 == mem + mem_size) {
			// fusion de l'espace disponible avec les nouvelles pages dispo
			pfbprec->size += size;
			pfbprec->next = NULL;
			return pfbprec;
		}

		pfbprec->next = (struct fb*) growSize;
		pfbprec->next->size = size;
		pfbprec->next->next = NULL;
		return pfbprec->next;
	} else {
    	pfbprec = pfba;
		return mem_fit_first(pfba->next, size);
	}
}

/* Facultatif */
struct fb* mem_fit_best(struct fb* pfba, size_t size) {
    if (!pfba)
        return pfba;

    /* récupération du free bloc suivnat de taille min  */
    struct fb* next_min = mem_fit_best(pfba->next, size);

    /* pas de suivant et bloc ok pour allocation */
    if (!next_min && pfba->size >= size )
        return pfba;

    /* pas de suivant et bloc non ok pour allocation */
    if (!next_min)
        return NULL;

    /* comparaison du bloc courant et du min des suivants */

    /* le bloc courant qui fit */
    if (pfba->size >= size) {
        /* mais le suivant fit mieux */
        if (next_min->size >= size && next_min->size < pfba->size)
            return next_min;
        return pfba;
    }

    /* le bloc courant ne fit pas mais le suivant oui */
    if (next_min->size >= size)
        return next_min;

    /* sinon rien */
    return NULL;
}


struct fb* mem_fit_worst(struct fb* pfba, size_t size) {
    if (!pfba)
        return pfba;

    /* récupération du free bloc suivnat de taille max */
    struct fb* next_max = mem_fit_best(pfba->next, size);

    /* pas de suivant et bloc ok pour allocation */
    if (!next_max && pfba->size >= size )
        return pfba;

    /* pas de suivant et bloc non ok pour allocation */
    if (!next_max)
        return NULL;

    /* comparaison du bloc courant et du max des suivants */

    /* le bloc courant qui fit */
    if (pfba->size >= size) {
        /* mais le suivant fit mieux */
        if (next_max->size >= size && next_max->size < pfba->size)
            return next_max;
        return pfba;
    }

    /* le bloc courant nefit pasmais le suivant oui */
    if (next_max->size >= size)
        return next_max;

    /* sinon rien */
    return NULL;
}

/* Fonctions ajoutés */
pfb get_zone_libre_prec(void* zone) {
    struct fb* prev = NULL;
    struct fb* ptr = fb_init;

    while ( ptr && ptr < (struct fb*)zone) {
        prev = ptr;
        ptr = ptr->next;
    }
    return prev;
}

pfb get_zone_libre_suiv(void* zone) {
    struct fb* ptr = fb_init;

    while (ptr && ptr <= (struct fb*)zone)
        ptr = ptr->next;

    return ptr;
}

void *malloc(size_t size)
{
	return mem_alloc(size);
}

void *realloc(void *ptr, size_t size)
{
	size_t s;
	char *result;

	if (!ptr)
	{
		return mem_alloc(size);
	}

	if (mem_get_size(ptr) >= size)
	{
		return ptr;
    }

	result = mem_alloc(size);

	if (!result)
	{
		return NULL;
	}

	for (s = 0; s < mem_get_size(ptr); s++)
        result[s] = ((char *) ptr)[s];

	mem_free(ptr);
	return result;
}

/* TODO: Optimize the set, maybe use memset */
void *calloc(size_t count, size_t size)
{
	size_t total_size = count * size;
	int i;
	char *res;

	res = mem_alloc(total_size);

	if (res)
	{
		for (i = 0; i < total_size; i++)
            res[i] = 0;
	}
	return res;
}

void free(void *ptr)
{
	if (ptr)
	{
		mem_free(ptr);
	}
}

void mem_print_stats(void)
{

}
