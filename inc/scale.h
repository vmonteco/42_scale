#ifndef __SCALE__
# define __SCALE__

# include <stdlib.h>
# include <stdio.h>
# include <stddef.h>
# include <execinfo.h>
# include <unistd.h>
# include <yaml.h>
# include <stdbool.h>
# include <GL/glew.h>
# include <GLFW/glfw3.h>
# include <math.h>
# include <assert.h>

# define ERROR(...) printf("\033[1;31mFATAL ERROR:\033[0m 42_scale must stop. \n\033[0;31m> Reason: \033[0m"); printf(__VA_ARGS__); print_trace(); _exit(1);
# define WINDOW_WIDTH 800
# define WINDOW_HEIGHT 600
# define MAX_INPUTS 9

enum {
	LANG_FR,
	LANG_EN,
	LANG_RU
};

enum {
	R_BOOL,
	R_MULTI
};

enum {
	R_STAND,
	R_BONUS
};

/* Some typedef for clarity.
 * NOTE: Those typedefs are not used in structures declaration, for type clarity.
 * NOTE: MD = Markdown
*/

typedef struct s_entry			scale_entry;
typedef struct s_skills			scale_skills;
typedef struct s_questions		scale_questions;
typedef struct s_sections		scale_sections;
typedef struct s_scale			scale;

struct		s_entry {
	char				*buf; // Entry buffer
	int					len; // Len of the buffer
	int					val; // Alternate value
	bool				bol; // Alternate value
};

struct		s_skills {
	scale_entry			percent; // Percentage (1-100) value of the skill
	scale_entry			name; // Skill name
	struct s_skills		*next; // Pointer to the next skill
};

struct		s_questions {
	scale_entry			name; // Name of the question
	scale_entry			guidelines; // Guidelines of the question (MD)
	scale_entry			rating; // Rating of the question (R_BOOL | R_MULTI)
	scale_entry			kind; // Type of question (R_STAND | R_BONUS)
	struct s_skills		*skills; // Pointer to s_skills struct
	struct s_questions	*next; // Pointer to the next question
};

struct		s_sections {
	scale_entry			name; // Name of the section
	scale_entry			description; // Description of the section
	struct s_questions	*questions; // Pointer to s_questions struct
	struct s_sections	*next; // Pointer to the next section
};

struct		s_scale {
	scale_entry			name; // Name of the subject
	scale_entry			lang; // Language of the scale (Enum value LANG_*)
	scale_entry			comment; // Comment about the scale
	scale_entry			intro; // Introduction about the scale (MD)
	scale_entry			disclaimer; // Disclaimer about the scale (MD)
	scale_entry			guidelines; // Guidelines of the scale (MD)
	scale_entry			correction_n; // Corrections number
	scale_entry			is_primary; // Always true
	scale_entry			duration; // Duration of the scale, with a 5 multiplier (1 = 5, 2 = 10, ...)
	struct s_sections	*sections; // Pointer to s_sections structs
};



/* HELPERS (helpers.c) */

void		print_trace(void);
scale_entry	m_strcpy(yaml_token_t token);
void		scale_debug(scale *res);
int			count_lines(char *s);
void		add_section(scale_sections *s, char *name);
void		add_question(scale_sections *s, char *name, int j);
void		add_skills(scale_questions *sk, int val, char *name);

/* YAML (yaml.c) */

scale	*read_scale(FILE *fd);

/* GRAPHIC (graphic.c) */
void	window(scale *s);

#endif
