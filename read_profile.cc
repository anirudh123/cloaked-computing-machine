#include<stdio.h>
#include<stdlib.h>
#include<string.h>

char** readFile(const char *filename, size_t *lineCount){
	FILE *fp;
	char buff[4096];
	size_t lines = 0, capacity=1024;
	char **line;

	if(NULL==(fp=fopen(filename, "r"))){
    		perror("file can't open.");
		return NULL;
	}
	if(NULL==(line=(char**)malloc(sizeof(char*)*capacity))){
		perror("can't memory allocate.");
		fclose(fp);
	    	return NULL;
	}
	while(NULL!=fgets(buff, sizeof(buff), fp)){
	    line[lines++] = strdup(buff);
	    if(lines == capacity){
		    capacity += 32;
		    if(NULL==(line=(char**)realloc(line, sizeof(char*)*capacity))){
			perror("can't memory allocate.");
		        fclose(fp);
			return NULL;
		    }
	    }
       }
    *lineCount = lines;
    fclose(fp);
    return (char**)realloc(line, sizeof(char*)*lines);
}

void freeMem(char** p, size_t size){
	size_t i;

	if(p==NULL) return;

	for(i=0;i<size;++i)
		free(p[i]);
    	free(p);
}

int main()
{
	char* file_name = "/root/.profile";
	size_t lines; 
	char **line;

	if(NULL==(line=readFile(file_name, &lines)))
	{
		printf("Error reading file: %s\n", file_name);
		return 1;
	}

	for(int i=0; i < lines; i++)
	{
		printf("%s\n", line[i]);
	}
	freeMem(line, lines);
	return 0;
}
