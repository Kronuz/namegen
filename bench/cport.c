/* The comparator: the C port of the fantasy name generator (skeeto/fantasyname, c/namegen.h,
 * public domain), compiled as C and called from bench/bench.cc. */
#include NAMEGEN_C_HEADER

int cport_namegen(char *dst, unsigned long len, const char *pattern, unsigned long *seed)
{
    return namegen(dst, len, pattern, seed);
}
