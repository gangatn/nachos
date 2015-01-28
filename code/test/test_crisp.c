#include <stdlib.h>

#include "crisp/parse.h"
#include "crisp/eval.h"
#include "crisp/symbol_table.h"
#include "syscall.h"



void print_memory_report(void)
{
	/* this is what happen when we don't have valgrinds */
	extern int mem_num_alloc, mem_num_free;
	PutString("\n\n===============\n");
	PutString("[memory report]\n");
	PutString("===============\n");
	PutString("| alloc: "); PutInt(mem_num_alloc); PutChar('\n');
	PutString("| free : "); PutInt(mem_num_free); PutChar('\n');
	PutString("===============\n");
}

int main(void)
{
	struct symbol_table st;
	struct sexp *parsed, *result;

	if (symbol_table_init(&st, 1024) != 0)
	{
		PutString("Could not init symbol table\n");
		return EXIT_FAILURE;
	}

	parsed = parse();

	if (parsed == (void*)-1)
	{
		PutString("Syntax error.\n");
		return EXIT_SUCCESS;
	}

	while (parsed != NULL)
	{
		result = eval(parsed, &st);

		if(result != (void*)-1)
		{
			sexp_print(result);
			PutChar('\n');
			sexp_free(result);
		}

		sexp_free(parsed);

		parsed = parse();
		if (parsed == (void*)-1)
		{
			PutString("Syntax error.\n");
			return EXIT_SUCCESS;
		}
	}

	symbol_table_free(&st);

	print_memory_report();
	return EXIT_SUCCESS;
}
