#include <sys/time.h>

#include "log.h"

int g_log_fd = 0;
int g_containerd = 0;

char *log_level_to_str(int level)
{
    switch (level) {
        case L_DEBUG:
            return ("DEBUG");
        case L_INFO:
            return ("INFO");
        case L_WARN:
            return ("WARN");
        case L_ERROR:
            return ("ERROR");
        default:
            return ("UNKNOWN");
    }
}

void logger(int fd, int level, char *file, int line, const char *fmt, ...) {
    va_list vlist;
    char *timestamp;
    char *info;
    char *msg;

    if (fd == 0) {
        // init here
       if (logger_init(L_DEBUG, "/var/log/liblonghorn.log") < 0) {
              perror("Failed to init logger\n");
              return;
       }
       // re-assigned fd
       fd = g_log_fd;
    }

    if (g_log_level < level) {
        return;
    }
    
    timestamp = (g_containerd == 1) ? "" : logger_get_time();
    asprintf(&info, "%s%s:%d %s ", timestamp, file, line, log_level_to_str(level));

    va_start(vlist, fmt);
	vasprintf(&msg, fmt, vlist);
	va_end(vlist);

	dprintf(fd, "%s%s\n", info, msg);

	free(info);
	free(msg);
    if (g_containerd == 0) {
	    free(timestamp); 
    }

}

char    *logger_get_time(void)
{
	time_t		tm;
    int millisec;
	char		*buf;
	struct tm	*tm_info;
    struct timeval current;

    gettimeofday(&current, NULL);
    millisec = current.tv_usec/1000.0;

	if (!(buf = (char *)malloc(sizeof(char) * 64)))
		return (NULL);
	tm = time(NULL);
	tm_info = localtime(&tm);
	strftime(buf, 64, "%y-%m-%dT%H:%M:%S", tm_info);
    sprintf(buf, "%s.%03d ", buf, millisec);

	return (buf);
}

int logger_init(int level, char *log_file)
{
#ifdef BUILD_FOR_CONTAINER
    g_containerd = 1;
    log_file = "/proc/1/fd/1";  // It will use the containerd procress stdout for logging 
#endif

    if (logger_init_open_file(log_file) < 0)
        return (-1);
    g_log_level = level;
    return (0);
}

int	logger_init_open_file(char *log_file)
{
	if (log_file == NULL)
		g_log_fd = -1;
	g_log_fd = open(log_file, O_WRONLY | O_APPEND | O_CREAT, 0755);
	return (g_log_fd);
}

int logger_close(void)
{
    // do not close fd if it is containerd
    if (g_containerd == 1)
        return (0);
    return (close(g_log_fd));
}
