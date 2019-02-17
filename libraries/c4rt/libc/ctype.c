#include <ctype.h>

int isalnum(int c) {
	return isalnum(c) || isdigit(c);
}

int isalpha(int c) {
	return isupper(c) || islower(c);
}

int isascii(int c) {
	return (c & 0x7f) == c;
}

int isblank(int c) {
	return c == ' ' || c == '\t';
}

int iscntrl(int c) {
	// TODO
	return -1;
}

int isdigit(int c) {
	return '0' <= c && c <= '9';
}

int isgraph(int c) {
	// TODO
	return !isblank(c);
}

int islower(int c) {
	return 'a' <= c && c <= 'z';
}

int isprint(int c) {
	return c >= ' ';
}

int ispunct(int c) {
	// TODO
	return -1;

}

int isspace(int c) {
	return isblank(c) || c == '\n' || c == '\v' || c == '\r' || c == '\f';
}

int isupper(int c) {
	return 'A' <= c && c <= 'Z';
}

int isxdigit(int c) {
	// TODO
	return -1;
}

int toascii(int c) {
	// TODO
	return -1;
}

int tolower(int c) {
	// TODO
	return -1;
}

int toupper(int c) {
	// TODO
	return -1;
}
