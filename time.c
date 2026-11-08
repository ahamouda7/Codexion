#include "codexion.h"

long long	current_time_ms(void)
{
	struct timeval	now;

	if (gettimeofday(&now, NULL) != 0)
		return (-1);
	return ((long long)now.tv_sec * 1000 + now.tv_usec / 1000);
}
