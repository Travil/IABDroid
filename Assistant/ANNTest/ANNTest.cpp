#include <stdio.h>
#include <fstream>
#include "../../src/include/floatfann.h"
using namespace std;
#define OUTPUT_NUM 6
#define INPUT_NUM 52

int main()
{
	fann_type *calc_out;
	ifstream is("src.txt");
	fann_type input[INPUT_NUM];

	struct fann *ann = fann_create_from_file("vector.net");

	for (int i = 0; i < INPUT_NUM; i ++)
	{
		is >> input[i];
	}
	
	calc_out = fann_run(ann, input);
/*
	printf("vector test -> %f\n", calc_out[0]);
	printf("vector test -> %f\n", calc_out[1]);*/
	int i = 0;
	while (i < OUTPUT_NUM)
		printf("vector test -> %f\n", calc_out[i ++]);

	fann_destroy(ann);
	return 0;
}

