/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahamouda <ahamouda@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 09:22:19 by ahamouda          #+#    #+#             */
/*   Updated: 2026/11/08 09:22:20 by ahamouda         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	print_config(t_config config)
{
	printf("Configuration accepted:\n");
	printf("coders: %d\n", config.number_of_coders);
	printf("burnout: %d ms\n", config.time_to_burnout);
	printf("compile: %d ms\n", config.time_to_compile);
	printf("debug: %d ms\n", config.time_to_debug);
	printf("refactor: %d ms\n", config.time_to_refactor);
	printf("required compiles: %d\n", config.number_of_compiles_required);
	printf("dongle cooldown: %d ms\n", config.dongle_cooldown);
	if (config.scheduler == FIFO)
		printf("scheduler: fifo\n");
	else
		printf("scheduler: edf\n");
}

int	main(int argc, char **argv)
{
	t_config	config;

	if (!parse_arguments(argc, argv, &config))
	{
		fprintf(stderr, "Error: invalid arguments\n");
		return (1);
	}
	print_config(config);
	if (!run_simulation(config))
	{
		fprintf(stderr, "Error: could not run coder threads\n");
		return (1);
	}
	return (0);
}
