/*
 * Symbol table
 *
 * This uses an hash table (with linked list for collisions)
 * You can only add sexp in the hashtable, this is for typing convenience
 * and because we don't use the symbol table for other types than sexp
 *
 * The symbol_table structure allocation is left to the user,
 * however, the internal table is allocated by this module
 * (symbol_table_init function)
 *
 * TODO:
 *    - Make the local environment shadowing easier (this is really easy to do)
 *      and this will simplify other code
 */
#ifndef SYMBOL_TABLE_H_
#define SYMBOL_TABLE_H_

#include <stddef.h>

#include "sexp.h"

struct entry
{
	const char *key;
	struct sexp *data;
	struct entry *next;
};

struct symbol_table
{
	size_t size;
	struct entry **table;
};

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
int symbol_table_init(struct symbol_table *st, int size);

/*
 * symbol_table_free:
 *
 * Free the internal symbol table
 */
void symbol_table_free(struct symbol_table *st);

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
					 const char *key, struct sexp *sexp);

/*
 * symbol_table_lookup:
 *
 * Try to find the entry with the given key in the symbol table
 * If no entry is found, return (void*)-1
 */
struct sexp *symbol_table_get(struct symbol_table *st, const char *key);

#endif /* SYMBOL_TABLE_H_ */
