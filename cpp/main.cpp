
#include <stdio.h>

class shape
{
public:
	int type;
	shape(int type)
	{
		this->type = type;
	}
	void print(void)
	{
		asm("reset");
	}
};

int main (int argc, char *argv[])
{
	shape x(7);
	shape *y;
	y = &x;
	y->print();
	return 0;
}


extern "C" {

void _start(void)
{
	main(0, NULL);
	while(1)
	{
		;
	}
}

};
