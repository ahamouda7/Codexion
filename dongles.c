/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: your_login <your_login@student.42.fr>     +#+  +:+       +#+         */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:00:00 by your_login        #+#    #+#             */
/*   Updated: 2026/11/08 12:00:00 by your_login       ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	init_dongle(t_dongle *dongle, t_scheduler policy)
{
	if (pthread_mutex_init(&dongle->mutex, NULL) != 0)
		return (0);
	if (!heap_init(&dongle->waiting, 2, policy))
	{
		pthread_mutex_destroy(&dongle->mutex);
		return (0);
	}
	dongle->in_use = 0;
	dongle->available_at = 0;
	return (1);
}

void	destroy_dongles(t_simulation *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		heap_destroy(&sim->dongles[i].waiting);
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		i++;
	}
	free(sim->dongles);
	sim->dongles = NULL;
}

int	init_dongles(t_simulation *sim)
{
	int	i;

	if ((size_t)sim->config.number_of_coders > SIZE_MAX / sizeof(*sim->dongles))
		return (0);
	sim->dongles = malloc(sizeof(*sim->dongles) * sim->config.number_of_coders);
	if (!sim->dongles)
		return (0);
	i = 0;
	while (i < sim->config.number_of_coders)
	{
		if (!init_dongle(&sim->dongles[i], sim->config.scheduler))
		{
			destroy_dongles(sim, i);
			return (0);
		}
		i++;
	}
	return (1);
}

void	init_coder(t_coder *coder, t_simulation *sim, int index)
{
	coder->id = index + 1;
	coder->simulation = sim;
	coder->last_compile = 0;
	coder->compiles = 0;
	coder->first_dongle = &sim->dongles[index];
	coder->second_dongle = NULL;
	if (sim->config.number_of_coders == 1)
		return ;
	if (index == 0)
		coder->second_dongle = &sim->dongles[sim->config.number_of_coders - 1];
	else
		coder->second_dongle = &sim->dongles[index - 1];
}
