#include "def/ascii.h"
#include <Arduino.h>

// Count NL chars inside a string, useful to count menu rows
unsigned short rowCount(String menu) {
    unsigned short count = 0;
    for (unsigned short i = 1; i < menu.length(); i++) {
        if (menu.charAt(i) == ASCII_NL) {
            count++;
        }
    }
    return count;
}

// Get a row text
String rowGet(String menu, unsigned short row) {
    unsigned short count = 0;
    unsigned short last = 0;
    for (unsigned short i = 0; i < menu.length(); i++) {
        if (menu.charAt(i) == ASCII_NL) {
            if (count == row) {
                return menu.substring(last, i);
            }
            count++;
            last = i + 1;
        }
    }
    return "MENU ERROR! (Unknown row?)";
}
