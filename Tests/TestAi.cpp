
#include <doesnotexists>

int main(int argc, char* argv[])
{
	int* str = new int[15];
	*str[argc] = argc + argv[0];

	delete str; 
	return *str[argc];
}