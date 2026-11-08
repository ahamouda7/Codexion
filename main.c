#include "codexion.h"

int	main(int argc, char **argv)
{
	t_config	config;

	if (!parse_arguments(argc, argv, &config))
	{
		fprintf(stderr, "Error: invalid arguments\n");
		return (1);
	}
	if (!run_simulation(config))
	{
		fprintf(stderr, "Error: simulation failed\n");
		return (1);
	}
	return (0);
}
