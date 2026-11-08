/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahamouda <ahamouda@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:00:00 by your_login        #+#    #+#             */
/*   Updated: 2026/11/08 12:09:04 by ahamouda         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	heap_init(t_heap *heap, size_t capacity, t_scheduler policy)
{
	if (!heap || capacity == 0 || capacity > SIZE_MAX / sizeof(*heap->items))
		return (0);
	if (policy != FIFO && policy != EDF)
		return (0);
	heap->items = malloc(capacity * sizeof(*heap->items));
	if (!heap->items)
		return (0);
	heap->size = 0;
	heap->capacity = capacity;
	heap->policy = policy;
	return (1);
}

void	heap_destroy(t_heap *heap)
{
	free(heap->items);
	heap->items = NULL;
	heap->size = 0;
	heap->capacity = 0;
}

int	heap_peek(const t_heap *heap, t_request *request)
{
	if (heap->size == 0 || !request)
		return (0);
	*request = heap->items[0];
	return (1);
}
