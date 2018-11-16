#include "seq_Kmeans_Manager.h"
#include <stdio.h>
#include <fstream>

using namespace std;

const string INPUT_FILE_NAME = /*"input3.txt"*/"INPUT_FILE.txt";
const string OUTPUT_FILE_NAME = "output.txt";

void main(int argc, char *argv[])
{
	Seq_Kmeans_Manager* seq_kmm = new Seq_Kmeans_Manager(INPUT_FILE_NAME);
	ofstream outputFile(OUTPUT_FILE_NAME);

	if (seq_kmm->runSequencialAlgorithm())
	{
		cout << "found clusters position !" << endl;
		outputFile << *seq_kmm;

	}
	else
	{ 
		cout << "clusters position with desired QM were not found!" << endl;
		outputFile << *seq_kmm;
	}
	


	
	
}