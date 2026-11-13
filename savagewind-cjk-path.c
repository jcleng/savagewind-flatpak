/* savagewind-cjk-path.c — LD_PRELOAD shim: give Savage Wind's hard-coded CJK font
 * paths a file to open inside the Flatpak sandbox.
 *
 * Why this exists
 * ---------------
 * Two different font pipelines look for CJK fonts, and neither can be redirected
 * by an environment variable alone:
 *
 *  1. The Dear ImGui start screen / overlay menu (runtime/ui/i18n.mm,
 *     ui_merge_fallback_font) merges CJK glyphs into its Latin atlas by fopen()ing a
 *     fixed list of paths, per language ("zh_hans_fonts" and friends):
 *
 *         C:/Windows/Fonts/msyh.ttc
 *         C:/Windows/Fonts/simsun.ttc
 *         /usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc
 *         /usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc
 *         /System/Library/Fonts/PingFang.ttc
 *
 *     SAVAGEWIND_MENU_FONT / SAVAGEWIND_LAUNCHER_FONT only replace the *base* face and
 *     are loaded with the glyph range [0x20,0xFF] (Latin-1): they never bring CJK
 *     codepoints, so the merge step above is the only thing that can.
 *
 *  2. The game's own text (runtime/hle/pl.c, nn::pl shared system fonts) is told a
 *     single font file through SAVAGEWIND_FONT; the wrapper sets that to a TrueType
 *     CJK font (the game font engine cannot use CFF). That one works on its own.
 *
 * In a Flatpak sandbox /usr is the read-only runtime (no CJK fonts), /app is where we
 * can install, and host fonts land in /run/host. None of the paths above exist, so
 * every CJK glyph draws as '?'. This shim rewrites exactly those literal paths — and
 * nothing else — to the fonts this package installs under /app/share/fonts. Game data,
 * saves, logs, shaders and mods are untouched (every other path passes through).
 *
 * Only compiled/used for the two ImGui entry points; the game text is handled by
 * SAVAGEWIND_FONT. If the target file is missing the original path is left as-is, so a
 * real host install (running outside the sandbox) keeps working unchanged.
 *
 * Build: cc -shared -fPIC -O2 -o libsavagewind-cjk-path.so savagewind-cjk-path.c -ldl
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <unistd.h>

/* Where this package installs the fonts (see the manifest). */
#define CJK_NOTO "/app/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc"
#define CJK_WQY  "/app/share/fonts/wqy/wqy-microhei.ttc"

/* The literal CJK lookups upstream tries, in order, for a simplified-Chinese UI. The
 * traditional, Japanese and Korean lists reuse the same Noto CFF collection paths
 * (/usr/share/fonts/opentype/noto/..., /usr/share/fonts/noto-cjk/...), so redirecting
 * those covers every CJK language. */
static const char *const k_paths[] = {
    "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
    "/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc",
    /* Some builds also list these; harmless to cover them. */
    "/usr/share/fonts/google-noto-cjk/NotoSansCJK-Regular.ttc",
    "/usr/share/fonts/noto/NotoSansCJK-Regular.ttc",
    "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
};

static int file_exists(const char *p) {
    if (!p) return 0;
    struct stat st;
    return stat(p, &st) == 0 && S_ISREG(st.st_mode);
}

/* Return the replacement for `path`, or `path` itself if it is not one we handle.
 * The Noto CFF collection serves the ImGui atlas (stb_truetype reads CFF fine). */
static const char *remap(const char *path) {
    if (!path) return path;
    for (size_t i = 0; i < sizeof k_paths / sizeof *k_paths; i++) {
        if (strcmp(path, k_paths[i]) == 0) {
            if (file_exists(CJK_NOTO)) return CJK_NOTO;
            if (file_exists(CJK_WQY)) return CJK_WQY; /* fallback: same glyphs, TrueType */
            return path; /* nothing installed: leave as-is */
        }
    }
    return path;
}

/* --- intercepted entry points ------------------------------------------------ */

typedef FILE *(*fopen_fn)(const char *, const char *);
typedef int (*open_fn)(const char *, int, ...);
typedef int (*open64_fn)(const char *, int, ...);
typedef int (*stat_fn)(const char *, struct stat *);
typedef int (*stat64_fn)(const char *, struct stat64 *);

static FILE *real_fopen(const char *p, const char *m) {
    static fopen_fn fn;
    if (!fn) fn = (fopen_fn)dlsym(RTLD_NEXT, "fopen");
    return fn(p, m);
}
static int real_open(const char *p, int flags, mode_t mode) {
    static open_fn fn;
    if (!fn) fn = (open_fn)dlsym(RTLD_NEXT, "open");
    return fn(p, flags, mode);
}
static int real_open64(const char *p, int flags, mode_t mode) {
    static open64_fn fn;
    if (!fn) fn = (open64_fn)dlsym(RTLD_NEXT, "open64");
    return fn ? fn(p, flags, mode) : real_open(p, flags, mode);
}
static int real_stat(const char *p, struct stat *st) {
    static stat_fn fn;
    if (!fn) fn = (stat_fn)dlsym(RTLD_NEXT, "stat");
    return fn(p, st);
}

FILE *fopen(const char *path, const char *mode) { return real_fopen(remap(path), mode); }

FILE *fopen64(const char *path, const char *mode) {
    static fopen_fn fn;
    if (!fn) fn = (fopen_fn)dlsym(RTLD_NEXT, "fopen64");
    return fn ? fn(remap(path), mode) : real_fopen(remap(path), mode);
}

int open(const char *path, int flags, ...) {
    mode_t mode = 0;
    if (flags & 0100 /* O_CREAT */) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
    return real_open(remap(path), flags, mode);
}

int open64(const char *path, int flags, ...) {
    mode_t mode = 0;
    if (flags & 0100 /* O_CREAT */) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
    return real_open64(remap(path), flags, mode);
}

/* ui_merge_fallback_font fopen()s the path to test it, then AddFontFromFileTTF opens
 * it again: both go through the calls above. We also remap stat so callers that check
 * existence first see the installed font. Distinct symbol names on glibc: __xstat is
 * gone in newer glibc, stat/stat64 cover it. */
int stat(const char *path, struct stat *st) { return real_stat(remap(path), st); }

int stat64(const char *path, struct stat64 *st) {
    static stat64_fn fn;
    if (!fn) fn = (stat64_fn)dlsym(RTLD_NEXT, "stat64");
    if (fn) return fn(remap(path), st);
    /* No stat64 in this libc: fall back to stat. */
    struct stat s1;
    int r = real_stat(remap(path), &s1);
    if (r == 0) {
        memset(st, 0, sizeof *st);
        st->st_mode = s1.st_mode;
        st->st_size = s1.st_size;
    }
    return r;
}
