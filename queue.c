#include "codexion.h"

void	lock_pair(t_coder *coder)
{
	if (coder->first_dongle < coder->second_dongle)
	{
		pthread_mutex_lock(&coder->first_dongle->mutex);
		pthread_mutex_lock(&coder->second_dongle->mutex);
	}
	else
	{
		pthread_mutex_lock(&coder->second_dongle->mutex);
		pthread_mutex_lock(&coder->first_dongle->mutex);
	}
}

void	unlock_pair(t_coder *coder)
{
	if (coder->first_dongle < coder->second_dongle)
	{
		pthread_mutex_unlock(&coder->second_dongle->mutex);
		pthread_mutex_unlock(&coder->first_dongle->mutex);
	}
	else
	{
		pthread_mutex_unlock(&coder->first_dongle->mutex);
		pthread_mutex_unlock(&coder->second_dongle->mutex);
	}
}

int	enqueue_request(t_coder *coder)
{
	t_simulation	*sim;
	t_request		request;

	sim = coder->simulation;
	if (sim->next_order == ULLONG_MAX
		|| coder->first_dongle->waiting.size == 2
		|| coder->second_dongle->waiting.size == 2)
		return (0);
	request.coder_id = coder->id;
	request.arrival_order = sim->next_order++;
	request.deadline = coder->last_compile + sim->config.time_to_burnout;
	heap_push(&coder->first_dongle->waiting, request);
	heap_push(&coder->second_dongle->waiting, request);
	return (1);
}

int	pair_ready(t_coder *coder, long long now)
{
	t_request	first;
	t_request	second;
	t_dongle	*a;
	t_dongle	*b;

	a = coder->first_dongle;
	b = coder->second_dongle;
	if (!heap_peek(&a->waiting, &first) || !heap_peek(&b->waiting, &second))
		return (0);
	return (first.coder_id == coder->id && second.coder_id == coder->id
		&& !a->in_use && !b->in_use && now >= a->available_at
		&& now >= b->available_at);
}

void	release_dongles(t_coder *coder)
{
	t_simulation	*sim;
	long long		available;

	sim = coder->simulation;
	available = current_time_ms();
	if (available < 0)
	{
		fail_simulation(sim);
		available = sim->start_ms;
	}
	available = available - sim->start_ms + sim->config.dongle_cooldown;
	lock_pair(coder);
	coder->first_dongle->in_use = 0;
	coder->second_dongle->in_use = 0;
	coder->first_dongle->available_at = available;
	coder->second_dongle->available_at = available;
	unlock_pair(coder);
	pthread_cond_broadcast(&sim->changed);
}
