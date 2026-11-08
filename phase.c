#include "codexion.h"

static int	wait_for_deadline(t_coder *coder, long long now)
{
	t_simulation	*sim;

	sim = coder->simulation;
	if (now - sim->start_ms < coder->last_compile + sim->config.time_to_burnout)
		return (1);
	pthread_cond_broadcast(&sim->changed);
	while (!sim->stopped)
		pthread_cond_wait(&sim->changed, &sim->state_mutex);
	return (0);
}

static long long	wait_for_end(t_simulation *sim, long long now,
		long long end)
{
	while (!sim->stopped && now < end)
	{
		wait_until(sim, end);
		now = current_time_ms();
		if (now < 0)
			fail_simulation(sim);
	}
	return (now);
}

int	perform_phase(t_coder *coder, const char *message, int duration)
{
	t_simulation	*sim;
	long long		now;
	long long		end;
	int				ok;

	sim = coder->simulation;
	pthread_mutex_lock(&sim->state_mutex);
	now = current_time_ms();
	if (now < 0)
		fail_simulation(sim);
	if (!sim->stopped && message && wait_for_deadline(coder, now))
		log_state(coder, message, now);
	end = now + duration;
	if (!message)
		end = sim->start_ms + coder->last_compile + duration;
	now = wait_for_end(sim, now, end);
	if (!sim->stopped)
		wait_for_deadline(coder, now);
	ok = !sim->stopped;
	pthread_mutex_unlock(&sim->state_mutex);
	return (ok);
}

int	finish_compile(t_coder *coder)
{
	t_simulation	*sim;
	int				ok;

	sim = coder->simulation;
	pthread_mutex_lock(&sim->state_mutex);
	release_dongles(coder);
	if (!sim->stopped)
	{
		coder->compiles++;
		if (coder->compiles == sim->config.number_of_compiles_required)
			sim->finished++;
		if (sim->finished == sim->config.number_of_coders)
		{
			sim->stopped = 1;
			pthread_cond_broadcast(&sim->changed);
		}
	}
	ok = !sim->stopped;
	pthread_mutex_unlock(&sim->state_mutex);
	return (ok);
}
