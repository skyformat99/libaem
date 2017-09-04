#include <stdlib.h>
#include <ctype.h>

#include "stringslice.h"

int stringslice_file_write(const struct stringslice *slice, FILE *fp)
{
	if (slice == NULL) return -1;
	if (fp == NULL) return -1;

	const char *p = slice->start;
	const char *pe = slice->end;

	while (p < pe)
	{
		size_t n_written = fwrite(p, 1, pe - p, fp);

		p += n_written;

		if (n_written == 0)
		{
			if (ferror(fp))
			{
				return -1;
			}
		}
	}

	return 0;
}

void stringslice_match_ws(struct stringslice *slice)
{
	if (slice == NULL) return;

	while (stringslice_ok(slice) && isspace(*slice->start)) slice->start++;
}

struct stringslice stringslice_match_word(struct stringslice *slice)
{
	if (slice == NULL) return STRINGSLICE_EMPTY;

	struct stringslice line;
	line.start = slice->start;

	while (stringslice_ok(slice) && !isspace(*slice->start)) slice->start++;

	line.end = slice->start;

	return line;
}

struct stringslice stringslice_match_line(struct stringslice *slice)
{
	if (slice == NULL) return STRINGSLICE_EMPTY;

	struct stringslice line;
	line.start = slice->start;

	while (stringslice_ok(slice) && !(*slice->start == '\n' || *slice->start == '\n')) slice->start++;

	line.end = slice->start;

	return line;
}

int stringslice_match(struct stringslice *slice, const char *s)
{
	if (slice == NULL) return 0;
	if (s == NULL) return 1;

	const char *p2 = slice->start;

	while (*s)
	{
		if (p2 == slice->end)
		{
			return 0;
		}

		if (*p2++ != *s++)
		{
			return 0;
		}
	}

	slice->start = p2;

	return 1;
}

int stringslice_eq(struct stringslice slice, const char *s)
{
	if (s == NULL) return 1;

	while (slice.start != slice.end && *s != '\0')
	{
		if (*slice.start++ != *s++) return 0;
	}

	return *s == '\0';
}

static inline int hex2nib(char c)
{
	if (c >= '0' && c <= '9')
	{
		return c - '0';
	}
	else if (c >= 'a' && c <= 'f')
	{
		return c - 'a' + 0xA;
	}
	else if (c >= 'A' && c <= 'F')
	{
		return c - 'A' + 0xA;
	}
	else
	{
		return -1;
	}
}

int stringslice_match_hexbyte(struct stringslice *slice)
{
	if (slice == NULL) return -1;

	if (slice->start == slice->end || slice->start + 1 == slice->end) return -1;

	const char *p = slice->start;

	int c1 = hex2nib(*p++);
	if (c1 < 0) return -1;

	int c0 = hex2nib(*p++);
	if (c0 < 0) return -1;

	slice->start = p;

	return c1 << 4 | c0;
}
