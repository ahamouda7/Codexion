/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahamouda <ahamouda@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:09:31 by ahamouda          #+#    #+#             */
/*   Updated: 2026/11/08 12:09:32 by ahamouda         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	parse_number(const char *text, int *result)
{
	int	value;
	int	digit;

	value = 0;
	if (*text == '+')
		text++;
	if (*text == '\0')
		return (0);
	while (*text)
	{
		if (*text < '0' || *text > '9')
			return (0);
		digit = *text - '0';
		if (value > (INT_MAX - digit) / 10)
			return (0);
		value = value * 10 + digit;
		text++;
	}
	*result = value;
	return (1);
}

static int	parse_scheduler(const char *text, t_scheduler *scheduler)
{
	if (strcmp(text, "fifo") == 0)
		*scheduler = FIFO;
	else if (strcmp(text, "edf") == 0)
		*scheduler = EDF;
	else
		return (0);
	return (1);
}

int	parse_arguments(int argc, char **argv, t_config *config)
{
	if (argc != 9)
		return (0);
	if (!parse_number(argv[1], &config->number_of_coders)
		|| config->number_of_coders == 0
		|| !parse_number(argv[2], &config->time_to_burnout)
		|| !parse_number(argv[3], &config->time_to_compile)
		|| !parse_number(argv[4], &config->time_to_debug)
		|| !parse_number(argv[5], &config->time_to_refactor)
		|| !parse_number(argv[6], &config->number_of_compiles_required)
		|| !parse_number(argv[7], &config->dongle_cooldown)
		|| !parse_scheduler(argv[8], &config->scheduler))
		return (0);
	return (1);
}
