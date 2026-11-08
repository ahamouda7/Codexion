/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahamouda <ahamouda@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:08:57 by ahamouda          #+#    #+#             */
/*   Updated: 2026/11/08 12:08:58 by ahamouda         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "heap_internal.h"

int	request_before(const t_heap *heap, const t_request *a, const t_request *b)
{
	if (heap->policy == EDF && a->deadline != b->deadline)
		return (a->deadline < b->deadline);
	if (a->arrival_order != b->arrival_order)
		return (a->arrival_order < b->arrival_order);
	return (a->coder_id < b->coder_id);
}

void	swap_requests(t_request *a, t_request *b)
{
	t_request	temp;

	temp = *a;
	*a = *b;
	*b = temp;
}
