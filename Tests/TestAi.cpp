
#include <doesnotexists>

int main(int argc, char* argv[])
{
	int* str = new int[15];
	int* ptr = nullptr;
	
	*ptr = *ptr + argc;
	if (argc == 2) 
	{
	delete str; 
	}
	return *ptr;
}