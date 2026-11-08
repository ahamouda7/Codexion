/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: your_login <your_login@student.42.fr>     +#+  +:+       +#+         */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:00:00 by your_login        #+#    #+#             */
/*   Updated: 2026/11/08 12:00:00 by your_login       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static t_coder	*earliest_coder(t_simulation *sim)
{
	t_coder	*first;
	int		i;

	first = &sim->coders[0];
	i = 1;
	while (i < sim->config.number_of_coders)
	{
		if (sim->coders[i].last_compile < first->last_compile)
			first = &sim->coders[i];
		i++;
	}
	return (first);
}

static void	check_coders(t_simulation *sim, long long now)
{
	t_coder		*first;
	long long	deadline;

	first = earliest_coder(sim);
	deadline = sim->start_ms + first->last_compile
		+ sim->config.time_to_burnout;
	if (now >= deadline)
	{
		sim->stopped = 1;
		log_state(first, "burned out", now);
		pthread_cond_broadcast(&sim->changed);
	}
	else if (deadline != sim->next_deadline)
	{
		sim->next_deadline = deadline;
		pthread_cond_broadcast(&sim->changed);
	}
}

void	*monitor_routine(void *arg)
{
	t_simulation	*sim;
	long long		now;

	sim = (t_simulation *)arg;
	pthread_mutex_lock(&sim->state_mutex);
	while (!sim->started && !sim->stopped)
		wait_for_signal(sim);
	while (!sim->stopped)
	{
		now = current_time_ms();
		if (now < 0)
			fail_simulation(sim);
		else
			check_coders(sim, now);
		if (!sim->stopped)
			wait_until(sim, sim->next_deadline);
	}
	pthread_mutex_unlock(&sim->state_mutex);
	return (NULL);
}
