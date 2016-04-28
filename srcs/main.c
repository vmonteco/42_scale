# include <scale.h>

int		main(int ac, char **av) {
	FILE	*fd;
	scale	*s;

	if (ac == 1) {
		ERROR("Need an argument !\n");
	}
	fd = fopen(av[1], "r");
	if (!fd) {
		ERROR("Failed to read file %s\n", av[1]);
	}
	s = read_scale(fd);
	s->o_file = malloc(sizeof(char) * strlen(av[1]) + 1);
	strcpy(s->o_file, av[1]);
	window(s);
	return 0;
}
