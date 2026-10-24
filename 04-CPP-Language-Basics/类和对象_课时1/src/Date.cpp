#include"Date.h"

void wind::Date::Init(int year, int month, int day)
{
	_year = year;
	_month = month;
	_day = day;
}

void wind::Date::Print()
{
	std::cout << _year << "/" << _month << "/" << _day << std::endl;
}

