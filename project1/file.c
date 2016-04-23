#include <string.h>
#include "constant.h"
#include "file.h"

/* Description : open file
 * Return value : File pointer
 * if error occurs, *err represent error code
 */
FILE* open_file(int *err, char* filename, int is_internal){
	FILE *fp;
	char *tmp;
	
	fp = fopen(filename, "r");
	if(fp == NULL){
		*err = 12;
		return NULL;
	}
	if(!is_internal)
		return fp;

	if((tmp = strtok(NULL, " "))){
		fclose(fp);
		*err =  6;
		return NULL;
	}

	return fp;
}

/* Description : print file content
 * return value : if error, return error value
 *                if do well, return 0
 */
int command_type(){
	FILE *fp;
	int c, error = 0;
	char *tmp;
	char filename[MAX_STR_LENGTH];

	tmp = strtok(NULL, " ");

	strcpy(filename, tmp);
	fp = open_file(&error, filename, 1);
	if(error > 0)
		return error;

	while((c = fgetc(fp)) != EOF)
		fputc(c,stdout);
	
	fclose(fp);
	return 0;
}
