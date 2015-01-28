#include <stdlib.h>

#include "crisp/parse.h"
#include "crisp/eval.h"
#include "crisp/symbol_table.h"
#include "syscall.h"

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

	return EXIT_SUCCESS;
}
