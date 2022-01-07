#include "hexdump.h"

void hexDump(void *pointer, int byte_count)
{
  printf("\n");
  uint8_t *dp = pointer;
  for (int i = 0; i < byte_count; ++i)
  {
    printf("%02X ", *dp);
    if ((i + 1) % 2 == 0)
    {
      printf("   ");
    }

    if ((i + 1) % 8 == 0)
    {
      printf("\n");
    }
    dp += 1;
  }
  printf("\n\n");
}