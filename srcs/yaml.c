# include <scale.h>

void	read_base_infos(yaml_parser_t *parser, scale *res) {
	yaml_token_t		token;
	int					cur = 0;
	char				*key = 0x0, *tmp;

	token.type = YAML_STREAM_START_TOKEN;
	res->sections = 0x0;
	while (token.type != YAML_STREAM_END_TOKEN) {
		if (!yaml_parser_scan(parser, &token)) {
			ERROR("YAML parser error ! Error value: (%d)", parser->error);
		}
		if (token.type == YAML_KEY_TOKEN) {
			cur = 1;
		} else if (token.type == YAML_VALUE_TOKEN) {
			cur = 2;
		} else if (token.type == YAML_SCALAR_TOKEN) {
			if (cur == 1) {
				key = m_strcpy(token);
			} else {
				if (!strcmp(key, "name"))
					res->name = m_strcpy(token);
			 	else if (!strcmp(key, "comment"))
					res->comment = m_strcpy(token);
				else if (!strcmp(key, "introduction_md"))
					res->intro = m_strcpy(token);
				else if (!strcmp(key, "disclaimer_md"))
					res->disclaimer = m_strcpy(token);
				else if (!strcmp(key, "guidelines_md"))
					res->guidelines = m_strcpy(token);
				else if (!strcmp(key, "lang")) {
					tmp = m_strcpy(token);
					if (!strcmp(tmp, "en"))
						res->lang = LANG_EN;
					else if (!strcmp(tmp, "fr"))
						res->lang = LANG_FR;
					else if (!strcmp(tmp, "ru"))
						res->lang = LANG_RU;
					free(tmp);
				} else if (!strcmp(key, "correction_number")) {
					res->correction_n = atoi((char *)token.data.scalar.value);
				}
				free(key);
				key = 0x0;
				cur = 0;
			}
		} else if (token.type == 14) {
			break ;
		}
		if (token.type != YAML_STREAM_END_TOKEN) {
			yaml_token_delete(&token);
		}
	}
	yaml_token_delete(&token);
}

void	read_skills(yaml_parser_t *parser, scale_questions *question) {
	yaml_token_t	token;
	scale_skills	*sk, *it;
	int				cur = 0, ct = 0;
	char			*key = 0x0, *tmp = 0x0;

	token.type = YAML_STREAM_START_TOKEN;
	sk = malloc(sizeof(scale_skills));
	sk->next = 0x0;
	(void)question;
	while (token.type != YAML_STREAM_END_TOKEN) {
		if (!yaml_parser_scan(parser, &token)) {
			ERROR("YAML parser error ! Error value: (%d)", parser->error);
		}
		if (token.type == YAML_KEY_TOKEN) {
			cur = 1;
		} else if (token.type == YAML_VALUE_TOKEN) {
			cur = 2;
		} else if (token.type == YAML_SCALAR_TOKEN) {
			if (cur == 1) {
				key = m_strcpy(token);
			} else {
				if (!strcmp(key, "name")) {
					sk->name = m_strcpy(token);
				} else if (!strcmp(key, "percentage")) {
					tmp = m_strcpy(token);
					sk->percent = atoi(tmp);
					free(tmp);
				}
			}
		} else if (token.type == YAML_BLOCK_END_TOKEN) {
			ct++;
			if (ct == 2)
				return ;
			if (!question->skills) {
				question->skills = sk;
			} else {
				for (it = question->skills; it->next; it = it->next);
				it->next = sk;
			}
			sk->next = 0x0;
			sk = malloc(sizeof(scale_skills));
		} else {
			ct = 0;
		}
	}

}

