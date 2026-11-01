#include "codexion.h"

static void	print_config(const t_config *config)
{
	printf("Configuration accepted:\n");
	printf("coders: %d\n", config->number_of_coders);
	printf("burnout: %d ms\n", config->time_to_burnout);
	printf("compile: %d ms\n", config->time_to_compile);
	printf("debug: %d ms\n", config->time_to_debug);
	printf("refactor: %d ms\n", config->time_to_refactor);
	printf("required compiles: %d\n", config->number_of_compiles_required);
	printf("dongle cooldown: %d ms\n", config->dongle_cooldown);
	if (config->scheduler == FIFO)
		printf("scheduler: fifo\n");
	else
		printf("scheduler: edf\n");
}

int	main(int argc, char **argv)
{
	t_config	config;

	if (!parse_arguments(argc, argv, &config))
	{
		fprintf(stderr, "Error: invalid arguments\n");
		return (1);
	}
	print_config(&config);
	return (0);
}
