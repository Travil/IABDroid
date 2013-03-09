#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include "ctbparser.h"
#define MAX_SINGLE_WORD_LENGTH 100
#define MAX_SINGLE_SENT_LENGTH 1000
#define MAX_KEYWORD_SIZE 1000
#define MAX_KEYWORD_COUNT_IN_SINGLE_SENT 10

using namespace std;

int main()
{
	vector<string> dict;
	ctbparser * initialParser();
	ifstream is("operation.set");
	ofstream os("keyword.set");
	ofstream vector_train("vector.train");
	char s[1000];
	char t[2000];
	ctbparser * c = initialParser();
	int i = 0;
	int api_count = 0;
	int dict_length = 0;
	vector<string>::iterator it;
	vector<int * > ia;
	int positon = 0;
	const int max_kw_num_of_single_sent = 10;
	while (!is.eof())
	{
		i = 0;
		is.getline(s, 1000);
		api_count ++;
		c->decode_string(s, t);
		int * vector_map = (int *)malloc(sizeof(int) * (max_kw_num_of_single_sent + dict_length + 1));
		for (int i = 0; i < max_kw_num_of_single_sent + dict_length; i ++)
			vector_map[i] = 0;
		vector_map[max_kw_num_of_single_sent + dict_length] = -1;
		//int vector_map[max_kw_num_of_single_sent + dict_length] = {};
		while (t[i] != 0)
		{
			string str;
			while (t[i] == ' ')
				i ++;
			while (t[i] != 32 && t[i] != 0)
				str += t[i ++];
			if (str.size())
			{
				it = find(dict.begin(), dict.end(), str);
				positon = distance(dict.begin(), it);
				vector_map[positon] = 1;
				if (positon == dict_length)
				{
					dict.push_back(str);
					vector_map[dict_length - 1] = 1;
					dict_length ++;
				}
			}
			while (t[i] == ' ')
				i ++;
			if (t[i] == 0)
				break;
		}
		ia.push_back(vector_map);
	}

	// output the abstracted vector of the api to file "vector.train"
	int vector_length = ceil(log(1.0 * api_count) / log(2.0));
	vector_train << api_count << " " << dict_length << " " << vector_length << endl;
	
	for (int i = 0; i < api_count; i ++)
	{
		int * tmp = ia[i];
		bool end = false;
		for (int j = 0; j < dict_length; j ++)
		{
			int k = 0;
			if (end)
			{
				vector_train << "0 ";
			}
			else
			{
				if (tmp[j] == -1)
				{
					end = true;
					vector_train << "0 ";
				}else
				{
					vector_train << tmp[j] << " ";
				}
			}
		}
		vector_train << endl;
		int index = i;
		int count = vector_length;
		while (index)
		{
			count --;
			vector_train << index % 2 << " ";
			index /= 2;
		}
		while (count --)
		{
			vector_train << "0 ";
		}
		vector_train << endl;
	}

	// output the keyword of the words in the dict;
	cout << "Whole set of the words in the dictionary:" << endl;
	for (int i = 0; i < dict_length; i ++)
	{
		os << dict[i] << endl;
	}
	for (int i = 0; i < api_count; i ++)
	{
		char * vec;
		vec = (char *)malloc(sizeof(api_count));
	}
	char character = getchar();
	return 0;
}

ctbparser * initialParser()
{
	ctbparser *c= new ctbparser();
	if(!c->load_config("config.txt")){
		delete c;
		c=0;
		return NULL;
	}
	return c;
}


//if (str.compare(*it) < 0)
//{
//	it = dict.insert(dict.begin(), str);
//}else
//{
//	for (it = dict.begin(); it != dict.end(); it ++)
//	{
//		if (str.compare(*it) > 0)
//		{
//			dict.push_back(str);
//		}
//	}
//}