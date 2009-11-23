#include <stdio.h>
#include <stdlib.h>

#include "main.h"

#include "fooid.h"
#include "sndfile.h"

int main(int argc, char ** argv)
{
    if (argc < 2)
    {
        printf("Usage: ./test filename.wav\n");
        return 0;
    }

    SF_INFO * sfinfo = malloc(sizeof(*sfinfo));
    SNDFILE * file = sf_open(argv[1], SFM_READ, sfinfo);

    print_file_info(sfinfo);

    t_fooid * fooid = fp_init(sfinfo->samplerate, sfinfo->channels);

    short * data = malloc(sizeof(short) * sfinfo->channels * sfinfo->samplerate);

    int centiseconds = 0;

    while (1)
    {
        sf_count_t items_read = sf_read_short(file, data, sfinfo->channels * sfinfo->samplerate);

        if (items_read == 0)
        {
            break;
        }

        centiseconds += 100 * sfinfo->channels * sfinfo->samplerate / items_read;

        int result = fp_feed_short(fooid, data, items_read);

        if (result < 0)
        {
            printf("Error!\n");
            break;
        }
    }

    unsigned char * buffer =  malloc(fp_getsize(fooid));

    int result = fp_calculate(fooid, centiseconds, buffer);

    if (result < 0)
    {
        printf("Failed to calculate fingerprint\n");
    }
    else
    {
        int i;
        for (i = 0; i < fp_getsize(fooid); i++)
        {
            if (!(i % 32))
            {
                printf("\n");
            }

            printf("%02x ", (int) buffer[i]);
        }

        printf("\n");
    }

    free(buffer);
    fp_free(fooid);
    free(data);
    free(sfinfo);

    return 0;
}
