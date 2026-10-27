#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE DateTests
#include <boost/test/unit_test.hpp>

#include "Date.h"

using wind::Date;

namespace {
bool is_date(const Date& d, int year, int month, int day)
{
    return d.year() == year && d.month() == month && d.day() == day;
}
} // namespace

BOOST_AUTO_TEST_CASE(construction_and_assignment)
{
    Date def;
    BOOST_TEST(is_date(def, 1, 1, 1));

    Date d(2024, 2, 28);
    BOOST_TEST(is_date(d, 2024, 2, 28));

    Date assigned;
    assigned = d;
    BOOST_TEST(assigned == d);
    BOOST_TEST(is_date(assigned, 2024, 2, 28));
}

BOOST_AUTO_TEST_CASE(comparisons)
{
    Date a(2024, 2, 28);
    Date b(2024, 3, 1);
    Date c(2024, 2, 28);

    BOOST_TEST(a == c);
    BOOST_TEST(!(a != c));
    BOOST_TEST(a != b);
    BOOST_TEST(a < b);
    BOOST_TEST(b > a);
    BOOST_TEST(!(a > b));
    BOOST_TEST(!(b < a));
}

BOOST_AUTO_TEST_CASE(date_arithmetic)
{
    const Date a(2024, 2, 28);
    BOOST_TEST(is_date(a + 1, 2024, 2, 29));
    BOOST_TEST(is_date(a + 2, 2024, 3, 1));
    BOOST_TEST(is_date(a - 1, 2024, 2, 27));
    BOOST_TEST(is_date(a, 2024, 2, 28));

    Date b(2023, 12, 31);
    Date& added = (b += 1);
    BOOST_TEST(&added == &b);
    BOOST_TEST(is_date(b, 2024, 1, 1));

    Date& subtracted = (b -= 1);
    BOOST_TEST(&subtracted == &b);
    BOOST_TEST(is_date(b, 2023, 12, 31));

    BOOST_TEST(is_date(Date(2023, 3, 1) - 1, 2023, 2, 28));
}

BOOST_AUTO_TEST_CASE(increment_and_decrement)
{
    Date d(2024, 2, 28);
    Date& pre_inc = ++d;
    BOOST_TEST(&pre_inc == &d);
    BOOST_TEST(is_date(d, 2024, 2, 29));

    Date old_inc = d++;
    BOOST_TEST(is_date(old_inc, 2024, 2, 29));
    BOOST_TEST(is_date(d, 2024, 3, 1));

    Date old_dec = d--;
    BOOST_TEST(is_date(old_dec, 2024, 3, 1));
    BOOST_TEST(is_date(d, 2024, 2, 29));

    Date& pre_dec = --d;
    BOOST_TEST(&pre_dec == &d);
    BOOST_TEST(is_date(d, 2024, 2, 28));
}

BOOST_AUTO_TEST_CASE(date_difference)
{
    Date a(2024, 2, 28);
    Date b(2024, 3, 1);
    BOOST_TEST((b - a) == 2);
    BOOST_TEST((a - b) == -2);
    BOOST_TEST((a - a) == 0);
}
