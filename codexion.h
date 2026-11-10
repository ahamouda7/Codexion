/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ahamouda <ahamouda@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/08 12:07:48 by ahamouda          #+#    #+#             */
/*   Updated: 2026/11/10 16:08:27 by ahamouda         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <limits.h>
# include <errno.h>
# include <pthread.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

typedef enum e_scheduler
{
	FIFO,
	EDF
}	t_scheduler;

typedef struct s_config
{
	int			number_of_coders;
	int			time_to_burnout;
	int			time_to_compile;
	int			time_to_debug;
	int			time_to_refactor;
	int			number_of_compiles_required;
	int			dongle_cooldown;
	t_scheduler	scheduler;
}	t_config;

typedef struct s_request
{
	int					coder_id;
	unsigned long long	arrival_order;
	long long			deadline;
}	t_request;

typedef struct s_heap
{
	t_request	*items;
	size_t		size;
	size_t		capacity;
	t_scheduler	policy;
}	t_heap;

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	int				in_use;
	long long		available_at;
	t_heap			waiting;
}	t_dongle;

typedef struct s_simulation
{
	t_config			config;
	long long			start_ms;
	long long			next_deadline;
	pthread_mutex_t		print_mutex;
	pthread_mutex_t		state_mutex;
	pthread_cond_t		changed;
	pthread_t			monitor;
	t_dongle			*dongles;
	struct s_coder		*coders;
	unsigned long long	next_order;
	int					started;
	int					stopped;
	int					failed;
	int					finished;
}	t_simulation;

typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	t_simulation	*simulation;
	t_dongle		*first_dongle;
	t_dongle		*second_dongle;
	long long		last_compile;
	int				compiles;
}	t_coder;

int					parse_arguments(int argc, char **argv, t_config *config);
int					run_simulation(t_config config);
long long			current_time_ms(void);
int					heap_init(t_heap *heap, size_t capacity,
						t_scheduler policy);
void				heap_destroy(t_heap *heap);
int					heap_peek(const t_heap *heap, t_request *request);
int					heap_push(t_heap *heap, t_request request);
int					heap_pop(t_heap *heap, t_request *request);
int					init_dongles(t_simulation *sim);
void				destroy_dongles(t_simulation *sim, int count);
void				init_coder(t_coder *coder, t_simulation *sim, int index);
void				*coder_routine(void *arg);
void				*monitor_routine(void *arg);
int					take_dongles(t_coder *coder);
void				release_dongles(t_coder *coder);
void				wait_until(t_simulation *sim, long long target);
void				wait_for_change(t_coder *coder, long long now);
void				log_state(t_coder *coder, const char *message,
						long long now);
int					perform_phase(t_coder *coder, const char *message,
						int duration);
int					finish_compile(t_coder *coder);
t_simulation		*create_simulation(t_config config);
void				destroy_simulation(t_simulation *sim);
void				lock_pair(t_coder *coder);
void				unlock_pair(t_coder *coder);
void				fail_simulation(t_simulation *sim);
void				wait_for_signal(t_simulation *sim);
long long			simulation_time(t_simulation *sim);
int					enqueue_request(t_coder *coder);
int					pair_ready(t_coder *coder, long long now);

#endif
