# include <scale.h>

void	print_trace(void) {
	void		*array[10];
	size_t		size;
	char		**strings;
	int			f_tab = 0, s_tab = 0, j, k;

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);
	printf("\033[0;31m> \033[0mPrinting backtrace ...\n\e[37m");
#ifndef __APPLE__
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
#else
	for (size_t i = 0; i < size; i++)
		printf("%s\n", strings[i]);
#endif
	printf("\033[0m");
}

scale_entry		m_strcpy(yaml_token_t token) {
	scale_entry		res;
	int				i;

	if (token.data.scalar.length < 64)
		res.buf = malloc(sizeof(char) * 64);
	else
		res.buf = malloc(sizeof(char) * 2024);
	if (!res.buf) {
		ERROR("Malloc error !\n");
	}
	// If i use strcpy here, i get a weird bug on yaml_token_delete
	// So, old school copy it is !
	for (i = 0; token.data.scalar.value[i]; i++)
		res.buf[i] = token.data.scalar.value[i];
	res.buf[i] = 0x0;
	res.len = token.data.scalar.length;
	return res;
}

void	scale_debug(scale *res) {
	printf("Name: %s\n", res->name.buf);
	printf("Comment: %s\n", res->comment.buf);
	printf("Intro: %s\n", res->intro.buf);
	printf("Disclaimer: %s\n", res->disclaimer.buf);
	printf("Guidelines: %s\n", res->guidelines.buf);
	printf("Duration: %d\n", res->duration.val);
	if (res->lang.val == LANG_EN)
		printf("Lang: LANG_EU\n");
	else if (res->lang.val == LANG_FR)
		printf("Lang: LANG_FR\n");
	else if (res->lang.val == LANG_RU)
		printf("Lang: LANG_RU\n");
	printf("Correction num: %d\n", res->correction_n.val);

	for (scale_sections *it = res->sections; it; it = it->next) {
		printf("SECTION: %s\n", it->name.buf);
		for (scale_questions *it2 = it->questions; it2; it2 = it2->next) {
			printf("\tQuestion: %s\n", it2->name.buf);
			for (scale_skills *it3 = it2->skills; it3; it3 = it3->next) {
				printf("\t\t%s : %d%%\n", it3->name.buf, it3->percent.val);
			}
		}
	}
}

int		count_lines(char *s) {
	int		res, i;

	for (res = i = 0; s[i]; i++) {
		if (s[i] == '\n')
			res++;
	}
	if (res <= 5)
		res = 7;
	return res * 20;
}

void		add_section(scale_sections *s, char *name) {
	scale_sections	*res, *it;

	if (!strlen(name))
		return ;
	res = malloc(sizeof(scale_sections));
	res->name.buf = malloc(sizeof(char) * strlen(name));
	res->description.buf = malloc(2024);
	strcpy(res->name.buf, name);
	strcpy(res->description.buf, "");
	res->description.len = 0;
	res->questions = 0x0;
	res->next = 0x0;
	if (!s) {
		s = res;
	} else {
		for (it = s; it->next; it = it->next);
		it->next = res;
	}
}

void		add_question(scale_sections *s, char *name, int j) {
	scale_questions		*res, *tmp;
	scale_sections		*it;
	int					z;

	if (!strlen(name))
		return ;
	res = malloc(sizeof(scale_questions));
	res->name.buf = malloc(64);
	res->guidelines.buf = malloc(1024);
	strcpy(res->guidelines.buf, "");
	strcpy(res->name.buf, name);
	res->skills = 0x0;
	res->next = 0x0;
	res->kind.val = 0;
	res->rating.val = 0;
	for (it = s, z = 0; it && z < j; it = it->next, z++);
	if (!it) {
		ERROR("Can't add question !\n");
	}
	if (!it->questions) {
		it->questions = res;
	} else {
		for (tmp = it->questions; tmp->next; tmp = tmp->next);
		tmp->next = res;
	}
}

void		add_skills(scale_questions *sk, int val, char *name) {
	scale_skills	*new, *it;

	if (!strlen(name))
		return ;
	new = malloc(sizeof(scale_skills));
	new->percent.val = 0;
	new->name.buf = malloc(strlen(name) + 1);
	strcpy(new->name.buf, name);
	new->name.val = val;
	new->next = 0x0;
	if (!sk->skills) {
		sk->skills = new;
	} else {
		for (it = sk->skills; it->next; it = it->next) {
			if (it->name.val == val) {
				free(new->name.buf);
				free(new);
				return ;
			}
		}
		if (it->name.val != val) {
			it->next = new;
		}
	}
}
