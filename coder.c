/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: your_login <your_login@student.42.fr>     +#+  +:+       +#+         */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:00:00 by your_login        #+#    #+#             */
/*   Updated: 2026/11/08 12:00:00 by your_login       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	wait_for_start(t_simulation *sim)
{
	pthread_mutex_lock(&sim->state_mutex);
	while (!sim->started && !sim->stopped)
		wait_for_signal(sim);
	pthread_mutex_unlock(&sim->state_mutex);
}

void	*coder_routine(void *arg)
{
	t_coder			*coder;
	t_simulation	*sim;

	coder = (t_coder *)arg;
	sim = coder->simulation;
	wait_for_start(sim);
	while (take_dongles(coder))
	{
		if (!perform_phase(coder, NULL, sim->config.time_to_compile))
		{
			pthread_mutex_lock(&sim->state_mutex);
			release_dongles(coder);
			pthread_mutex_unlock(&sim->state_mutex);
			break ;
		}
		if (!finish_compile(coder))
			break ;
		if (!perform_phase(coder, "is debugging", sim->config.time_to_debug))
			break ;
		if (!perform_phase(coder, "is refactoring",
				sim->config.time_to_refactor))
			break ;
	}
	return (NULL);
}
