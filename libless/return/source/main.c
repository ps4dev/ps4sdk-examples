int main(void)
{
	//return EXIT_SUCCESS;
	return 0x42;
}

//gcc ../example/null.c -o e -I include/ -pie -O3 -Wall -pedantic -std=c11 -Wl,--build-id=none -m64 -mcmodel=large -ffreestanding -nostdlib -nostdinc -fno-builtin
