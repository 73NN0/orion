#include <u.h>
#include <arena.h>  /* arena, arenaClear, arenaCreate, arenaDestroy, arenaGetPos */
#include <stdlib.h> /* calloc malloc uvlong free */
#include <string.h> /* memset */

#define ARENA_ALIGN 16

static uvlong
alignUp(uvlong value, uvlong align) {
    return (value + (align - 1)) & ~(align - 1);
}

struct arena *
arenaCreate(uvlong capacity) {
    struct arena *pArena;

    pArena = calloc(1, sizeof(*pArena));
    if (pArena == nil) {
        goto fail_arena;
    }
    pArena->pBase = malloc(capacity);
    if (pArena->pBase == nil) {
        goto fail_base;
    }

    pArena->capacity = capacity;
    pArena->pos = 0;

    return pArena;

fail_base:
    if (pArena != nil) {
        free(pArena);
    }
fail_arena:
    return nil;
}

void
arenaDestroy(struct arena **ppArena) {
    if (ppArena == nil || *ppArena == nil) {
        return;
    }

    if ((*ppArena)->pBase != nil) {
        free((*ppArena)->pBase);
    }

    free(*ppArena);
    *ppArena = nil;
}

void *
arenaPush(struct arena *pArena, uvlong size) {
    uvlong aligned = 0;
    void *result;

    if (pArena == nil) {
        return nil;
    }
    aligned = alignUp(pArena->pos, ARENA_ALIGN);
    if (aligned + size > pArena->capacity) {
        return nil; /* OOM — strategie a definir */
    }
    result = pArena->pBase + aligned;
    pArena->pos = aligned + size;
    return result;
}

void *
arenaPushZero(struct arena *pArena, uvlong size) {
    void *p = arenaPush(pArena, size);
    if (p != nil) {
        memset(p, 0, size);
    }
    return p;
}

uvlong
arenaGetPos(struct arena *pArena) {
    return pArena ? pArena->pos : 0;
}

void
arenaSetPos(struct arena *pArena, uvlong pos) {
    if (pArena != nil && pos <= pArena->pos) {
        pArena->pos = pos;
    }
}

void
arenaClear(struct arena *pArena) {
    if (pArena != nil) {
        pArena->pos = 0;
    }
}