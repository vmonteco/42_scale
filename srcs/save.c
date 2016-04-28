# include <scale.h>

void	write_s(FILE *fd, char *name, char *string, int is_md) {
	if (!strlen(string)) {
		string = malloc(3);
		strcpy(string, "''");
		is_md = 0;
	}
	if (is_md)
		fprintf(fd, "%s: |\n\n%s\n\n", name, string);
	else
		fprintf(fd, "%s: %s\n\n", name, string);
}

void	write_i(FILE *fd, char *name, int val) {
	fprintf(fd, "%s: %d\n\n", name, val);
}

void	save_scale(scale *s) {
	FILE	*fd = fopen("/tmp/test.yml", "w+");

	if (!fd) {
		ERROR("Cannot open %s for writing\n", s->o_file);
	}
	fprintf(fd, "#########################################\nThis scale has been generated by 42_scale\n#########################################\n");
	write_s(fd, "name", s->name.buf, 0);
	if (s->lang.val == LANG_EN) {
		write_s(fd, "lg", "en", 0);
	} else if (s->lang.val == LANG_FR) {
		write_s(fd, "lg", "fr", 0);
	} else if (s->lang.val == LANG_RU) {
		write_s(fd, "lg", "ru", 0);
	}
	fprintf(fd, "is_primary: true\n\n");
	write_s(fd, "comment", s->comment.buf, 0);
	write_s(fd, "introduction_md", s->intro.buf, 1);
	write_s(fd, "disclaimer_md", s->disclaimer.buf, 1);
	write_s(fd, "guidelines_md", s->guidelines.buf, 1);
	write_i(fd, "correction_number", s->correction_n.val);
	write_i(fd, "duration", s->duration.val);
	fclose(fd);
}