# include <scale.h>

void	print_trace(void) {
	void		*array[10];
	size_t		size;
	char		**strings;
	int			f_tab = 0, s_tab = 0, j, k;

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);
	printf("\033[0;31m> \033[0mPrinting backtrace ...\n\e[37m");
	for (size_t i = 0; i < size; i++) {
		for (j = 0; strings[i][j] && strings[i][j] != '('; j++);
		if (j > f_tab)
			f_tab = j;
		for (k = j; strings[i][j] && strings[i][j] != ')'; j++);
		if ((j - k) > s_tab)
			s_tab = (j - k) + 2;
	}
	for (size_t i = 0; i < size; i++) {
		for (j = 0; strings[i][j] && strings[i][j] != '('; write(1, &strings[i][j], 1), j++);
		if (j < f_tab)
			for (k = j; k < f_tab; write(1, " ", 1), k++);
		write(1, " ", 1);
		for (k = j; strings[i][j] && strings[i][j] != ')'; write(1, &strings[i][j], 1), j++) {
			if (strings[i][j] == '+' || (j && strings[i][j - 1] == '+'))
				write(1, " ", 1);
		}
		write(1, ")", 1);
		if ((j - k) < s_tab) {
			if ((j - k) == 1)
				write(1, "  ", 2);
			for (k = (j - k); k < s_tab; write(1, " ", 1), k++);
		}
		write(1, " ", 1);
		for (j++; strings[i][j]; write(1, &strings[i][j], 1), j++);
		write(1, "\n", 1);
	}
	free(strings);
	printf("\033[0m");
}
