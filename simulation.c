/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: your_login <your_login@student.42.fr>     +#+  +:+       +#+         */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:00:00 by your_login        #+#    #+#             */
/*   Updated: 2026/11/08 12:00:00 by your_login       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	join_coders(t_simulation *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
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
		sim->next_deadline = sim->start_ms + sim->config.time_to_burnout;
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
	join_coders(sim, i);
	if (monitor_ok)
		pthread_join(sim->monitor, NULL);
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
	destroy_simulation(sim);
	return (result);
}
