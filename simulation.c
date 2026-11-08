#include "codexion.h"

static int	join_coders(t_simulation *sim, int count)
{
	int	i;
	int	ok;

	i = 0;
	ok = 1;
	while (i < count)
	{
		if (pthread_join(sim->coders[i].thread, NULL) != 0)
			ok = 0;
		i++;
	}
	return (ok);
}

static void	start_simulation(t_simulation *sim, int created, int monitor_ok)
{
	pthread_mutex_lock(&sim->state_mutex);
	if (created != sim->config.number_of_coders || !monitor_ok)
	{
		sim->failed = 1;
		sim->stopped = 1;
	}
	else
	{
		sim->start_ms = current_time_ms();
		if (sim->start_ms < 0)
		{
			sim->failed = 1;
			sim->stopped = 1;
		}
		if (sim->config.number_of_compiles_required == 0)
			sim->stopped = 1;
		sim->started = 1;
	}
	pthread_cond_broadcast(&sim->changed);
	pthread_mutex_unlock(&sim->state_mutex);
}

static int	launch_threads(t_simulation *sim)
{
	int	i;
	int	monitor_ok;
	int	joined;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		init_coder(&sim->coders[i], sim, i);
		if (pthread_create(&sim->coders[i].thread, NULL, coder_routine,
				&sim->coders[i]) != 0)
			break ;
		i++;
	}
	monitor_ok = 0;
	if (i == sim->config.number_of_coders)
		monitor_ok = (pthread_create(&sim->monitor, NULL, monitor_routine,
					sim) == 0);
	start_simulation(sim, i, monitor_ok);
	joined = join_coders(sim, i);
	if (monitor_ok && pthread_join(sim->monitor, NULL) != 0)
		joined = 0;
	if (!joined)
		return (-1);
	return (!sim->failed);
}

int	run_simulation(t_config config)
{
	t_simulation	*sim;
	int				result;

	sim = create_simulation(config);
	if (!sim)
		return (0);
	result = launch_threads(sim);
	if (result < 0)
		return (0);
	destroy_simulation(sim);
	return (result);
}
