/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   wait.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: your_login <your_login@student.42.fr>     +#+  +:+       +#+         */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:00:00 by your_login        #+#    #+#             */
/*   Updated: 2026/11/08 12:00:00 by your_login       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	fail_simulation(t_simulation *sim)
{
	sim->failed = 1;
	sim->stopped = 1;
	pthread_cond_broadcast(&sim->changed);
}

void	wait_for_signal(t_simulation *sim)
{
	if (pthread_cond_wait(&sim->changed, &sim->state_mutex) != 0)
		fail_simulation(sim);
}

/* Called with state_mutex locked; let the monitor handle expired deadlines. */
long long	simulation_time(t_simulation *sim)
{
	long long	now;

	now = current_time_ms();
	while (!sim->stopped && now >= sim->next_deadline)
	{
		pthread_cond_broadcast(&sim->changed);
		wait_for_signal(sim);
		now = current_time_ms();
	}
	if (now < 0)
		fail_simulation(sim);
	return (now);
}

void	wait_until(t_simulation *sim, long long target)
{
	struct timespec	timeout;
	int				result;

	timeout.tv_sec = (time_t)(target / 1000);
	timeout.tv_nsec = (long)((target % 1000) * 1000000);
	result = pthread_cond_timedwait(&sim->changed, &sim->state_mutex, &timeout);
	if (result != 0 && result != ETIMEDOUT)
		fail_simulation(sim);
}

void	wait_for_change(t_coder *coder, long long now)
{
	t_simulation	*sim;
	long long		first;
	long long		second;
	long long		next;

	sim = coder->simulation;
	lock_pair(coder);
	first = sim->start_ms + coder->first_dongle->available_at;
	second = sim->start_ms + coder->second_dongle->available_at;
	unlock_pair(coder);
	next = LLONG_MAX;
	if (first > now)
		next = first;
	if (second > now && second < next)
		next = second;
	if (next != LLONG_MAX)
		wait_until(sim, next);
	else
		wait_for_signal(sim);
}
