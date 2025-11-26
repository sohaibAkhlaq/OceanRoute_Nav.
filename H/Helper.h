#pragma once

#include <iostream>
#include<cstring>
#include<string>
using namespace std;

string trimString(string s)
{
    int start = 0;
    int end = s.size() - 1;

    while (start <= end)
    {
        char c = s[start];
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '{' || c == '}' ||
            c == '(' || c == ')' ||
            c == '+' || c == '-' ||
            c == '*' || c == '/')
            break;
        start++;
    }

    while (end >= start)
    {
        char c = s[end];
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '{' || c == '}' ||
            c == '(' || c == ')' ||
            c == '+' || c == '-' ||
            c == '*' || c == '/')
            break;
        end--;
    }

    string result = "";
    for (int i = start; i <= end; i++)
        result += s[i];

    return result;
}

string to_lower(string st) {
    for (char &c : st) {
        c = tolower(c);
    }
    return st;
}

bool isNumber(const string &s)
{
    if (s.empty())
        return false;
    for (char c : s)
        if (c < '0' || c > '9')
            return false;
    return true;
}

const int BLACK   = 30;
const int RED     = 31;
const int GREEN   = 32;
const int YELLOW  = 33;
const int BLUE    = 34;
const int MAGENTA = 35;
const int CYAN    = 36;
const int WHITE   = 37;

// Bright variants
const int BRIGHT_BLACK   = 90;
const int BRIGHT_RED     = 91;
const int BRIGHT_GREEN   = 92;
const int BRIGHT_YELLOW  = 93;
const int BRIGHT_BLUE    = 94;
const int BRIGHT_MAGENTA = 95;
const int BRIGHT_CYAN    = 96;
const int BRIGHT_WHITE   = 97;

string colorText(const string &text, int colorCode)
{
    return "\033[" + to_string(colorCode) + "m" + text + "\033[0m";
}

void clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pauseSystem()
{
    cout << "\033[34m";
#ifdef _WIN32
    cout << endl
         << endl;
    system("pause");
#else
    cout << "\n\nPress Enter to continue...";
    cin.ignore();
    cin.get();
#endif
    cout << "\033[0m";
}


