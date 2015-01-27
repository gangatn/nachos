#include "crisp/parse.h"
#include "crisp/eval.h"
#include "syscall.h"

int main(void)
{
	struct sexp *parsed, *result;

	parsed = parse();

	while (parsed != NULL)
	{
		result = eval(parsed);

		sexp_print(result);
		PutChar('\n');

		sexp_free(result);
		sexp_free(parsed);

		parsed = parse();
	}

	return 0;
}
