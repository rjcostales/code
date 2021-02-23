#include <stdio.h>

int main(int argc, char *argv[])
{
	int n1 = 1;
	int n2 = 2;
	int dummy = scanf("%d,%d", &n1, &n2);
	printf("n=%d, o=%d, scanf=%d.\n", n1, n2, dummy);
	scanf("%d,%d", &n1, &n2);
	printf("n=%d, o=%d, scanf=%d.\n", n1, n2, dummy);

}
