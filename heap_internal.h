#ifndef HEAP_INTERNAL_H
# define HEAP_INTERNAL_H

# include "codexion.h"

int	request_before(const t_heap *heap, const t_request *a, const t_request *b);
void	swap_requests(t_request *a, t_request *b);

#endif
