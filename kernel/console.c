#include "console.h"
#include "irq.h"
#include "lib/string.h"


/* memory mapping for the serial port */
#define UART0 ((volatile unsigned int*)0x101f1000)

/* serial port interrupt line numbers */
#define UART0_IRQ 12

/* serial port register offsets */
#define UART_DATA        0x00 
#define UART_FLAGS       0x06
#define UART_INT_ENABLE  0x0e
#define UART_INT_TARGET  0x0f
#define UART_INT_CLEAR   0x11

/* serial port bitmasks */
#define UART_RECEIVE  0x10
#define UART_TRANSMIT 0x20

/* receive buffer */
#define RECEIVE_BUFFER_SIZE 16
static char receive_buffer[RECEIVE_BUFFER_SIZE];
static int receive_buffer_head = 0;
static int receive_buffer_tail = 0;


/* forward declarations for local functions */
static void uart0_interrupt_handler(void);
static int circular_inc(int operand, int circle_size);
static bool is_special_key_sequence_prefix(char *sequence);
static int get_special_key_code(char *sequence);
static char * get_special_key_sequence(int code);


/*
 * console_init intializes the receive_buffer, then setups up the interrupt
 * handler for the serial port.
 */
void
console_init(void)
{
	receive_buffer_head = 0;
	receive_buffer_tail = 0;

	UART0[UART_INT_ENABLE] = UART_RECEIVE;
	register_interrupt_handler(UART0_IRQ, uart0_interrupt_handler);
}


/* putch writes a character to the uart0 serial port. */
void
putch(int c)
{
	int sequence_length = 0;
	int index = 0;

	char single_char_sequence[2];
	char *sequence = get_special_key_sequence(c);
	if (sequence == NULL)
	{
		sequence = single_char_sequence;
		sequence[0] = c;
		sequence[1] = 0x00;
	}

	sequence_length = strlen(sequence);

	for (index = 0; index < sequence_length; index++)
	{
		/* wait until transmit buffer is full */
		while (UART0[UART_FLAGS] & UART_TRANSMIT);

		/* write the character */
		UART0[UART_DATA] = sequence[index];
	}
}


/*
 * getch reads the next available character from receive_buffer. If the buffer
 * is empty, this function returns 0.
 */
int
getch(void)
{
	int keycode = 0;
	char sequence[MAX_SPECIAL_KEY_SEQ_LEN] = {0};
	int sequence_length = 0;

	while (receive_buffer_head != receive_buffer_tail &&
		   is_special_key_sequence_prefix(sequence) &&
		   get_special_key_code(sequence) == 0)
	{
		sequence[sequence_length] = receive_buffer[receive_buffer_tail];
		receive_buffer_tail = circular_inc(receive_buffer_tail, RECEIVE_BUFFER_SIZE);
		sequence_length++;
	}

	keycode = get_special_key_code(sequence);
	if (keycode == 0 && sequence_length != 0)
	{
		keycode = sequence[sequence_length - 1];
	}

	return keycode;
}


/*
 * uart0_interrupt_handler reads a character from the uart0 serial port, and
 * puts it into receive_buffer. If the buffer is full, the character is ignored.
 */
static void
uart0_interrupt_handler(void)
{
	while (UART0[UART_INT_TARGET] & UART_RECEIVE)
	{
		int receive_buffer_new_head = 0;
		int data = UART0[UART_DATA];

		receive_buffer_new_head = circular_inc(receive_buffer_head, RECEIVE_BUFFER_SIZE);
		if (receive_buffer_new_head != receive_buffer_tail)
		{
			/* buffer not full */
			receive_buffer[receive_buffer_head] = (char) data;
			receive_buffer_head = receive_buffer_new_head;
		}
	}
}


/* circular_inc increments operand modula circle_size. */
static int
circular_inc(int operand, int circle_size)
{
	operand++;
	if (operand == circle_size)
	{
		operand = 0;
	}

	return operand;
}


/*
 * is_special_key_sequence_prefix returns true if the given sequence is prefix
 * of any special key's sequence. Otherwise, it returns false.
 */
static bool
is_special_key_sequence_prefix(char *sequence)
{
	bool is_prefix = false;
	size_t index = 0;
	int sequence_length = strlen(sequence);

	for (index = 0; index < SPECIAL_KEY_COUNT; index++)
	{
		SpecialKeySequence special_key = SPECIAL_KEY_SEQUENCES[index];

		if (strncmp(sequence, special_key.sequence, sequence_length) == 0)
		{
			is_prefix = true;
			break;
		}
	}

	return is_prefix;
}


/*
 * get_special_key_code returns the code of a special key with the given
 * sequence. If no such special key is found, it returns 0.
 */
static int
get_special_key_code(char *sequence)
{
	int keycode = 0;
	size_t index = 0;

	for (index = 0; index < SPECIAL_KEY_COUNT; index++)
	{
		if (strcmp(sequence, SPECIAL_KEY_SEQUENCES[index].sequence) == 0)
		{
			keycode = SPECIAL_KEY_SEQUENCES[index].code;
			break;
		}
	}

	return keycode;
}


/*
 * get_special_key_sequence returns the sequence of a special key with the given
 * code. If no such special key is found, it returns NULL.
 */
static char *
get_special_key_sequence(int code)
{
	char *sequence = 0;
	size_t index = 0;

	for (index = 0; index < SPECIAL_KEY_COUNT; index++)
	{
		if (SPECIAL_KEY_SEQUENCES[index].code == code)
		{
			sequence = (char *) SPECIAL_KEY_SEQUENCES[index].sequence;
			break;
		}
	}

	return sequence;
}
