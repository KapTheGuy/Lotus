#include "monitor.h"
#include "lib/stdio.h"
#include "lib/string.h"
#include "types.h"


/* forward declarations for local functions */
static void tokenize_command(char *command_string, int *argc_pointer, char **argv);
static command_handler get_command_handler(const char *command_name);


/* 
 * Entry point for the kernel monitor. Monitor provides a simple shell with
 * a set of commands which starts when the OS boots.
 */
void
monitor(void)
{
	char command_string[BUFFER_MAX_LENGTH];
	char *argv[ARG_COUNT_MAX];
	int argc = 0;

	while (true)
	{
		printf(">: ");
		gets(command_string);
		tokenize_command(command_string, &argc, argv);

		if (argc != 0)
		{
			command_handler handler = get_command_handler(argv[0]);
			if (handler == NULL)
			{
				printf("The handler for %s was not found.\n", argv[0]);
			}
			else
			{
				handler(argc, argv);
			}
		}
	}
}


/*
 * tokenize_command splits the command_string into tokens. It returns the argument
 * count in argc_pointer, and the arguments in argv.
 */
static void
tokenize_command(char *command_string, int *argc_pointer, char **argv)
{
	char *token = NULL;
	int argc = 0;

	token = strtok(command_string, " ");
	while (token != NULL && argc < ARG_COUNT_MAX)
	{
		argv[argc] = token;
		argc++;

		token = strtok(NULL, " ");
	}

	(*argc_pointer) = argc;
}


/*
 * get_command_handler returns the handler function if command exists, and it
 * returns NULL otherwise.
 */
static command_handler
get_command_handler(const char *command_name)
{
	uint32_t command_index = 0;

	for (command_index = 0; command_index < COMMAND_COUNT; command_index++)
	{
		Command command = commands[command_index];
		if (strcmp(command.name, command_name) == 0)
		{
			return command.handler;
		}
	}

	return NULL;
}
