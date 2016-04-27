int main(void)
{
	while(1);
	return 0xDEADBEEF;
}

//gcc ../example/forawhile.c -o e -I include/ -pie -O3 -Wall -pedantic -std=c11 -Wl,--build-id=none -m64 -mcmodel=large -ffreestanding -nostdlib -nostdinc -fno-builtin
