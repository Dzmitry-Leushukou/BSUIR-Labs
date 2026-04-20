#include "morse.h"
#include <ctype.h>
#include <stddef.h>
typedef struct {
    char ch;
    const char* code;
} morse_pair;

static const morse_pair morse_table[] = {
    {'A', ".-"}, {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},
    {'E', "."},  {'F', "..-."}, {'G', "--."},  {'H', "...."},
    {'I', ".."}, {'J', ".---"}, {'K', "-.-"},  {'L', ".-.."},
    {'M', "--"}, {'N', "-."},   {'O', "---"},  {'P', ".--."},
    {'Q', "--.-"},{'R', ".-."},  {'S', "..."},  {'T', "-"},
    {'U', "..-"},{'V', "...-"}, {'W', ".--"},  {'X', "-..-"},
    {'Y', "-.--"},{'Z', "--.."},
    {'0', "-----"}, {'1', ".----"}, {'2', "..---"}, {'3', "...--"},
    {'4', "....-"}, {'5', "....."}, {'6', "-...."}, {'7', "--..."},
    {'8', "---.."}, {'9', "----."},
    {' ', "/"},
};

const char* morse_encode(char c) {
    c = toupper((unsigned char)c);

    for (unsigned int i = 0; i < sizeof(morse_table)/sizeof(morse_table[0]); ++i) {
        if (morse_table[i].ch == c) {
            return morse_table[i].code;
        }
    }
    return NULL;
}
