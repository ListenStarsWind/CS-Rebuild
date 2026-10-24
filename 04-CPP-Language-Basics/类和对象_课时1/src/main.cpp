#include"Date.h"

int main()
{
	wind::Date d1,d2;
	d1.Init(1, 1, 1);
	d2.Init(2, 2, 2);
	d1.Print();
	d2.Print();
	return 0;
}

