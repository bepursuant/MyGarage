// takes an html file input and viewname and creates a
// file with a PROGMEM statement that contains the
// original html

#include <fstream>
#include <sstream>
#include <cstring>

using namespace std;

int main(int argc, char* argv[])
{
	// check if we were provided enough arguments, if not gripe about it
	if(argc<3) {
		printf("Usage: %s inputHtmlFile.html viewname\n", argv[0]);
		return 0;
	}

	// output file name - hard coded for now, could easily be an input
	char oname[] = "CompiledAssets.h";

	// open a pointer to the input file and if we can't, toss a wobbly
	ifstream inFile (argv[1]);
	if (!inFile.is_open())
	{
		printf("Can't open input file %s\n", argv[1]);
		return 0;
	}

	// open a pointer to the output file and if we can't, toss a different wobbly
	ofstream outFile(oname, fstream::in | fstream::out | fstream::app);
	if(!outFile.is_open()) {
		printf("Can't open output file %s\n", oname);
		return 0;
	}

	// to hold the lines pulled from the input file
	string line; 

	// start by writing our 'header' for this view to the file
	outFile << "const char ";
	outFile << argv[2];
	outFile << "[] PROGMEM = R\"=====(";

	// read the input file line by line, left trim the line
	// then write to the output file
	while(getline(inFile, line))
	{
		// trim leading spaces
		istringstream iss(line);
		size_t startpos = line.find_first_not_of(" \t");
		if(string::npos != startpos)
		  line = line.substr(startpos);

		if(line.length()>0)
			outFile << line << "\r\n";
	}

	outFile << ")=====\";\n";

	inFile.close();
	outFile.close();
}