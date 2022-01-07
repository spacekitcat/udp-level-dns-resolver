#include "hexdump.h"

void hexDump(void *pointer, int byte_count)
{
	printf("\n");
	uint8_t *dp = pointer;
	char buffer[17];
	char *buffer_it = buffer;
	for (int i = 0; i < byte_count; ++i)
	{
		printf("%02X ", *dp);
		if (((int8_t)*dp) > 40 && ((int8_t)*dp) < 177)
		{
			*buffer_it = (char)*dp;
		}
		else
		{
			*buffer_it = '.';
		}

		buffer_it++;

		if ((i + 1) % 8 == 0)
		{
			printf("  ");
		}

		if ((i + 1) % 16 == 0)
		{
			*buffer_it = '\0';
			buffer_it = buffer;
			printf("%s\n", buffer);
		}
		dp += 1;
	}
	printf("\n\n");
}