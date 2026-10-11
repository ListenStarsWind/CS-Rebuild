
#include <stdarg.h>
#include <stdio.h>

__attribute__((noinline))
int probe(int count, ...)
{
    va_list ap;
    va_start(ap, count);

    int a = va_arg(ap, int);
    int b = va_arg(ap, int);

    va_end(ap);

    return a + b;
}

int main(void)
{
    int result = probe(2, 10, 20);
    printf("result = %d\n", result);
    return 0;
}
