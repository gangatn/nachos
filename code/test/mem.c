/*
 * mem.c
 *
 */
#include "mem.h"
#include "syscall.h"
#define NULL ((void *)0)
#define EXIT_FAILURE -1

#define exit Exit

pfb fb_init;
char* mem_start;
mem_fit_function_t* mem_fit_function;
size_t mem_size;
char * mem;
//pthread_mutex_t mutex;

void mem_init(size_t taille) {
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

    //char nom_log[256];

    //snprintf(nom_log, 50, "mem_%d.log", getpid());

    //pthread_mutex_init(&mutex, NULL);
}

void* mem_alloc(size_t taille) {
    struct fb *prev, *next, *pfba;
    struct bb *pbba;
    size_t bb_size = BUSYBLK_HEADSIZE + taille;
    char* ptr;

    //pthread_mutex_lock(&mutex);

    /* on cherche un freeblock de la taille du busyblock */
    pfba = mem_fit_function(fb_init, bb_size);
    if (!pfba) {
        //pthread_mutex_unlock(&mutex);
        return NULL;
    }

    prev = get_zone_libre_prec(pfba);
    next = get_zone_libre_suiv(pfba);

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
        // fprint_mem_state(getpid(), pthread_self(), 1, taille, pbb->size, ((char*)pbb) + BUSYBLK_HEADSIZE, mem_get_state());
        
        // pthread_mutex_unlock(&mutex);
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
    //fprint_mem_state(getpid(), pthread_self(), 1, taille, pbb->size, ((char*)pbb) + BUSYBLK_HEADSIZE, mem_get_state());

    // pthread_mutex_unlock(&mutex);
    return ((char*)pbba) + BUSYBLK_HEADSIZE;
}

void mem_free(void* ptr) {
    struct fb *next, *prev, *p;
    struct bb* pbba = (struct bb*)(((char*)ptr) - BUSYBLK_HEADSIZE);
    struct fb* pfba = (struct fb*)pbba;
    //int pbba_size = pbba->size;
    // pthread_mutex_lock(&mutex);

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
        //fprint_mem_state(getpid(), pthread_self(), 0, 0, pbb_size, ptr, mem_get_state());

        //pthread_mutex_unlock(&mutex);
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
        //fprint_mem_state(getpid(), pthread_self(), 0, 0, pbb_size, ptr, mem_get_state());

        //pthread_mutex_unlock(&mutex);
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
    //fprint_mem_state(getpid(), pthread_self(), 0, 0, pbb_size, ptr, mem_get_state());

    //pthread_mutex_unlock(&mutex);
}

size_t mem_get_size(void * ptr) { // pour le realloc
    //pthread_mutex_lock(&mutex);
    pbb pbusy = (pbb)((char*)ptr - BUSYBLK_HEADSIZE);
    //pthread_mutex_unlock(&mutex);
    return pbusy->size - sizeof(bb);
}

/* Itérateur sur le contenu de l'allocateur */
void mem_show(void (*print)(void *, size_t, int free)) {
    pfb f = fb_init;
    char* ptr = mem_start;
    char* mem_end = mem_start + mem_size;

    //pthread_mutex_lock(&mutex);

    while (ptr < mem_end) {
        if (f && (char*) f == ptr) {
            /* ptr sur f: on est sur un freeblock */
            print(ptr, f->size, 1);
            ptr += f->size;
            f = f->next;
        } else {
            /* ptr avant f: on est sur un busyblock */
            print(ptr, ((pbb)ptr)->size, 0);
            ptr += ((pbb)ptr)->size;
        }
    }

    //pthread_mutex_unlock(&mutex);
}

void mem_fit(mem_fit_function_t* function) {
    mem_fit_function = function;
}

struct fb* mem_fit_first(struct fb* pfba, size_t size) {
    if (!pfba || pfba->size >= size)
        return pfba;
    else
        return mem_fit_first(pfba->next, size);
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

struct mem_state mem_get_state() {
    struct mem_state state;

    char* p = mem_start;
    pfb free = fb_init;
    char* mem_end = mem_start + mem_size;

    pfb fcur;
    pbb bcur;

    char* first_busy_down = NULL;
    char* last_busy_up;

    state.nb_free = 0;
    state.nb_busy = 0;
    state.free = 0;
    state.min_free = 0;
    state.max_free = 0;
    state.busy = 0;
    state.min_busy = 0;
    state.max_busy = 0;
    state.segment_used = 0;

    while (p < mem_end) {

        if ((char*)free == p) {
            /* le bloc courant est un freeblock */
            fcur = (pfb)p;

            state.nb_free++;
            state.free += fcur->size;

            if (fcur->size < state.min_free || state.min_free == 0)
                state.min_free = fcur->size;

            if (fcur->size > state.max_free || state.max_free == 0)
                state.max_free = fcur->size;

            p += fcur->size;
            free = fcur->next;

            continue;
        }

        /* le bloc courant est un busyblock */
        bcur = (pbb)p;

        state.nb_busy++;
        state.busy += bcur->size;

        if (bcur->size < state.min_busy || state.min_busy == 0)
            state.min_busy = bcur->size;

        if (bcur->size > state.max_busy || state.max_busy == 0)
            state.max_busy = bcur->size;

        p += bcur->size;

        /* on a trouvé le premier busyblock */
        if (!first_busy_down)
            first_busy_down = (char*)bcur;

        /* recherche de la borne supérieure du dernier busyblock */
        last_busy_up = p;
    }

    /* calcul de la taille du segment mémoire utilisé */
    if (first_busy_down)
        state.segment_used = last_busy_up - first_busy_down;

    return state;
}

//void fprint_mem_state(pid_t pid, pthread_t tid, int is_alloc, size_t asked_size, size_t size, char* addr, struct mem_state state) {
//   fprintf(stderr, "##%d;", pid);
//   fprintf(stderr, "%lu;", (unsigned long)tid);
//   fprintf(stderr, "%p;", addr);
//   fprintf(stderr, "%d;", is_alloc);
//   fprintf(stderr, "%zu;", asked_size);
//   fprintf(stderr, "%zu;", size);
//   fprintf(stderr, "%d;", state.nb_free);
//   fprintf(stderr, "%d;", state.nb_busy);
//   fprintf(stderr, "%zu;", state.segment_used);
//   fprintf(stderr, "%zu;", state.free);
//   fprintf(stderr, "%zu;", state.min_free);
//   fprintf(stderr, "%zu;", state.max_free);
//   fprintf(stderr, "%zu;", state.busy);
//   fprintf(stderr, "%zu;", state.min_busy);
//   fprintf(stderr, "%zu\n", state.max_busy);
//}
