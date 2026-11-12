/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahamouda <ahamouda@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:17:46 by ahamouda          #+#    #+#             */
/*   Updated: 2026/11/08 12:17:57 by ahamouda         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	append_number(char *line, int *length, long long number)
{
	if (number < 0)
	{
		line[(*length)++] = '-';
		number = -number;
	}
	if (number >= 10)
		append_number(line, length, number / 10);
	line[(*length)++] = '0' + number % 10;
}

static int	write_line(char *line, int length)
{
	int		sent;
	ssize_t	written;

	sent = 0;
	while (sent < length)
	{
		written = write(STDOUT_FILENO, line + sent, length - sent);
		if (written < 0 && errno == EINTR)
			continue ;
		if (written <= 0)
			return (0);
		sent += written;
	}
	return (1);
}

void	log_state(t_coder *coder, const char *message, long long now)
{
	t_simulation	*sim;
	char			line[80];
	int				length;

	sim = coder->simulation;
	length = 0;
	append_number(line, &length, now - sim->start_ms);
	line[length++] = ' ';
	append_number(line, &length, coder->id);
	line[length++] = ' ';
	while (*message)
		line[length++] = *message++;
	line[length++] = '\n';
	pthread_mutex_lock(&sim->print_mutex);
	if (!write_line(line, length))
		fail_simulation(sim);
	pthread_mutex_unlock(&sim->print_mutex);
}
