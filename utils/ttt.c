main()
{
	int a;

	a= 0;
	a |= 1;
	a |= 2;
	a |= 4;
	a |= 8;
	printf("%d %d\n", (a == 1 + 2 + 4 + 8), a);
}
