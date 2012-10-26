#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "207layer.h"

void die (char *s){
	perror(s);
	exit(1);
}

void check_for_error(int ret, char* s){
	if (ret < 0){
		die (s);
	}
}

