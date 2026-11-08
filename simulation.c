/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simulation.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahamouda <ahamouda@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 09:21:36 by ahamouda          #+#    #+#             */
/*   Updated: 2026/11/08 09:21:37 by ahamouda         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	*coder_routine(void *arg)
{
	t_coder			*coder;
	t_simulation	*sim;
	long long		elapsed;

	coder = (t_coder *)arg;
	sim = coder->simulation;
	if (pthread_mutex_lock(&sim->print_mutex) != 0)
		return (NULL);
	elapsed = current_time_ms();
	if (elapsed >= 0)
		printf("%lld %d coder thread started\n", elapsed - sim->start_ms,
			coder->id);
	else
		fprintf(stderr, "Error: could not read time\n");
	pthread_mutex_unlock(&sim->print_mutex);
	return (NULL);
}

static int	join_coders(t_coder *coders, int count)
{
	int	i;
	int	failed;

	i = 0;
	failed = 0;
	while (i < count)
	{
		if (pthread_join(coders[i].thread, NULL) != 0)
			failed = 1;
		i++;
	}
	return (!failed);
}

static int	start_coders(t_simulation *sim, t_coder *coders)
{
	int	i;

	i = 0;
	while (i < sim->config.number_of_coders)
	{
		init_coder(&coders[i], sim, i);
		if (pthread_create(&coders[i].thread, NULL, coder_routine,
				&coders[i]) != 0)
			break ;
		i++;
	}
	return (i);
}

static t_simulation	*create_simulation(t_config config)
{
	t_simulation	*sim;

	sim = malloc(sizeof(*sim));
	if (!sim)
		return (NULL);
	sim->config = config;
	sim->start_ms = current_time_ms();
	if (sim->start_ms < 0 || pthread_mutex_init(&sim->print_mutex, NULL) != 0)
	{
		free(sim);
		return (NULL);
	}
	if (!init_dongles(sim))
	{
		pthread_mutex_destroy(&sim->print_mutex);
		free(sim);
		return (NULL);
	}
	return (sim);
}

int	run_simulation(t_config config)
{
	t_simulation	*sim;
	t_coder			*coders;
	int				created;

	if ((size_t)config.number_of_coders > SIZE_MAX / sizeof(*coders))
		return (0);
	coders = malloc(sizeof(*coders) * (size_t)config.number_of_coders);
	if (!coders)
		return (0);
	sim = create_simulation(config);
	if (!sim)
	{
		free(coders);
		return (0);
	}
	created = start_coders(sim, coders);
	if (!join_coders(coders, created))
		return (0);
	destroy_dongles(sim, config.number_of_coders);
	pthread_mutex_destroy(&sim->print_mutex);
	free(sim);
	free(coders);
	return (created == config.number_of_coders);
}
