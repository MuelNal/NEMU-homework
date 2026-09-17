#include "trap.h"
#include "FLOAT.h"

int main() { 
	int x=f2F(-1.0);

	nemu_assert(x==0x80010000);

	return 0;
}
