#ifndef __SCALE__
# define __SCALE__

# include <nuklear.h>
# include <stdlib.h>
# include <stdio.h>
# include <stddef.h>
# include <execinfo.h>
# include <unistd.h>

# define ERROR(str) printf("\033[1;31mFATAL ERROR:\033[0m 42_scale must stop. \n\033[0;32m> Reason: \033[0m %s\n", str); print_trace(); _exit(1);

/* HELPERS (helpers.c) */

void	print_trace(void);

#endif
