
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct _linkedlist {
	int address;
	unsigned char byte;
	struct _linkedlist *next;
} linkedlist;

int usage(char *name)
{
	printf("%s file address=byte[,byte,...] [...]\n", name);
	return 1;
}

int getplainint(char *str, int start, int end)
{
	int i;
	int accum = 0;

	for (i = start; i < end; i++)
	{
		if (str[i] < '0' || str[i] > '9')
			return -1;
		accum = accum*10 + (str[i] - '0');
	}

	return accum;
}

int validHex(char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	return -1;
}

int gethexint(char *str, int start, int end)
{
	int i;
	int accum = 0, value;

	for (i = start + 2; i < end; i++)
	{
		if ((value = validHex(str[i])) < 0)
			return -1;
		accum = accum*16 + value;
	}

	return accum;
}

int getint(char *str, int start, int end)
{
	int length;

	length = strlen(str);

	if (start > end || start > length || end > length || start == end)
		return -1;

	if (end - start < 3 || str[start] != '0' || (str[start+1] != 'x' && str[start+1] != 'X'))
		return getplainint(str, start, end);

	return gethexint(str, start, end);
}

int addlink(linkedlist **list, int address, int byte)
{
	linkedlist *link = malloc(sizeof(linkedlist));

	if (link == NULL)
		return 1;

	link->address = address;
	link->byte = (unsigned char) byte;
	link->next = *list;
	*list = link;

	return 0;
}

int process(linkedlist **ls, int size, char *patch)
{
	int i, count, seperate;
	int address, byte;
	int start, stop;
	int length;

	length = strlen(patch);

	if (length < 3)
		return 1;

	seperate = -1;

	for (i = 0; i < length; i++)
	{
		if (patch[i] == '=') {
			if (i == 0 || patch[i+1] == '\0')
				return 1;
			seperate = i;
			break;
		}
	}

	if (seperate == -1)
		return 1;	

	if ((address = getint(patch, 0, seperate)) < 0 || address >= size)
		return 1;

	start = seperate + 1;
	while (start < length) {
		if (address >= size)
			return 1;
		for (stop = start + 1; stop < length; stop++)
		{
			if (patch[stop] == '=')
				return 1;
			if (patch[stop] == ',')
				break;
		}
		if ((byte = getint(patch, start, stop)) < 0 || byte > 255)
			return 1;
		if (addlink(ls, address, byte))
			return 1;
		address++;
		start = stop + 1;
	}

	return 0;
}

int apply(FILE *f, linkedlist *ls)
{
	for (; ls != NULL; ls = ls->next)
	{
		//printf("Patching %x to %d\n", ls->address, ls->byte);
		if (fseek(f, ls->address, SEEK_SET))
			return 1;

		if (!fwrite(&(ls->byte), 1, 1, f))
			return 1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	FILE *f;
	int i, size;
	linkedlist *changes = NULL;

	if (argc < 2)
		return usage(argv[0]);

	f = fopen(argv[1], "rb+");

	if (f == NULL) {
		printf("Error opening %s; patching failed\n", argv[1]);
		return 2;
	}

	if (fseek(f, 0, SEEK_END)) {
		fclose(f);
		printf("Failed seeking %s; patching failed\n", argv[1]);
		return 5;
	}

	size = ftell(f);

	if (size < 0) {
		fclose(f);
		printf("Failed reading size of %s; patching failed\n", argv[1]);
	}

	for (i = 2; i < argc; i++)
	{
		if (process(&changes, size, argv[i])) {
			fclose(f);
			printf("Error processing %s; patching failed\n", argv[i]);
			return 3;
		}
	}

	if (apply(f, changes)) {
		fclose(f);
		printf("Error applying patches; some patches may have been applied anyways\n");
		return 4;
	}

	fclose(f);
	printf("Patching successful\n");
	return 0;
}
