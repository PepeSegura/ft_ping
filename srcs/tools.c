#include "ft_ping.h"

uint16_t checksum(void *buffer, size_t len)
{
	uint8_t		*bytes = (uint8_t *)buffer;
	uint32_t	sum = 0;

	for (size_t i = 0; i < len; i += 2)
	{
        uint16_t two_bytes;
        if (i + 1 < len)
            two_bytes = (bytes[i] << 8) | bytes[i + 1];
        else
            two_bytes = (bytes[i] << 8);
        sum += two_bytes;
    }

	while (sum >> 16 != 0)
	{
		sum = (sum & 0xFFFF) + (sum >> 16);
	}
	sum = (uint16_t)(~sum);
	return (sum);
}

double time_diff(struct timeval *prev)
{
	struct timeval now;

	gettimeofday(&now, NULL);
	return (((now.tv_sec - prev->tv_sec) * 1000.0) + (now.tv_usec - prev->tv_usec) / 1000.0);
}
