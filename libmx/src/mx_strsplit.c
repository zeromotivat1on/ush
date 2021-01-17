#include "../inc/libmx.h"

char **mx_strsplit(char const *s, char c) {
	if(!s) return 0;
	int count_words = mx_count_words(s, c);
	char **res = (char**)malloc(count_words * 8);
	int j = 0;
	for (int i = 0; i < count_words; ++i) {
		int word_length = 0;
		int start = 0;
		for (; s[j]; ++j) {
			if (s[j] != c) { 
				word_length++;
				if (start == 0) start = j;
			}
			else if (s[j] == c && word_length) break;
		}
		char *new_word = mx_strnew(word_length);
		for (int k = 0, m = start - 1; m < j; ++k, ++m) new_word[k] = s[m];
		res[i] = new_word;
	}
	res[count_words] = NULL;
	for (int i = 0; i < count_words; ++i) printf("%s", res[i]);
	return res;
}
