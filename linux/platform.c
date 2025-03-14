#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void nes_get_rom (const char *filename, uint8_t **data, uint64_t *filesize)
{
	FILE *fp = fopen (filename, "r");
	uint64_t sz_file = 0L;
	fseek (fp, 0, SEEK_END);
	sz_file = ftell (fp);
	fseek (fp, 0, SEEK_SET);
	(*data) = malloc (sz_file);
	fread (*data, 1, sz_file, fp);
	fclose (fp);
	*filesize = sz_file;
}
