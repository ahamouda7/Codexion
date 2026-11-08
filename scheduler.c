#include "codexion.h"

static int	wait_alone(t_coder *coder)
{
	t_simulation	*sim;
	long long		now;

	sim = coder->simulation;
	now = current_time_ms();
	if (now >= 0 && now - sim->start_ms < sim->config.time_to_burnout)
	{
		pthread_mutex_lock(&coder->first_dongle->mutex);
		coder->first_dongle->in_use = 1;
		pthread_mutex_unlock(&coder->first_dongle->mutex);
		log_state(coder, "has taken a dongle", now);
	}
	while (!sim->stopped)
		pthread_cond_wait(&sim->changed, &sim->state_mutex);
	return (0);
}

static void	claim_pair(t_coder *coder, long long now)
{
	t_request		removed;
	t_simulation	*sim;

	sim = coder->simulation;
	heap_pop(&coder->first_dongle->waiting, &removed);
	heap_pop(&coder->second_dongle->waiting, &removed);
	coder->first_dongle->in_use = 1;
	coder->second_dongle->in_use = 1;
	coder->last_compile = now - sim->start_ms;
	log_state(coder, "has taken a dongle", now);
	log_state(coder, "has taken a dongle", now);
	log_state(coder, "is compiling", now);
	pthread_cond_broadcast(&sim->changed);
}

static int	wait_for_pair(t_coder *coder)
{
	t_simulation	*sim;
	long long		now;
	int				ready;

	sim = coder->simulation;
	while (!sim->stopped)
	{
		now = current_time_ms();
		if (now < 0)
			break ;
		lock_pair(coder);
		ready = (now - sim->start_ms < coder->last_compile
				+ sim->config.time_to_burnout
				&& pair_ready(coder, now - sim->start_ms));
		if (ready)
			claim_pair(coder, now);
		unlock_pair(coder);
		if (ready)
			return (1);
		wait_for_change(coder, now);
	}
	if (!sim->stopped)
		fail_simulation(sim);
	return (0);
}

static int	join_queue(t_coder *coder)
{
	int	ok;

	lock_pair(coder);
	ok = enqueue_request(coder);
	unlock_pair(coder);
	if (!ok)
	{
		fail_simulation(coder->simulation);
		return (0);
	}
	return (wait_for_pair(coder));
}

int	take_dongles(t_coder *coder)
{
	t_simulation	*sim;
	int				ok;

	sim = coder->simulation;
	pthread_mutex_lock(&sim->state_mutex);
	if (sim->stopped)
	{
		pthread_mutex_unlock(&sim->state_mutex);
		return (0);
	}
	if (!coder->second_dongle)
		ok = wait_alone(coder);
	else
		ok = join_queue(coder);
	pthread_mutex_unlock(&sim->state_mutex);
	return (ok);
}
