#pragma once

double add(double x, double y);
double sub(double x, double y);
double mul(double x, double y);
double idiv(double x, double y);

extern double (*operationTable[])(double, double);