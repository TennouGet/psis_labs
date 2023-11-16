#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>


/* include the correct .h file */


int main(){
	int a;
	char line[100];
	char library_name[100];

	printf("What version of the functions you whant to use?\n");
	printf("\t1 - Normal    (lib1)\n");
	printf("\t2 - Optimized (lib2)\n");
	fgets(line, 100, stdin);
	sscanf(line,"%d", &a);
	if (a == 1){
		strcpy(library_name, "./lib1.so");
		printf("running the normal versions from %s \n", library_name);
	}else{
		if(a== 2){
			strcpy(library_name, "./lib2.so");
			printf("running the normal versions %s \n", library_name);
		}else{
			printf("Not running anything\n");
			exit(-1);
		}
	}
	/* load library from name library_name */

	void *handle;

	if (a == 1){
		handle = dlopen("lib1.so", RTLD_LOCAL | RTLD_LAZY);
	}
	if (a == 2){
		handle = dlopen("lib2.so", RTLD_LOCAL | RTLD_LAZY);
	}
	/* declare pointers to functions */

	void *lib_handle;
	double (*fn)(int *);

	int x;

	/*load func_1 from loaded library */

	lib_handle = dlopen("lib/lib1.so", RTLD_LAZY);

	/*load func_2 from loaded library */

	fn = dlsym(lib_handle, "func_1"); 

	/* call func_1 from whichever library was loaded */

	(*fn)(&x);
	dlclose(lib_handle);

	/* call func_2 from whichever library was loaded */

	exit(0);
}


