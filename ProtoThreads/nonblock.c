#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "lib/pt.h"

#define READ_SIZE 256

struct read_task {
	char* line;
	int line_length;
	int index;
	ssize_t bytes_read;
	struct pt state;
};

PT_THREAD(stdin_reader(struct read_task *rdtsk))
{
  PT_BEGIN(&rdtsk->state);
  while(1) {
	PT_YIELD(&rdtsk->state);
	
	rdtsk->bytes_read = read(STDIN_FILENO, &rdtsk->line[rdtsk->index], rdtsk->line_length);
	
	if((rdtsk->bytes_read < 0) && (errno != EAGAIN) && (errno != EWOULDBLOCK))
		break;
		
	if(rdtsk->bytes_read >= 0)
	{
	rdtsk->index += rdtsk->bytes_read;
	if(rdtsk->line[rdtsk->index-1] == '\n') {
		rdtsk->line[rdtsk->index] = '\0';
		break;
	}
        }
	}
  
  PT_END(&rdtsk->state);
}

int set_stdin_nonblock_mode() {
    // Устанавливаем stdin в неблокирующий режим
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1) {
        perror("Error fcntl getting flag F_GETFL\n");
        return EXIT_FAILURE;
    }
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("Error fcntl setting flag F_SETFL\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void init_reader(struct read_task *rtask, char* buffer, int size) {
    rtask->line = buffer;
    rtask->line_length = size;
    rtask->index = 0;
    PT_INIT(&rtask->state);
}

int main(void) {
    char buffer[READ_SIZE];
    struct read_task rtask;

    if (set_stdin_nonblock_mode())
    return EXIT_FAILURE;

    init_reader(&rtask, (char *)&buffer, sizeof(buffer));

     printf("Enter some string:\n");
    do{
    }while(stdin_reader(&rtask) != PT_ENDED);
    
    if(rtask.bytes_read >= 0)
		printf("Read: %s \n", rtask.line);
	else 
		printf("Error reading from STDIN\n");

  return EXIT_SUCCESS;
}

