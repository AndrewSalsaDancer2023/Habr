#include <stdio.h>
#include <stdlib.h>
#include "lib/pt.h"

#define LINE_LENGTH 256

struct line_task 
{
	unsigned long lc;
	struct pt state;
};

struct file_task 
{
	FILE* stream;
	char* line;
	int line_length;
	struct pt state;
};

PT_THREAD(line_counter(struct line_task *tsk))
{
	PT_BEGIN(&tsk->state);
	while(1) 
	{
		PT_YIELD(&tsk->state);
		tsk->lc++;
	}
  
	PT_END(&tsk->state);
}

PT_THREAD(line_reader(struct file_task *fltsk))
{
	PT_BEGIN(&fltsk->state);
	while(1) 
	{
		if(!fgets(fltsk->line, fltsk->line_length, fltsk->stream))
      		break;
		PT_YIELD(&fltsk->state);
	}
  
	PT_END(&fltsk->state);
}

int main(int argc, char *argv[])
{
	char line[LINE_LENGTH];   
	struct file_task fltsk;

	if (argc != 2) 
	{
		printf("Usage : %s <file_name>\n", argv[0]);
		return EXIT_FAILURE;
    }

    fltsk.stream = fopen(argv[1], "r");
    
    if (!fltsk.stream) 
	{
		printf("File not found\n");
		return EXIT_FAILURE;
    }
	fltsk.line = (char *)&line;
	fltsk.line_length = sizeof(line);
   
	PT_INIT(&fltsk.state);
    
	struct line_task tsk;
	tsk.lc=1;

	PT_INIT(&tsk.state);

	while(line_reader(&fltsk) != PT_ENDED) 
	{
		line_counter(&tsk);
		printf("%lu:", tsk.lc);
		printf("%s", fltsk.line);
	}

	fclose(fltsk.stream);
	return EXIT_SUCCESS;
}
