#include "stringslice.h"
#include "stringbuf.h"

int aem_stringbuf_put_utf8(struct aem_stringbuf *str, unsigned int c)
{
	if (c < 0x80)
	{
		aem_stringbuf_putc(str, c);
	}
	else if (c < 0x800)
	{
		aem_stringbuf_putc(str, 0xc0 | ((c >>  6) & 0x1f));
		aem_stringbuf_putc(str, 0x80 | ((c >>  0) & 0x3f));
	}
	else if (c < 0x10000)
	{
		aem_stringbuf_putc(str, 0xe0 | ((c >> 12) & 0x0f));
		aem_stringbuf_putc(str, 0x80 | ((c >>  6) & 0x3f));
		aem_stringbuf_putc(str, 0x80 | ((c >>  0) & 0x3f));
	}
	else if (c < 0x200000)
	{
		aem_stringbuf_putc(str, 0xf0 | ((c >> 18) & 0x07));
		aem_stringbuf_putc(str, 0x80 | ((c >> 12) & 0x3f));
		aem_stringbuf_putc(str, 0x80 | ((c >>  6) & 0x3f));
		aem_stringbuf_putc(str, 0x80 | ((c >>  0) & 0x3f));
	}
	else if (c < 0x4000000)
	{
		aem_stringbuf_putc(str, 0xf8 | ((c >> 24) & 0x03));
		aem_stringbuf_putc(str, 0x80 | ((c >> 18) & 0x3f));
		aem_stringbuf_putc(str, 0x80 | ((c >> 12) & 0x3f));
		aem_stringbuf_putc(str, 0x80 | ((c >>  6) & 0x3f));
		aem_stringbuf_putc(str, 0x80 | ((c >>  0) & 0x3f));
	}
	else if (c < 0x80000000)
	{
		aem_stringbuf_putc(str, 0xfc | ((c >> 30) & 0x01));
		aem_stringbuf_putc(str, 0x80 | ((c >> 24) & 0x3f));
		aem_stringbuf_putc(str, 0x80 | ((c >> 18) & 0x3f));
		aem_stringbuf_putc(str, 0x80 | ((c >> 12) & 0x3f));
		aem_stringbuf_putc(str, 0x80 | ((c >>  6) & 0x3f));
		aem_stringbuf_putc(str, 0x80 | ((c >>  0) & 0x3f));
	}
	else
	{
		return -1;
	}

	return 0;
}

int aem_stringslice_get_utf8(struct aem_stringslice *slice)
{
	const char *start = slice->start; // make backup of start

	int c = aem_stringslice_getc(slice);
	if (c < 0) return c; // end of input

	size_t n = 0;

	if (c >= 0 && c < 0x80)
	{
		n = 0;
		c &= 0x7f;
	}
	else if (c >= 0x80 && c < 0xc0)
	{
		return -1;
	}
	else if (c >= 0xc0 && c < 0xe0)
	{
		n = 1;
		c &= 0x1f;
	}
	else if (c >= 0xe0 && c < 0xf0)
	{
		n = 2;
		c &= 0x0f;
	}
	else if (c >= 0xf0 && c < 0xf8)
	{
		n = 3;
		c &= 0x07;
	}
	else if (c >= 0xf8 && c < 0xfc)
	{
		n = 4;
		c &= 0x03;
	}
	else if (c >= 0xfc && c < 0xfe)
	{
		n = 5;
		c &= 0x01;
	}
	else
	{
		return -1;
	}

	for (size_t i = 0; i < n; i++)
	{
		if (!aem_stringslice_ok(slice)) return -1;

		int c2 = aem_stringslice_getc(slice);
		if (c2 < 0 || (c2 & 0xc0) != 0x80)
		{
			slice->start = start;
			return -1;
		}

		c <<= 6;
		c |= c2 & 0x3f;
	}

	return c;
}
