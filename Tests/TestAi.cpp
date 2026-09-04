
#include <doesnotexists>

int main(int argc, char* argv[])
{
	int* ptr = nullptr;
	int* str = new int[15];

	*ptr = *ptr + argc;
	if (argc == 2) 
	{
		delete str;
	}
	return *ptr;
}