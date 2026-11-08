/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   phase.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahamouda <ahamouda@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:09:37 by ahamouda          #+#    #+#             */
/*   Updated: 2026/11/08 12:09:38 by ahamouda         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	wait_for_end(t_simulation *sim, long long now,
		long long end)
{
	while (!sim->stopped && now < end)
	{
		wait_until(sim, end);
		now = simulation_time(sim);
	}
}

int	perform_phase(t_coder *coder, const char *message, int duration)
{
	t_simulation	*sim;
	long long		now;
	long long		end;
	int				ok;

	sim = coder->simulation;
	pthread_mutex_lock(&sim->state_mutex);
	now = simulation_time(sim);
	if (!sim->stopped && message)
		log_state(coder, message, now);
	end = now + duration;
	if (!message)
		end = sim->start_ms + coder->last_compile + duration;
	wait_for_end(sim, now, end);
	ok = !sim->stopped;
	pthread_mutex_unlock(&sim->state_mutex);
	return (ok);
}

int	finish_compile(t_coder *coder)
{
	t_simulation	*sim;
	int				ok;

	sim = coder->simulation;
	pthread_mutex_lock(&sim->state_mutex);
	simulation_time(sim);
	release_dongles(coder);
	if (!sim->stopped
		&& coder->compiles < sim->config.number_of_compiles_required)
	{
		coder->compiles++;
		if (coder->compiles == sim->config.number_of_compiles_required)
			sim->finished++;
		if (sim->finished == sim->config.number_of_coders)
		{
			sim->stopped = 1;
			pthread_cond_broadcast(&sim->changed);
		}
	}
	ok = !sim->stopped;
	pthread_mutex_unlock(&sim->state_mutex);
	return (ok);
}
