#include "../inc/libmx.h"

char *mx_nbr_to_hex(unsigned long nbr) {
	if(nbr == 0) return "0";
	int remainder, j = 0, size = 0;
	int temp = nbr;
	while (temp != 0) { size++; temp /= 16; }
	char *hexadecimal = mx_strnew(size);
	while (nbr != 0) {
		remainder = nbr % 16;
		if (remainder < 10) hexadecimal[j] = 48 + remainder;
		else hexadecimal[j] = 87 + remainder;
		j++;
		nbr /= 16;
	}
	j--;
	for (int i = 0; i < j; i++, j--) {
		char tmp = hexadecimal[i];
		hexadecimal[i] = hexadecimal[j];
		hexadecimal[j] = tmp;
	}
	return hexadecimal;
}
