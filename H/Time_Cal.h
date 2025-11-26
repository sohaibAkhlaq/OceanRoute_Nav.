#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

using namespace std;

// Helper: convert "HH:MM" to minutes since midnight
int timeToMinutes(const string &t)
{
    int h = (t[0] - '0') * 10 + (t[1] - '0');
    int m = (t[3] - '0') * 10 + (t[4] - '0');
    return h * 60 + m;
}

// Helper: minutes to "Xh Ym"
string minutesToString(int mins)
{
    if (mins < 0)
        mins = 0;
    int h = mins / 60;
    int m = mins % 60;
    stringstream ss;
    if (h > 0)
        ss << h << "h ";
    if (m > 0 || h == 0)
        ss << m << "m";
    return ss.str().empty() ? "0m" : ss.str();
}

long long toMinutes(const string &date, const string &time)
{
    int d, m, y, hh, mm;
    char c1, c2, c3;

    stringstream ss(date + " " + time);
    ss >> d >> c1 >> m >> c2 >> y >> hh >> c3 >> mm;

    tm t = {};
    t.tm_mday = d;
    t.tm_mon = m - 1;
    t.tm_year = y - 1900;
    t.tm_hour = hh;
    t.tm_min = mm;

// Use timegm instead of mktime to get minutes in UTC
#ifdef _WIN32
    // Windows doesn't have timegm, use _mkgmtime
    return _mkgmtime(&t) / 60;
#else
    return timegm(&t) / 60;
#endif
}

// Converts minutes since midnight or any epoch-based minute count into "HH:MM" format
string minutesToTimeString(long long minutes)
{
    // Make sure minutes is non-negative
    if (minutes < 0)
        minutes = 0;

    // Only keep time within a single day
    int dayMinutes = minutes % 1440; // 1440 = 24*60
    int hh = dayMinutes / 60;
    int mm = dayMinutes % 60;

    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", hh, mm);
    return string(buf);
}
