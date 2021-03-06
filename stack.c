#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "stack.h"

struct aem_stack *aem_stack_alloc_raw(void)
{
	struct aem_stack *stk = malloc(sizeof(*stk));

	if (stk == NULL) return NULL;

	*stk = AEM_STACK_EMPTY;

	return stk;
}

struct aem_stack *aem_stack_init_prealloc(struct aem_stack *stk, size_t maxn)
{
	if (stk == NULL) return NULL;

	stk->n = 0;
	stk->maxn = maxn;
	stk->s = malloc(stk->maxn * sizeof(void*));

	return stk;
}

struct aem_stack *aem_stack_init_v(struct aem_stack *stk, size_t n, ...)
{
	va_list ap;
	va_start(ap, n);

	aem_stack_init_prealloc(stk, n);

	for (size_t i = 0; i < n; i++)
	{
		aem_stack_push(stk, va_arg(ap, void*));
	}

	va_end(ap);

	return stk;
}

void aem_stack_free(struct aem_stack *stk)
{
	if (stk == NULL) return;

	aem_stack_dtor(stk);

	free(stk);
}

void aem_stack_dtor(struct aem_stack *stk)
{
	if (stk == NULL) return;

	aem_stack_reset(stk);

	stk->n = stk->maxn = 0;

	if (stk->s == NULL) return;

	free(stk->s);

	stk->s = NULL;
}


void **aem_stack_release(struct aem_stack *stk, size_t *n)
{
	if (stk == NULL)
	{
		*n = 0;
		return NULL;
	}

	aem_stack_shrink(stk);

	void **s = stk->s;
	*n = stk->n;

	free(stk);

	return s;
}


static inline void aem_stack_grow(struct aem_stack *stk, size_t maxn_new)
{
#if AEM_STACK_DEBUG
	aem_logf_ctx(AEM_LOG_DEBUG, "realloc: %zd, %zd -> %zd\n", stk->n, stk->maxn, maxn_new);
#endif
	stk->maxn = maxn_new;
	stk->s = realloc(stk->s, stk->maxn * sizeof(void*));
}

void *aem_stack_shrink(struct aem_stack *stk)
{
	if (stk == NULL) return NULL;

	aem_stack_grow(stk, stk->n);

	return stk->s;
}

void aem_stack_reserve(struct aem_stack *stk, size_t maxn)
{
	if (stk == NULL) return;

	if (stk->maxn < maxn)
	{
		aem_stack_grow(stk, maxn);
	}
}


void aem_stack_push(struct aem_stack *stk, void *s)
{
	if (stk == NULL) return;

#if AEM_STACK_DEBUG
	aem_logf_ctx(AEM_LOG_DEBUG, "push %p\n", s);
#endif

	if (stk->maxn < stk->n + 1)
	{
		aem_stack_grow(stk, (stk->n + 1)*2);
	}

	stk->s[stk->n++] = s;
}

void aem_stack_pushn(struct aem_stack *restrict stk, size_t n, void **restrict elements)
{
	if (stk == NULL) return;

	if (elements == NULL) return;

	for (size_t i = 0; i < n; i++)
	{
		aem_stack_push(stk, elements[i]);
	}
}

void aem_stack_append(struct aem_stack *restrict stk, struct aem_stack *restrict stk2)
{
	if (stk == NULL) return;

	if (stk2 == NULL) return;

	size_t n = stk->n + stk2->n;
	// make room for stk2
	if (stk->maxn < n)
	{
		aem_stack_grow(stk, n*2);
	}

	memcpy(&stk->s[stk->n], stk2->s, stk2->n*sizeof(void*));

	stk->n = n;
}

size_t aem_stack_transfer(struct aem_stack *restrict dest, struct aem_stack *restrict src, size_t n)
{
	if (dest == NULL) return 0;
	if (src  == NULL) return 0;

	if (src->n < n) return 0;

	size_t new_top = src->n - n;

	for (size_t i = 0; i < n; i++)
	{
		aem_stack_push(dest, src->s[new_top + i]);
	}

	aem_stack_trunc(src, new_top);

	return n;
}

void *aem_stack_pop(struct aem_stack *stk)
{
	if (stk == NULL) return NULL;

	if (stk->n <= 0)
	{
#if AEM_STACK_DEBUG
		aem_logf_ctx(AEM_LOG_DEBUG, "underflow\n");
#endif

		return NULL;
	}

	void *p = stk->s[--stk->n];

#if AEM_STACK_DEBUG
	aem_logf_ctx(AEM_LOG_DEBUG, "%p\n", p);
#endif

	return p;
}

void *aem_stack_peek(struct aem_stack *stk)
{
	if (stk == NULL) return NULL;

	if (stk->n <= 0)
	{
#if AEM_STACK_DEBUG
		aem_logf_ctx(AEM_LOG_DEBUG, "underflow\n");
#endif

		return NULL;
	}

#if AEM_STACK_DEBUG
	aem_logf_ctx(AEM_LOG_DEBUG, "%p\n", stk->s[stk->n-1]);
#endif

	return stk->s[stk->n-1];
}

void *aem_stack_index_end(struct aem_stack *stk, size_t i)
{
	if (stk == NULL) return NULL;

	size_t i2 = stk->n - 1 - i;

	if (i >= stk->n)
	{
#if AEM_STACK_DEBUG
		aem_logf_ctx(AEM_LOG_DEBUG, "[-%zd = %zd] underflow\n", i, i2);
#endif

		return NULL;
	}

#if AEM_STACK_DEBUG
	aem_logf_ctx(AEM_LOG_DEBUG, "[-%zd = %zd]\n", i, i2, stk->s[i2]);
#endif

	return stk->s[i2];
}

void *aem_stack_index(struct aem_stack *stk, size_t i)
{
	if (stk == NULL) return NULL;

	if (i >= stk->n)
	{
		return NULL;
	}
#if AEM_STACK_DEBUG
	aem_logf_ctx(AEM_LOG_DEBUG, "[%zd] = %p\n", i, stk->s[i]);
#endif

	return stk->s[i];
}

void aem_stack_assign(struct aem_stack *stk, size_t i, void *s)
{
	if (stk == NULL) return;

	if (i + 1 > stk->n)
	{
		stk->n = i + 1;

		if (stk->n > stk->maxn)
		{
			aem_stack_grow(stk, stk->n);
		}
	}

	stk->s[i] = s;
}


void aem_stack_qsort(struct aem_stack *stk, int (*compar)(const void *p1, const void *p2))
{
	qsort(stk->s, stk->n, sizeof(stk->s[0]), compar);
}
