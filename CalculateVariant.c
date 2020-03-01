#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
int k= atoi(argv[1]);
int x= atoi(argv[2]);
srand(x);
int N=(rand() % k)+1;
printf("%d\n", N);
return 0;
}
