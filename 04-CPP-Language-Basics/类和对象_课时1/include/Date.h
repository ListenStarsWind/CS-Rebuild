#pragma once

#include<iostream>


namespace wind
{
	class Date
	{
	public:
		void Init(int year, int month, int day);
		void Print();
	private:
		int _year;
		int _month;
		int _day;
	};
}

