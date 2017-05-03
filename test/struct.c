#include <stdio.h>

struct defs {
#define AAA 0x0023435
#define BBB 0x2
#define CCC 0x3
int a;
int b;
} __attribute__((packed));

int main()
{
	struct defs *df = (struct defs *)(AAA);
	printf("%p\n", &(df->a));
	printf("%p\n", &(df->b));
	return 0;
}
