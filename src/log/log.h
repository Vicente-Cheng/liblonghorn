#ifndef _LOG_H
#define _LOG_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>


/*
 * Macro to get the basename of the __FILE__ macro.
 */

# define __FILENAME__	\
	(strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

/*
 * Log levels.
 */

# define L_ERROR		0
# define L_WARN			1
# define L_INFO			2
# define L_DEBUG		3

/*
 * Functions to use for display log messages.
 */

#define log_error(...)	logger(g_log_fd, L_ERROR, __FILENAME__, __LINE__, __VA_ARGS__)
#define log_warn(...)	logger(g_log_fd, L_WARN, __FILENAME__, __LINE__, __VA_ARGS__)
#define log_info(...)	logger(g_log_fd, L_INFO, __FILENAME__, __LINE__, __VA_ARGS__)
#define log_debug(...)	logger(g_log_fd, L_DEBUG, __FILENAME__, __LINE__, __VA_ARGS__)

/*
** Global variables, the file descriptor to write on, and the log level.
*/

int g_log_fd;
int	g_log_level;
int	g_containerd;

/*
** Tools functions.
*/

extern void		logger(int fd, int level, char *file, int line, const char *fmt, ...);
extern int		logger_close(void);
extern int		logger_init(int level, char *path);
extern char		*logger_get_time(void);
extern int		logger_init_open_file(char *path);

#endif
