#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "sexp.h"
#include "symbol_table.h"


/*
 * hash function
 *
 * this function return the hash code of the key
 * the is not dependent of the hashtable, this mean
 * the modulo is done later
 *
 * Actual implementation:
 * djb2 by Dan Bernstein
 */
static unsigned hash(const char *key)
{
	unsigned val = 5381;
    int c;

	while ((c = *key++))
		val = ((val << 5) + val) + c;

	return val;
}

static unsigned st_hash(struct symbol_table *st, const char *key)
{
	return hash(key) % st->size;
}

/*
 * symbol_table_init:
 *
 * Initialize the symbol table, does the allocation
 * of the internal table
 *
 * the behavior is undefined if you call this function
 * twice without calling symbol_table_free
 *
 * This function return 0 if the symbol could be set
 * another value otherwise
 */
int symbol_table_init(struct symbol_table *st, int size)
{
	int i;
	assert(st);
	assert(size >= 0);
	st->size = size;
	st->table = malloc(size * sizeof(*st->table));

	for (i = 0; i < size; i++)
		st->table[i] = NULL;

	return st->table == NULL ? 1 : 0;
}


static void free_entries(struct entry *start)
{
	struct entry *cur = start;

	while(cur != NULL)
	{
		free((char*)cur->key);
		sexp_free(cur->data);
		free(cur);
		cur = cur->next;
	}
}

/*
 * symbol_table_free:
 *
 * Free the internal symbol table
 */
void symbol_table_free(struct symbol_table *st)
{
	int i;

	assert(st);

	for (i = 0; i < st->size; i++)
	{
		free_entries(st->table[i]);
	}
	free(st->table);
}

/*
 * symbol_table_set:
 *
 * set the key with the given data, both the key and the data
 * (which is a sexp) are copied in memory

 * This function return 0 if the symbol could be set
 * another value otherwise
 *
 * If key is NULL or empty, this will be considered as an error
 *
 */
int symbol_table_set(struct symbol_table *st,
					 const char *key, struct sexp *sexp)
{
	unsigned hashval;
	struct entry *entry, *new, *last;

	assert(st);

	if (key == NULL || !*key)
	{
		return 1;
	}

	hashval = st_hash(st, key);
	entry = st->table[hashval];
	last = NULL;

	/* We search if the entry already exists */
	while (entry != NULL)
	{
		if (!strcmp(entry->key, key))
		{
			/* We found the entry, we try to replace it */
			struct sexp *replace;

			replace = sexp_dup(sexp);
			if (sexp && replace == NULL)
			{
				return 1;
			}
			sexp_free(entry->data);
			entry->data = replace;
			return 0;
		}

		last = entry;
		entry = entry->next;
	}

	/* At this point, no entry was found, so we create a new one (if we can) */
	new = malloc(sizeof(*new));

	if (new == NULL)
	{
		return 1;
	}
	new->next = NULL;

	new->key = strdup(key);
	if (new->key == NULL)
	{
		free(new);
		return 1;
	}


	new->data = sexp_dup(sexp);
	if (sexp && new->data == NULL)
	{
		free((char*)new->key);
		free(new);
		return 1;
	}

	/* if last is NULL, there was no entry for the hash value
	 * so we add one */
	if (last == NULL)
	{
		st->table[hashval] = new;
	}
	else
	{
		/* We simply link to the last entry otherwise */
		last->next = new;
	}

	return 0;
}

/*
 * symbol_table_lookup:
 *
 * Try to find the entry with the given key in the symbol table
 * If no entry is found, return (void*)-1
 */
struct sexp *symbol_table_get(struct symbol_table *st, const char *key)
{
	struct entry *entry;
	unsigned hashval;

	assert(st);

	if (key == NULL || !*key)
	{
		return (void*)-1;
	}

	hashval = st_hash(st, key);
	entry = st->table[hashval];

	while (entry != NULL)
	{
		if (!strcmp(key, entry->key))
			return entry->data;

		entry = entry->next;
	}

	return (void*)-1;
}
