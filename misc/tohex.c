/* tohex.c, UNIX version */

#include <stdio.h>

char hex[] = "@ABCDEFGHIJKLMNO";
int bytes, sum;
unsigned long htonl();

int main(int argc, char *argv[])
{
	int i, len;
	char *cp;

	len = 0;
	while ((i = getchar()) != EOF) {
		bytes++;
		sum += i;
		putchar(hex[i >> 4]);
		putchar(hex[i & 0xF]);
		if (++len > 32) {
			putchar('\n');
			len = 0;
		}
	}
	fprintf(stderr, "bytes %d, sum %d\n", bytes, sum);
	putchar('|');
	sum += bytes;
	sum = htonl(sum);
	cp = (char *) &sum;
	for (len = 0; len < 4; len++) {
		i = (*cp++ & 0xff);
		putchar(hex[i >> 4]);
		putchar(hex[i & 0xF]);
	}
	putchar('\n');
	return 0;
}

#define nohtonl
#ifdef nohtonl                  /* if not in library */

/* "Host" to "net" byte order swappers. */
unsigned short htons(unsigned short a)
{
	unsigned short result;
	char *sp = (char *) &a;
	char *dp = (char *) &result;

	dp[1] = *sp++;
	dp[0] = *sp;
	return (result);
}

unsigned long htonl(unsigned long a)
{
	unsigned long result;
	char *sp = (char *) &a;
	char *dp = (char *) &result;

	dp[3] = *sp++;
	dp[2] = *sp++;
	dp[1] = *sp++;
	dp[0] = *sp;
	return (result);
}

#endif
