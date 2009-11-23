#include "sndfile.h"

void print_file_info(SF_INFO * file_info)
{
    printf("--- File Info ---\n");
    printf("Sample rate:\t%d\n", file_info->samplerate);
    printf("Channels: \t%d\n", file_info->channels);
    printf("\n");
}
