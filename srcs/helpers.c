# include <scale.h>

void	print_trace(void) {
	void		*array[10];
	size_t		size;
	char		**strings;

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);
	printf("\033[0;32m> \033[0m Printing backtrace ...\n\e[37m");
# define NK_IMPLEMENTATION
	for (size_t i = 0; i < size; i++)
		printf("%s\n", strings[i]);
	free(strings);
	printf("\033[0m");
}
