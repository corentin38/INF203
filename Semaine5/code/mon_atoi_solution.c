#include <stdio.h>

int mon_atoi(char *s)
{
	int i = 0;
	int acc = 0;

	while (s[i] != '\0') {
		printf("acc[%d] = %d\n", i, acc);
		if ((s[i] < '0') || ( '9' < s[i])) {
			return 0;
		}

		acc = (acc * 10) + (s[i] - '0');
		i++;
	}
	printf("acc[final] = %d\n", acc);
	return acc;
}

int main(int argc, char **argv)
{
	int i;
	for ( i = 1; i < argc; i++) {
		printf("argv[%d] = %d\n", i, mon_atoi(argv[i]));
	}

	return 0;
}