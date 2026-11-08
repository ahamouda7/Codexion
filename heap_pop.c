/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_pop.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahamouda <ahamouda@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 09:22:32 by ahamouda          #+#    #+#             */
/*   Updated: 2026/11/08 09:22:33 by ahamouda         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "heap_internal.h"

static size_t	first_child(t_heap *heap, size_t index)
{
	size_t	left;
	size_t	right;

	left = index * 2 + 1;
	right = left + 1;
	if (right < heap->size && request_before(heap, &heap->items[right],
			&heap->items[left]))
		return (right);
	return (left);
}

int	heap_pop(t_heap *heap, t_request *request)
{
	size_t	index;
	size_t	child;

	if (heap->size == 0 || !request)
		return (0);
	*request = heap->items[0];
	heap->size--;
	if (heap->size == 0)
		return (1);
	heap->items[0] = heap->items[heap->size];
	index = 0;
	while (index < heap->size / 2)
	{
		child = first_child(heap, index);
		if (!request_before(heap, &heap->items[child], &heap->items[index]))
			break ;
		swap_requests(&heap->items[index], &heap->items[child]);
		index = child;
	}
	return (1);
}
