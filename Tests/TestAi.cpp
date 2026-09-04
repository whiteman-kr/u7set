
#include <doesnotexists>

int main(int argc, char* argv[])
{
		int* ptr = nullptr;
		if (ptr == nullptr) 
		{
			*ptr = 42;
			return 1;
		}
	else
	{
		*ptr = 44;
		return *ptr;
	}
}