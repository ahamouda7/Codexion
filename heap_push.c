#include "heap_internal.h"

int	heap_push(t_heap *heap, t_request request)
{
	size_t	index;
	size_t	parent;

	if (heap->size == heap->capacity)
		return (0);
	index = heap->size;
	heap->items[index] = request;
	heap->size++;
	while (index > 0)
	{
		parent = (index - 1) / 2;
		if (!request_before(heap, &heap->items[index], &heap->items[parent]))
			break ;
		swap_requests(&heap->items[index], &heap->items[parent]);
		index = parent;
	}
	return (1);
}
