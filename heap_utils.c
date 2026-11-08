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
