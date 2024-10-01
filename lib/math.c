#include "lib/math.h"

/*
 * unsigned_divmod divides numerator and denmoriator, then returns the quotient as
 * result. If remainder_pointer is not NULL, then the function returns the division
 * remainder in remainder_pointer.
 *
 * Algorithm: http://en.wikipedia.org/wiki/Division_algorithm
 * Thanks Wikipedia :D
 */
uint32_t
unsigned_divmod(uint32_t numerator, uint32_t denominator, uint32_t* remainder_pointer)
{
	int bit_index = 0;
	uint32_t quotient = 0;
	uint32_t remainder = 0;

	for (bit_index = 31; bit_index >= 0; bit_index--)
	{
		uint32_t numerator_bit = ((numerator & (1u << bit_index)) ? 1 : 0);
		remainder = (remainder << 1) | numerator_bit;
		if (remainder >= denominator)
		{
			remainder -= denominator;
			quotient |= (1u << bit_index);
		}
	}

	if (remainder_pointer != NULL)
	{
		(*remainder_pointer) = remainder;
	}

	return quotient;
}
