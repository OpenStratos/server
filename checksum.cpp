#include <iostream>
#include <string>
#include <regex>
using namespace std;

int checksum(string frame)
{
	int checksum = 0;
	for (char c : frame)
	{
		if (c == '$') continue;
		if (c == '*') break;

		checksum ^= c;
	}

	return checksum;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		cout << "You must provide a frame to calculate it's checksum" << endl;
		return 0;
	}

	if (argc > 2)
	{
		cout << "You must provide only one argument to calculate it's checksum" << endl;
		return 0;
	}

	string frame(argv[1]);
	regex frame_regex("\\$[A-Z][0-9a-zA-Z\\.,-]*\\*[0-9A-F]{1,2}");

	if ( ! regex_match(frame, frame_regex))
	{
		cout << "You must provide a valid frame" << endl;
		return 0;
	}

	cout << "The computed checksum is: " << hex << uppercase << checksum(frame) << endl;
}