#include "codexion.h"

static int	init_locks(t_simulation *sim)
{
	if (pthread_mutex_init(&sim->state_mutex, NULL) != 0)
		return (0);
	if (pthread_cond_init(&sim->changed, NULL) != 0)
	{
		pthread_mutex_destroy(&sim->state_mutex);
		return (0);
	}
	if (pthread_mutex_init(&sim->print_mutex, NULL) != 0)
	{
		pthread_cond_destroy(&sim->changed);
		pthread_mutex_destroy(&sim->state_mutex);
		return (0);
	}
	return (1);
}

static void	destroy_locks(t_simulation *sim)
{
	pthread_mutex_destroy(&sim->print_mutex);
	pthread_cond_destroy(&sim->changed);
	pthread_mutex_destroy(&sim->state_mutex);
}

t_simulation	*create_simulation(t_config config)
{
	t_simulation	*sim;

	if ((size_t)config.number_of_coders > SIZE_MAX / sizeof(t_coder))
		return (NULL);
	sim = malloc(sizeof(*sim));
	if (!sim)
		return (NULL);
	memset(sim, 0, sizeof(*sim));
	sim->config = config;
	sim->coders = malloc(sizeof(t_coder) * (size_t)config.number_of_coders);
	if (!sim->coders || !init_locks(sim))
	{
		free(sim->coders);
		free(sim);
		return (NULL);
	}
	if (!init_dongles(sim))
	{
		destroy_locks(sim);
		free(sim->coders);
		free(sim);
		return (NULL);
	}
	return (sim);
}

void	destroy_simulation(t_simulation *sim)
{
	destroy_dongles(sim, sim->config.number_of_coders);
	destroy_locks(sim);
	free(sim->coders);
	free(sim);
}
