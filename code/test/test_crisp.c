#include "crisp/parse.h"
#include "crisp/eval.h"
#include "syscall.h"

int main(void)
{
	struct sexp *parsed, *result;

	parsed = parse();
	result = eval(parsed);

	sexp_print(result);
	PutChar('\n');

	sexp_free(result);
	sexp_free(parsed);
	return 0;
}
