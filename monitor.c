#include "codexion.h"

static long long	check_coders(t_simulation *sim, long long now)
{
	long long	deadline;
	long long	first;
	int			i;

	i = 0;
	first = LLONG_MAX;
	while (i < sim->config.number_of_coders)
	{
		deadline = sim->start_ms + sim->coders[i].last_compile
			+ sim->config.time_to_burnout;
		if (now >= deadline)
		{
			log_state(&sim->coders[i], "burned out", now);
			sim->stopped = 1;
			pthread_cond_broadcast(&sim->changed);
			return (0);
		}
		if (deadline < first)
			first = deadline;
		i++;
	}
	return (first);
}

void	*monitor_routine(void *arg)
{
	t_simulation	*sim;
	long long		now;
	long long		deadline;

	sim = (t_simulation *)arg;
	pthread_mutex_lock(&sim->state_mutex);
	while (!sim->started && !sim->stopped)
		pthread_cond_wait(&sim->changed, &sim->state_mutex);
	while (!sim->stopped)
	{
		now = current_time_ms();
		if (now < 0)
		{
			fail_simulation(sim);
			break ;
		}
		deadline = check_coders(sim, now);
		if (!sim->stopped)
			wait_until(sim, deadline);
	}
	pthread_mutex_unlock(&sim->state_mutex);
	return (NULL);
}
