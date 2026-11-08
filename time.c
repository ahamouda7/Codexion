/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   time.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahamouda <ahamouda@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 09:21:18 by ahamouda          #+#    #+#             */
/*   Updated: 2026/11/08 09:21:19 by ahamouda         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long long	current_time_ms(void)
{
	struct timeval	now;

	if (gettimeofday(&now, NULL) != 0)
		return (-1);
	return ((long long)now.tv_sec * 1000 + now.tv_usec / 1000);
}
