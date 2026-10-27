#include "Date.h"

#include <boost/assert.hpp>
#include <iostream>
#include <limits>

void wind::Date::Print() const {
    std::cout << _year << "/" << _month << "/" << _day << std::endl;
}

std::ostream& wind::operator<<(std::ostream& os, const wind::Date& d) {
    return os << d.year() << '-' << d.month() << '-' << d.day();
}

wind::Date::Date() {
    _year = 1;
    _month = 1;
    _day = 1;
}

wind::Date::Date(int year, int month, int day) {
    if (IsValidDate(year, month, day)) {
        std::cout << "错误: 非法的输入参数!" << std::endl;
    }
    _year = year;
    _month = month;
    _day = day;
}

wind::Date::Date(const Date& d) {
    _year = d._year;
    _month = d._month;
    _day = d._day;
}

wind::Date& wind::Date::operator=(const Date& d) {
    if (this != &d) {
        _year = d._year;
        _month = d._month;
        _day = d._day;
    }
    return *this;
}

int wind::Date::year() const {
    return _year;
}

int wind::Date::month() const {
    return _month;
}

int wind::Date::day() const {
    return _day;
}

bool wind::Date::operator!=(const Date& d) const {
    if (_year != d._year || _month != d._month || _day != d._day)
        return true;
    else
        return false;
}

bool wind::Date::operator==(const Date& d) const {
    return !(*this != d);
}

bool wind::Date::operator>(const Date& d) const {
    if (_year > d._year) return true;
    if (_year < d._year) return false;
    if (_month > d._month) return true;
    if (_month < d._month) return false;
    if (_day > d._day) return true;
    return false;
}

bool wind::Date::operator<(const Date& d) const {
    if (*this == d || *this > d)
        return false;
    else
        return true;
}

wind::Date& wind::Date::operator+=(int num) {
    BOOST_ASSERT_MSG(num != std::numeric_limits<int>::min(),
                     "Negation would cause signed integer overflow");
    if (num < 0) {
        return *this -= -num;
    }

    while (num > 0) {
        int remaing = GetDays(_year, _month) - _day;
        if (num <= remaing) {
            _day += num;
            break;
        }

        num -= remaing + 1;
        _day = 1;
        ++_month;
        if (_month > 12) {
            _month = 1;
            ++_year;
        }
    }

    return *this;
}

wind::Date& wind::Date::operator-=(int num) {
    BOOST_ASSERT_MSG(num != std::numeric_limits<int>::min(),
                     "Negation would cause signed integer overflow");
    if (num < 0) return *this += -num;

    while (num > 0) {
        if (num < _day) {
            _day -= num;
            break;
        }

        num -= _day;
        --_month;
        if (_month < 1) {
            --_year;
            _month = 12;
        }
        _day = GetDays(_year, _month);
    }

    return *this;
}

wind::Date& wind::Date::operator++() {
    return *this += 1;
}

wind::Date& wind::Date::operator--() {
    return *this -= 1;
}

wind::Date wind::Date::operator++(int) {
    Date d = *this;
    *this += 1;
    return d;
}

wind::Date wind::Date::operator--(int) {
    Date d = *this;
    *this -= 1;
    return d;
}

wind::Date wind::Date::operator+(int num) const {
    Date d = *this;
    return d += num;
}

wind::Date wind::Date::operator-(int num) const {
    Date d = *this;
    return d -= num;
}

int wind::Date::operator-(const Date& d) const {
    int sign = 1;
    Date later = *this;
    Date earlier = d;

    if (*this < d) {
        sign = -1;
        later = d;
        earlier = *this;
    }

    int count = 0;
    while (earlier != later) {
        ++earlier;
        count++;
    }

    return sign * count;
}

int wind::Date::GetDays(int year, int month) {
    constexpr static int arr[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month != 2) return arr[month];
    if ((year % 4 == 0 && year % 100 != 0) || (year % 100 == 0 && year % 400 == 0))
        return arr[month] + 1;
    else
        return arr[month];
}

int wind::Date::IsValidDate(int year, int month, int day) {
    if (month < 1 || month > 12) return 1;
    if (day < 1 || day > GetDays(year, month)) return 1;
    return 0;
}