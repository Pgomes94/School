#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>

#define KEYBOARD_TEST _IOR(0, 6, struct keyboard_input_t)
#define BUFFER_SIZE 100

struct keyboard_input_t {
	char character;
};

static struct keyboard_input_t keyboard_input;

int main() {
	int fd = open("/proc/keyboard_test", O_RDONLY), i = 0;
	char buffer[BUFFER_SIZE];
	char c;

	while (1) {
		ioctl (fd, KEYBOARD_TEST, &keyboard_input);
		c = keyboard_input.character;
		if (c == '\n' || i == BUFFER_SIZE - 1) {
			buffer[i] = '\0';
			printf(" Got from kernel: %s\n", buffer);
			memset(&buffer, '\0', BUFFER_SIZE);
			i = 0;
		}else if (c == '\b' && i > 0) {
			buffer[i] = '\0';
			--i;
		} 
		else if ( i < BUFFER_SIZE ){
			buffer[i] = c;
			++i;
		}
	}
	return 0;
}
