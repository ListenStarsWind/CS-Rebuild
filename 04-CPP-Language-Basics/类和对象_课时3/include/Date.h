#pragma once

#include <ostream>
#include <istream>

namespace wind
{
	class Date
	{
		friend std::ostream& operator<<(std::ostream& out, const Date& d); 
		friend std::istream& operator>>(std::istream& in, Date& d); 

	public:
		Date();
		Date(int year, int month, int day);
		Date(const Date& d);
		int year()const;
		int month()const;
		int day()const;
		void Print()const;
		Date& operator=(const Date& d); 
		bool operator!=(const Date& d)const;
		bool operator==(const Date& d)const;
		bool operator>(const Date& d)const;
		bool operator<(const Date& d)const;
		Date& operator+=(int num);
		Date& operator-=(int num);
		Date& operator++();
		Date& operator--();
		Date operator++(int);
		Date operator--(int);
		Date operator+(int num)const;
		Date operator-(int num)const;
		int operator-(const Date& d)const;

	private:
		static int GetDays(int year, int month);
		static int IsValidDate(int year, int month, int day);

	private:
		int _year = 1;
		int _month = 1;
		int _day = 1;

	};

}

