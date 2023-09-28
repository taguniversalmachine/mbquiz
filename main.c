/*
 * C Quize
 *
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2023, Middleby Corp.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/*
 * Compile this file together with the UnQLite database engine source code
 * to generate the executable. For example:
 *  gcc -W -Wall -O6 unqlite_doc_intro.c unqlite.c -o unqlite_doc
*/
/*
 * This simple program is a quick introduction on how to embed and start
 * experimenting with UnQLite without having to do a lot of tedious
 * reading and configuration.
 *
 * Introduction to the UnQLite Document-Store Interfaces:
 *
 * The Document store to UnQLite which is used to store JSON docs (i.e. Objects, Arrays, Strings, etc.)
 * in the database is powered by the Jx9 programming language.
 *
 * Jx9 is an embeddable scripting language also called extension language designed
 * to support general procedural programming with data description facilities.
 * Jx9 is a Turing-Complete, dynamically typed programming language based on JSON
 * and implemented as a library in the UnQLite core.
 *
 * Jx9 is built with a tons of features and has a clean and familiar syntax similar
 * to C and Javascript.
 * Being an extension language, Jx9 has no notion of a main program, it only works
 * embedded in a host application.
 * The host program (UnQLite in our case) can write and read Jx9 variables and can
 * register C/C++ functions to be called by Jx9 code.
 *
 * For an introduction to the UnQLite C/C++ interface, please refer to:
 *        http://unqlite.org/api_intro.html
 * For an introduction to Jx9, please refer to:
 *        http://unqlite.org/jx9.html
 * For the full C/C++ API reference guide, please refer to:
 *        http://unqlite.org/c_api.html
 * UnQLite in 5 Minutes or Less:
 *        http://unqlite.org/intro.html
 * The Architecture of the UnQLite Database Engine:
 *        http://unqlite.org/arch.html
 */
/* $SymiscID: unqlite_doc_intro.c v1.0 FreeBSD 2013-05-17 15:56 stable <chm@symisc.net> $ */
/*
 * Make sure you have the latest release of UnQLite from:
 *  http://unqlite.org/downloads.html
 */
#include <stdio.h>  /* puts() */
#include <stdlib.h> /* exit() */
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "unqlite.h"
#include "jx9.h"

int verbose_flag=0;
int profile_flag = 0;
int counter_mode = 0;

static void Fatal(unqlite *pDb,const char *zMsg);

/*
 * Banner
 */
static const char zBanner[] = {
	"============================================================\n"
	"Quiz                                                        \n"
	"Copyright (c) 2023 Middleby Corp.                           \n"
	"============================================================\n"
};

/* Forward declaration: VM output consumer callback */
static int VmOutputConsumer(const void *pOutput,unsigned int nOutLen,void *pUserData /* Unused */);

unsigned int uint_limit( unsigned int lower, unsigned int upper, unsigned int val){
    return ((val < lower) ? lower : (val > upper ? upper : val));
}

/* Generate a given number of blocks
   Return a byte array

jx9_value_int()
jx9_value_int64()
jx9_value_bool()
jx9_value_null()
jx9_value_double()
jx9_value_string()
jx9_value_string_format()
jx9_value_resource()

*/
int foo(unqlite_context *pCtx, int argc, unqlite_value **argv)
{
  unqlite_value *pArray;    /* Our JSON Array */
  unqlite_value *pValue;    /* Objecr entries value */
  printf("Starting native foo function");
  /* Create a new JSON object */
  pArray = unqlite_context_new_array(pCtx);
  /* Create a worker scalar value */
  pValue = unqlite_context_new_scalar(pCtx);

  if( argc < 1 || !unqlite_value_is_string(argv[0]) ){
     unqlite_context_throw_error(pCtx, JX9_CTX_WARNING, "Argument must be a string");
		/* Return 33 */
		unqlite_result_int(pCtx, 33);
		return UNQLITE_OK;
	}

  for (int i=0; i<128; i++){
	unqlite_value_int(pValue, i);
	unqlite_array_add_elem(pArray, NULL, pValue);
  }
  /* Return the array as the function return value */
  unqlite_result_value(pCtx, pArray);

   return UNQLITE_OK;
}

