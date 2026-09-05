
#include <doesnotexists>

int main(int argc, char* argv[])
{
	int* str1 = new int[15];
	*str1[argc] = argc + argv[0];

	delete str1; 
	return *str1[argc];
}