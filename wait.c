#include "codexion.h"

void	fail_simulation(t_simulation *sim)
{
	sim->failed = 1;
	sim->stopped = 1;
	pthread_cond_broadcast(&sim->changed);
}

void	wait_until(t_simulation *sim, long long target)
{
	struct timespec	timeout;
	int				result;

	timeout.tv_sec = (time_t)(target / 1000);
	timeout.tv_nsec = (long)((target % 1000) * 1000000);
	result = pthread_cond_timedwait(&sim->changed, &sim->state_mutex, &timeout);
	if (result != 0 && result != ETIMEDOUT)
		fail_simulation(sim);
}

void	wait_for_change(t_coder *coder, long long now)
{
	t_simulation	*sim;
	long long		first;
	long long		second;
	long long		next;

	sim = coder->simulation;
	lock_pair(coder);
	first = sim->start_ms + coder->first_dongle->available_at;
	second = sim->start_ms + coder->second_dongle->available_at;
	unlock_pair(coder);
	next = LLONG_MAX;
	if (first > now)
		next = first;
	if (second > now && second < next)
		next = second;
	if (next != LLONG_MAX)
		wait_until(sim, next);
	else if (pthread_cond_wait(&sim->changed, &sim->state_mutex) != 0)
		fail_simulation(sim);
}

void	log_state(t_coder *coder, const char *message, long long now)
{
	t_simulation	*sim;

	sim = coder->simulation;
	pthread_mutex_lock(&sim->print_mutex);
	printf("%lld %d %s\n", now - sim->start_ms, coder->id, message);
	pthread_mutex_unlock(&sim->print_mutex);
}
