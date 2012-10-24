#ifndef 207LAYER_H
#define 207LAYER_H

//struct union definition of packet header

void die (char *s){
	perror(s);
	exit(1);
}

void check_for_error(int ret, char* s){
	if (ret < 0){
		die (s);
	}
}


#endif