static void TIME_Constant(jx9_value *pValue, void *pUserData /* Unused */)
{
	struct tm *pLocal;
	time_t tt;
	/* Get the current local time */
	time(&tt);
	pLocal = localtime(&tt);
	/* Expand the current time now */
	jx9_value_string_format(pValue, "%02d:%02d:%02d",
		pLocal->tm_hour,
		pLocal->tm_min,
		pLocal->tm_sec
		);
}

int main(int argc,char *argv[])
{
	unqlite *pDb;       /* Database handle */
    unqlite_vm *pVm = NULL; /* Compiled JX9 program */
	jx9_value *pScalar,*pObject; /* Foreign JX9 variable to be installed later */
	int rc;
    const char *script_filename = "doc_intro.jx9";// "boot.jx9";
	int show_usage = 0;
    int c;
	int server_port = 0;
    char* dbfile_name;
	puts(zBanner);

	/* Get options */
	while (1)
     {
       static struct option long_options[] =
         {
           /* These options set a flag. */
           {"verbose", no_argument,     &verbose_flag, 1},
           /* These options don't set a flag.
              We distinguish them by their indices. */
           {"database",  required_argument,         0, 'd'},  // File to use for db
           {"output",  required_argument,       0, 'o'},  // Write to file
           {"help",    no_argument,             0, 'h'},  // Usage
           {0, 0, 0, 0}
         };
       /* getopt_long stores the option index here. */
       int option_index = 0;

       c = getopt_long (argc, argv, "d:o:f:s:h?",
                        long_options, &option_index);

       /* Detect the end of the options. */
       if (c == -1)
         break;

       switch (c)
         {
         case 0:
           /* If this option set a flag, do nothing else now. */
           if (long_options[option_index].flag != 0)
             break;
           printf ("option %s", long_options[option_index].name);
           if (optarg)
             printf (" with arg %s", optarg);
           printf ("\n");
           break;

         case 'd':
		   dbfile_name = optarg;
		   printf("Database file: %s\n", dbfile_name);
		   break;

		 case 'p':
		   server_port = atoi(optarg);
		   printf("Port %d\n", server_port);
		   break;

         case '?':
           /* getopt_long already printed an error message. */
           show_usage = 1;
           break;

        case 'h':
           show_usage = 1;
           break;

         default:
           show_usage = 1;
         }
         if (verbose_flag) { printf ("Processing command-line options\n");}
     }


	/* Open our database */
	if ((argc > 1) && (dbfile_name)) {
	   printf("Opening database in %s\n", dbfile_name);
       rc = unqlite_open(&pDb,dbfile_name,UNQLITE_OPEN_CREATE);
	} else {
	   printf("Opening in-memory DB\n");
       rc = unqlite_open(&pDb,":mem:" /* In-mem DB */,UNQLITE_OPEN_CREATE);
	}

	if( rc != UNQLITE_OK ){
		Fatal(0,"Out of memory");
	}

    printf("Compiling %s\n", script_filename);

	/* Compile our Jx9 script loaded above */
	rc = unqlite_compile_file(pDb, script_filename, &pVm);
//	rc = jx9_compile_file(pDb,filename, &pVm);
	if( rc != UNQLITE_OK ){
			printf("Compile failed, Extracting error log\n");
			const char *zBuf;
		    int iLen;
			unqlite_config(pDb,
				UNQLITE_CONFIG_JX9_ERR_LOG,
				&zBuf,
				&iLen
				);
			if( iLen > 0 ){
				/* zBuf is null terminated */
				puts(zBuf);
			}

		/* Exit */
		Fatal(0, "Compile error");
	} else {
		printf("Compiled OK\n");
	}

    /* Install foreign constants */
	rc = unqlite_create_constant(pVm, "__TIME__", TIME_Constant, 0);
    if( rc != UNQLITE_OK ){
		Fatal(0,"Error while installing the __TIME__ constant");
	}

	/* Install foreign vars */
	/*
     * Create a simple scalar variable.
     */

    pScalar = unqlite_vm_new_scalar(pVm);

    if( pScalar == 0 ){
      Fatal(0, "Cannot create foreign variable $my_app");
    }

    /* Populate the variable with the desired information */
    unqlite_value_int(pScalar, 3);

   /*
    * Install the variable ($my_app).
    */
	unqlite_vm_config(pVm, UNQLITE_VM_CONFIG_CREATE_VAR, "my_app", pScalar);

    if (rc != UNQLITE_OK) {
		Fatal(0, "Error while installing $my_app");
	}

    /*
	 * Install foreign function
	 */
	rc = unqlite_create_function(pVm, "foo", foo, NULL);

    /* Install a VM output consumer callback */
	rc = unqlite_vm_config(pVm,UNQLITE_VM_CONFIG_OUTPUT,VmOutputConsumer,0);
	if( rc != UNQLITE_OK ){
		Fatal(pDb,0);
	} else {
		printf("Ouput consumer installed\n");
	}


  //rc = jx9_vm_config(
   //      pVm,
   //      JX9_VM_CONFIG_CREATE_VAR, /* Create variable command */
   //      "my_app", /* Variable name (without the dollar sign) */
   //       pScalar /* Value */
   //    );

   // if( rc != JX9_OK ){
    //   Fatal(0,"Error while installing $my_app");
   // }

	/* Execute our script */
	rc = unqlite_vm_exec(pVm);
	if( rc != UNQLITE_OK ){
		Fatal(0, "JX9 Execution failed");
	} else {
		printf("\nExecution ok\n");
	}

	/* Release our VM */
    unqlite_vm_release(pVm);
//	jx9_release(pEngine);
	/* Auto-commit the transaction and close our database */
	unqlite_close(pDb);
	return 0;
}

