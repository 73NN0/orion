/*
 * arena.h - Linear (bump) allocator with sub-lifetime support.
 *
 * Inspired by Ryan Fleury's "Untangling Lifetimes" essay [1] and Chris
 * Wellons' arena tips [2]. The core idea: instead of pairing each malloc
 * with a free, group allocations by their natural lifetime and free them
 * en masse by resetting a position counter.
 *
 * --------------------------------------------------------------------
 * Why arenas?
 * --------------------------------------------------------------------
 *
 * Manual memory management with malloc/free becomes painful because the
 * interface allows arbitrary lifetimes for each allocation. Real programs
 * rarely need that flexibility. When 1000 allocations all share the same
 * lifetime ("this level", "this frame", "this request"), pairing 1000
 * mallocs with 1000 frees is busywork that introduces bugs (leaks,
 * use-after-free, double-free).
 *
 * An arena is a single block of memory and a position counter. Allocation
 * is a pointer bump (O(1), no syscall, no fragmentation). Deallocation is
 * either a position restore (sub-lifetime) or a clear (whole arena). Both
 * are O(1).
 *
 * The trade-off: you cannot free an individual allocation in the middle
 * of an arena. If you need that, build a free-list or pool allocator on
 * top of the arena. See Fleury's "Composition With More Complex Allocators"
 * section.
 *
 * --------------------------------------------------------------------
 * Usage patterns
 * --------------------------------------------------------------------
 *
 * 1. PERMANENT ARENA
 *    Lives the entire program/session. Stores objects whose lifetime
 *    matches the program's. Created once at startup, destroyed at exit.
 *
 *      struct arena *pPermanent = arenaCreate(64 * 1024 * 1024);
 *      struct window *pWin    = windowCreate(pPermanent, ...);
 *      struct mesh   *pMesh   = loadMesh(pPermanent, "model.obj");
 *      / * ... run program ... * /
 *      arenaDestroy(&pPermanent);  / * frees window struct, mesh, etc. * /
 *
 * 2. FRAME ARENA
 *    Lives one frame (one tick of the main loop). Cleared at the start
 *    of every frame. Stores per-frame data: projected triangles, draw
 *    commands, transient UI elements.
 *
 *      struct arena *pFrame = arenaCreate(4 * 1024 * 1024);
 *      while (running) {
 *          arenaClear(pFrame);            / * O(1), all frame data freed * /
 *
 *          struct triangle *pTris = arenaPush(pFrame,
 *              sizeof(*pTris) * faceCount);
 *          / * project triangles, render them ... * /
 *      }
 *
 *    Replaces the dynamic-array-with-malloc-per-push pattern with a
 *    single allocation per frame.
 *
 * 3. SCRATCH ARENA + SUB-LIFETIME TRICK
 *    A scratch arena is a long-lived arena used for temporary work
 *    inside a function. The function records the position before its
 *    work and restores it on exit, freeing everything pushed in between.
 *
 *      void someWork(struct arena *pScratch) {
 *          uvlong saved = arenaGetPos(pScratch);
 *
 *          char *pBuf = arenaPush(pScratch, 4096);
 *          / * use pBuf for transient computation ... * /
 *
 *          arenaSetPos(pScratch, saved);  / * pBuf is gone * /
 *      }
 *
 *    Use case: a function that produces a persistent result AND needs
 *    transient buffers. The persistent result goes to one arena (passed
 *    in by the caller), the transient work goes to scratch.
 *
 *      struct mesh *
 *      loadMesh(struct arena *pPermanent, struct arena *pScratch,
 *               const char *path) {
 *          uvlong saved = arenaGetPos(pScratch);
 *
 *          char *pRaw = arenaPush(pScratch, fileSize);
 *          readFile(path, pRaw);
 *
 *          struct objToken *pTokens = tokenize(pScratch, pRaw);
 *
 *          struct mesh *pMesh = arenaPushZero(pPermanent, sizeof(*pMesh));
 *          buildMesh(pMesh, pTokens, pPermanent);
 *
 *          arenaSetPos(pScratch, saved);  / * raw + tokens disappear * /
 *          return pMesh;                  / * mesh persists in pPermanent * /
 *      }
 *
 * --------------------------------------------------------------------
 * Rules of thumb
 * --------------------------------------------------------------------
 *
 * - One arena per logical lifetime, not one per function or per object.
 * - The caller chooses the arena. Never let a function pick which arena
 *   to push into based on hidden state.
 * - For per-frame temporaries: frame arena, no save/restore needed.
 * - For function-local temporaries: scratch arena with save/restore.
 * - For session-long objects: permanent arena, never cleared.
 * - External resources (SDL handles, file descriptors, GL contexts) still
 *   need explicit cleanup. Arenas only manage memory.
 *
 * --------------------------------------------------------------------
 * References
 * --------------------------------------------------------------------
 *
 * [1] Ryan Fleury, "Untangling Lifetimes: The Arena Allocator", 2022.
 *     https://www.dgtlgrove.com/p/untangling-lifetimes-the-arena-allocator
 *
 * [2] Chris Wellons, "Arena allocator tips and tricks", 2023.
 *     https://nullprogram.com/blog/2023/09/27/
 *
 * [3] Ginger Bill, "Memory Allocation Strategies - Part 2: Linear/Arena",
 *     2019. https://www.gingerbill.org/article/2019/02/08/
 *           memory-allocation-strategies-002/
 */

struct arena {
    uchar *pBase;    /* start of the bloc */
    uvlong capacity; /* total size */
    uvlong pos;      /* current positon (bump pointer) */
};

/*
 * Create an arena with the given capacity. Returns nil on failure.
 * Allocates both the struct arena and its backing block via malloc.
 */
struct arena *arenaCreate(/*capacity */ uvlong);

/*
 * Destroy an arena. Frees the backing block and the struct. Sets
 * *ppArena to nil to make use-after-free more visible.
 */
void
arenaDestroy(/* ppArena */ struct arena **);

/*
 * Push 'size' bytes onto the arena. Returns a pointer to uninitialized
 * memory aligned to ARENA_ALIGN. Returns nil if the arena is full.
 */
void *
arenaPush(/*pArena*/ struct arena *, /*size */ uvlong);

/*
 * Like arenaPush but zero-initializes the returned memory.
 * Use for structs where you want defined default values.
 */
void *
arenaPushZero(/*pArena*/ struct arena *, /*size */ uvlong);

/*
 * Read the current position. Save this before a sub-lifetime and
 * restore it via arenaSetPos to free everything pushed in between.
 */
uvlong
arenaGetPos(/* pArena */ struct arena *);

/*
 * Restore the position to a previous value. All allocations made
 * after that point become invalid. The memory is not zeroed; it
 * will be reused by the next arenaPush.
 *
 * Pos values greater than the current position are silently ignored
 * (you can only rewind, not forward).
 */
void
arenaSetPos(/* pArena */ struct arena *, /*pos*/ uvlong);

/*
 * Reset the arena to empty. Equivalent to arenaSetPos(pArena, 0).
 * Use for frame arenas at the start of each frame.
 */
void
arenaClear(/*pArena */ struct arena *);