void	read_questions(yaml_parser_t *parser, scale_sections *sec) {
	yaml_token_t	token;
	scale_questions	*question, *it;
	int				cur = 0;
	char			*key = 0x0, *tmp = 0x0;

	question = malloc(sizeof(scale_questions));
	question->next = 0x0;
	token.type = YAML_STREAM_START_TOKEN;
	while (token.type != YAML_STREAM_END_TOKEN) {
		if (!yaml_parser_scan(parser, &token)) {
			ERROR("YAML parser error ! Error value: (%d)", parser->error);
		}
		if (token.type == YAML_KEY_TOKEN) {
			cur = 1;
		} else if (token.type == YAML_VALUE_TOKEN) {
			cur = 2;
		} else if (token.type == YAML_SCALAR_TOKEN) {
			if (cur == 1) {
				key = m_strcpy(token);
				if (!strcmp(key, "questions_skills")) {
					read_skills(parser, question);
					if (!sec->questions) {
						sec->questions = question;
					} else {
						for (it = sec->questions; it->next; it = it->next);
						it->next = question;
					}
					question = malloc(sizeof(scale_questions));
					question->next = 0x0;
				}
			} else {
				if (!strcmp(key, "name"))
					question->name = m_strcpy(token);
				else if (!strcmp(key, "guidelines"))
					question->guidelines = m_strcpy(token);
				else if (!strcmp(key, "rating")) {
					tmp = m_strcpy(token);
					if (!strcmp(tmp, "bool"))
						question->rating = R_BOOL;
					else if (!strcmp(tmp, "multi"))
						question->rating = R_MULTI;
					free(tmp);
				} else if (!strcmp(key, "kind")) {
					tmp = m_strcpy(token);
					if (!strcmp(tmp, "standard"))
						question->kind = R_STAND;
					else if (!strcmp(tmp, "bonus"))
						question->kind = R_BONUS;
				}
				free(key);
			}
		} else if (token.type == YAML_BLOCK_END_TOKEN) {
			return ;
		}
		if (token.type != YAML_STREAM_END_TOKEN) {
			yaml_token_delete(&token);
		}
	}
	yaml_token_delete(&token);
}

void	read_sections(yaml_parser_t *parser, scale *res) {
	yaml_token_t	token;
	int				cur = 0;
	char			*key = 0x0;
	scale_sections	*sec, *it;

	token.type = YAML_STREAM_START_TOKEN;
	sec = malloc(sizeof(scale_sections));
	sec->questions = 0x0;
	sec->next = 0x0;
	while (token.type != YAML_STREAM_END_TOKEN) {
		if (!yaml_parser_scan(parser, &token)) {
			ERROR("YAML parser error ! Error value: (%d)", parser->error);
		}
		if (token.type == YAML_KEY_TOKEN) {
			cur = 1;
		} else if (token.type == YAML_VALUE_TOKEN) {
			cur = 2;
		} else if (token.type == YAML_SCALAR_TOKEN) {
			if (cur == 1) {
				key = m_strcpy(token);
				if (!strcmp(key, "questions")) {
					read_questions(parser, sec);
					if (!res->sections) {
						res->sections = sec;
					} else {
						for (it = res->sections; it->next; it = it->next);
						it->next = sec;
					}
					sec = malloc(sizeof(scale_sections));
					sec->questions = 0x0;
					sec->next = 0x0;
				}
			} else {
				if (!strcmp(key, "name"))
					sec->name = m_strcpy(token);
				else if (!strcmp(key, "description"))
					sec->description = m_strcpy(token);
				free(key);
				key = 0x0;
				cur = 0;
			}
		}
		if (token.type != YAML_STREAM_END_TOKEN) {
			yaml_token_delete(&token);
		}
	}
	yaml_token_delete(&token);
}

scale	*read_scale(FILE *fd) {
	yaml_parser_t		parser;
	scale				*res;

	if (!yaml_parser_initialize(&parser)) {
		ERROR("Failed to initialize YAML parser !\n");
	}
	yaml_parser_set_input_file(&parser, fd);
	if (!(res = malloc(sizeof(scale)))) {
		ERROR("Malloc error !\n");
	}
	read_base_infos(&parser, res);
	read_sections(&parser, res);
	scale_debug(res);
	yaml_parser_delete(&parser);
	fclose(fd);
	return res;
}
