#ifndef __SCALE__
# define __SCALE__

# include <nuklear.h>
# include <stdlib.h>
# include <stdio.h>
# include <stddef.h>
# include <execinfo.h>
# include <unistd.h>
# include <yaml.h>

# define ERROR(...) printf("\033[1;31mFATAL ERROR:\033[0m 42_scale must stop. \n\033[0;31m> Reason: \033[0m"); printf(__VA_ARGS__); print_trace(); _exit(1);

enum {
	LANG_FR,
	LANG_EN,
	LANG_RU
};

enum {
	R_BOOL,
	R_MULTI,
	R_STAND,
	R_BONUS
};

/* Some typedef for clarity.
 * NOTE: Those typedefs are not used in structures declaration, for type clarity.
 * NOTE: MD = Markdown
*/

typedef struct s_skills			scale_skills;
typedef struct s_questions		scale_questions;
typedef struct s_sections		scale_sections;
typedef struct s_scale			scale;

struct		s_skills {
	int					percent; // Percentage (1-100) value of the skill
	char				*name; // Skill name
	struct s_skills		*next; // Pointer to the next skill
};

struct		s_questions {
	char				*name; // Name of the question
	char				*guidelines; // Guidelines of the question (MD)
	int					rating; // Rating of the question (R_BOOL | R_MULTI)
	int					kind; // Type of question (R_STAND | R_BONUS)
	struct s_skills		*skills; // Pointer to s_skills struct
	struct s_questions	*next; // Pointer to the next question
};

struct		s_sections {
	char				*name; // Name of the section
	char				*description; // Description of the section
	struct s_questions	*questions; // Pointer to s_questions struct
	struct s_sections	*next; // Pointer to the next section
};

struct		s_scale {
	char				*name; // Name of the subject
	int					lang; // Language of the scale (Enum value LANG_*)
	char				*comment; // Comment about the scale
	char				*intro; // Introduction about the scale (MD)
	char				*disclaimer; // Disclaimer about the scale (MD)
	char				*guidelines; // Guidelines of the scale (MD)
	int					correction_n; // Corrections number
	struct s_sections	*sections; // Pointer to s_sections structs
};

/* HELPERS (helpers.c) */

void	print_trace(void);
char	*m_strcpy(yaml_token_t token);
void	scale_debug(scale *res);

/* YAML (yaml.c) */

scale	*read_scale(FILE *fd);

#endif
