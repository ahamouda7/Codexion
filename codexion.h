#ifndef CODEXION_H
# define CODEXION_H

# include <limits.h>
# include <pthread.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

typedef struct s_config
{
	int			number_of_coders;
	int			time_to_burnout;
	int			time_to_compile;
	int			time_to_debug;
	int			time_to_refactor;
	int			number_of_compiles_required;
	int			dongle_cooldown;
	t_scheduler	scheduler;
}	t_config;

typedef struct s_simulation
{
	t_config			config;
	long long			start_ms;
	pthread_mutex_t		print_mutex;
}	t_simulation;

typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	t_simulation	*simulation;
}	t_coder;

int	parse_arguments(int argc, char **argv, t_config *config);
int	run_simulation(t_config config);
long long	current_time_ms(void);

#endif