#ifdef __WINNT__
#include <Windows.h>
#else
/* Assume UNIX */
#include <unistd.h>
#endif
/*
 * The following define is used by the UNIX build process and has
 * no particular meaning on windows.
 */
#ifndef STDOUT_FILENO
#define STDOUT_FILENO	1
#endif
/*
 * VM output consumer callback.
 * Each time the UnQLite VM generates some outputs, the following
 * function gets called by the underlying virtual machine to consume
 * the generated output.
 *
 * All this function does is redirecting the VM output to STDOUT.
 * This function is registered via a call to [unqlite_vm_config()]
 * with a configuration verb set to: UNQLITE_VM_CONFIG_OUTPUT.
 */
static int VmOutputConsumer(const void *pOutput,unsigned int nOutLen,void *pUserData /* Unused */)
{
#ifdef __WINNT__
	BOOL rc;
	rc = WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),pOutput,(DWORD)nOutLen,0,0);
	if( !rc ){
		/* Abort processing */
		return UNQLITE_ABORT;
	}
#else
	ssize_t nWr;
	nWr = write(STDOUT_FILENO,pOutput,nOutLen);
	if( nWr < 0 ){
		/* Abort processing */
		return UNQLITE_ABORT;
	}
#endif /* __WINT__ */

	/* All done, data was redirected to STDOUT */
	return UNQLITE_OK;
}

/*
 * Extract the database error log and exit.
 */
static void Fatal(unqlite *pDb,const char *zMsg)
{
	if( pDb ){
		const char *zErr;
		int iLen = 0; /* Stupid cc warning */

		/* Extract the database error log */
		unqlite_config(pDb,UNQLITE_CONFIG_ERR_LOG,&zErr,&iLen);
		if( iLen > 0 ){
			/* Output the DB error log */
			puts(zErr); /* Always null termniated */
		}
	}else{
		if( zMsg ){
			puts(zMsg);
		}
	}
	/* Manually shutdown the library */
	unqlite_lib_shutdown();
	/* Exit immediately */
	exit(0);
}