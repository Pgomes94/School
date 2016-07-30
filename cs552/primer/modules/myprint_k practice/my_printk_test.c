#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define KEYBOARD_TEST _IOWR(0, 7, struct ioctl_test_t)
#define IOCTL_TEST _IOW(0, 6, struct ioctl_test_t)

int main () {

	/* attribute structures */
	struct ioctl_test_t {
		int field1;
		char *field2;
		char field3;
	} ioctl_test;
	
	int fd = open ("/proc/ioctl_test", O_RDONLY);
	
	ioctl_test.field1 = 10;
	ioctl_test.field2 = "abc";
	
	ioctl (fd, KEYBOARD_TEST, &ioctl_test);		
		
	return 0;
}

