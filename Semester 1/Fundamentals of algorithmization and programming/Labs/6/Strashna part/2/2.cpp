#include<iostream>
#include "D:\Programming\BSUIR\Labs\Hat.h"

#define ll long long


const ll word_size = 100 * sizeof(char);


ll insert(char *(&a), ll text_index, char c[], ll pref_size)
{

	if (c[pref_size] == -1)
		pref_size--;


	for (int q = 0; q <= pref_size; q++, text_index++)
			a[text_index] = c[q];



	return text_index;
}


bool letter(char a)
{
	return (a >= 'A' && a <= 'Z') || (a >= 'a' && a <= 'z');
}

int main()
{
	ll text_len, p_len, buf, word_len, edit_len;
	char input;


	hat(2);
	std::cout << "In the text of a word of a given length, replace the specified substring,\nthe length of which may not coincide with the fourth word\n\n\n";


	while (true)
	{

		text_len = p_len=0;
		buf = 1;
		char* text = (char*)malloc(sizeof(char));
		char* p = (char*)malloc(word_size);

		std::cout << "Write lenght: ";
		edit_len = cinll();

		input = getchar();
		std::cout << "Write substring: ";


		do
		{
			input = getchar();
			
			if (input == '\n')
			{
				break;
			}

			
			
			if(p_len+1>=100*buf)
			{ 
				buf++;
				p=(char*)realloc(p, word_size * buf);
			}


			p[p_len] = input;
			p_len++;
		} while (true);


		p_len--;
		buf = 1;
		
		// read text
		std::cout << "Write text:\n";


		do
		{
			//read word
			char *word = (char*)malloc(word_size);
			word_len = -1;


			do
			{
				input = getchar();
				if (!letter(input))
					break;

				if (word_len >= word_size * buf)
				{
					buf++;
					word=(char*)realloc(word, word_size * buf);
				}

				word_len++;
				word[word_len] = input;
				

			} while (true);


			if (word_len > 0)
			{


				if (word_len+1 != edit_len)
				{
					text = (char*)realloc(text, (text_len + word_len + 2) * sizeof(char));
					text_len = insert(text, text_len, word, word_len);
				}
				else
				{
					text = (char*)realloc(text, (text_len + p_len + 1) * sizeof(char));
					text_len = insert(text, text_len, p, p_len);
				}


			}



			if (input == -1)
				break;


			text = (char*)realloc(text, (text_len + 1) * sizeof(char));
			text[text_len] = input;
			text_len++;


			free(word);
			word = nullptr;

		} while (true);


		for (ll i = 0; i < text_len; i++)
		{
			std::cout << text[i];
		}


		free(text);
		free(p);
		text = p = nullptr;

	

	}
}
/*
3
SSS
rDY dsd. dads,ddasda ddd!

3
SSS
The text about xuy, xyi.com, xuiZALUPI, Now we will check enter blyat 
good?two enters


poxuy naverno work
*/