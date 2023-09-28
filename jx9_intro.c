/*
 * Compile this file together with the jx9 scripting engine source code to generate
 * the executable. For example:
 *  gcc -W -Wall -O6 -o test jx9_intro.c jx9.c
*/
/*
 * This simple program is a quick introduction on how to embed and start
 * experimenting with the JX9 scripting engine without having to do a lot
 * of tedious reading and configuration.
 *
 * For an introduction to the JX9 C/C++ interface, please refer to this page
 *        http://jx9.symisc.net/api_intro.html
 * For the full C/C++ API reference guide, please refer to this page
 *        http://jx9.symisc.net/c_api.html
 * For the full list of built-in functions (over 303), please refer to this page
 *        http://jx9.symisc.net/builtin_func.html
 * For the full JX9 language reference manual, please refer to this page
 *        http://jx9.symisc.net/jx9_lang.html
 */
/*
 * The following is the JX9 program to execute.
 *   $my_conf = {
 *    // Greeting message
 *    greeting : "Hello world!\n",
 *	  // Dummy field
 *	  __id: 1,
 *	  // Host Operating System
 *	  os_name: uname(),
 *	  // Current date
 *	  date : __DATE__,
 *	  // Return the current time using an anonymous function
 *	  time : function(){ return __TIME__; }
 *  };
 *  // invoke object fields
 *  print $my_conf.greeting;
 *  print "Host Operating System: ",$my_conf.os_name,JX9_EOL;
 *  print "Current date: ",$my_conf.date,JX9_EOL;
 *  print "Current time: ",$my_conf.time(); // Anonymous function
 *
 * That is, this simple program when running should display a greeting
 * message, the current system time and the host operating system.
 * A typical output of this program would look like this:
 *
 *	Hello world!
 *	Host Operating System:  Microsoft Windows 7 localhost 6.1 build 7600 x86
 *  Current date: 2012-12-27
 *  Current time: 10:36:02
 */
#define JX9_PROG \
    "$my_conf = {"\
    "/* Greeting message */"\
    "greeting : \"Hello world!\n\","\
	"/* Dummy field */"\
	"__id: 1,"\
	"/* Host Operating System */"\
	"os_name: uname(),"\
	"/* Current date */"\
	"date : __DATE__,"\
	"/* Return the current time using an annonymous function */"\
	"time : function(){ return __TIME__; }"\
    "};"\
    "/* invoke JSON object members now */"\
    "print $my_conf.greeting;"\
    "print \"Host Operating System: \",$my_conf.os_name,JX9_EOL;"\
    "print \"Current date: \",$my_conf.date,JX9_EOL;"\
    "print \"Current time: \",$my_conf.time(); /* Annynoymous function */"
/* Make sure you have the latest release of the JX9 engine
 * from:
 *  http://jx9.symisc.net/downloads.html
 */
#include <stdio.h>
#include <stdlib.h>
/* Make sure this header file is available.*/
#include "jx9.h"
/*
 * Display an error message and exit.
 */
static void Fatal(const char *zMsg)
{
	puts(zMsg);
	/* Shutdown the library */
	jx9_lib_shutdown();
	/* Exit immediately */
	exit(0);
}
/*
 * VM output consumer callback.
 * Each time the virtual machine generates some outputs, the following
 * function gets called by the underlying virtual machine to consume
 * the generated output.
 * All this function does is redirecting the VM output to STDOUT.
 * This function is registered later via a call to jx9_vm_config()
 * with a configuration verb set to: JX9_VM_CONFIG_OUTPUT.
 */
static int Output_Consumer(const void *pOutput, unsigned int nOutputLen, void *pUserData /* Unused */)
{
	/*
	 * Note that it's preferable to use the write() system call to display the output
	 * rather than using the libc printf() which everybody now is extremely slow.
	 */
	printf("%.*s",
		nOutputLen,
		(const char *)pOutput /* Not null terminated */
		);
	/* All done, VM output was redirected to STDOUT */
	return JX9_OK;
}
/*
 * Main program: Compile and execute the JX9 program defined above.
 */
int main(void)
{
	jx9 *pEngine; /* JX9 engine handle*/
	jx9_vm *pVm;  /* Compiled JX9 program */
	int rc;

	/* Allocate a new jx9 engine instance */
	rc = jx9_init(&pEngine);
	if( rc != JX9_OK ){
		/*
		 * If the supplied memory subsystem is so sick that we are unable
		 * to allocate a tiny chunk of memory, there is no much we can do here.
		 */
		Fatal("Error while allocating a new JX9 engine instance");
	}


	/* Compile the test program defined above */
	rc = jx9_compile(pEngine,JX9_PROG,sizeof(JX9_PROG)-1,&pVm);
	if( rc != JX9_OK ){
		if( rc == JX9_COMPILE_ERR ){
			const char *zErrLog;
			int nLen;
			/* Extract error log */
			jx9_config(pEngine,
				JX9_CONFIG_ERR_LOG,
				&zErrLog,
				&nLen
				);
			if( nLen > 0 ){
				/* zErrLog is null terminated */
				puts(zErrLog);
			}
		}
		/* Exit */
		Fatal("Compile error");
	}


	/*
	 * Now we have our script compiled, it's time to configure our VM.
	 * We will install the VM output consumer callback defined above
	 * so that we can consume the VM output and redirect it to STDOUT.
	 */
	rc = jx9_vm_config(pVm,
		JX9_VM_CONFIG_OUTPUT,
		Output_Consumer,    /* Output Consumer callback */
		0                   /* Callback private data */
		);
	if( rc != JX9_OK ){
		Fatal("Error while installing the VM output consumer callback");
	}


	/*
	 * And finally, execute our program. Note that your output (STDOUT in our case)
	 * should display the result.
	 */
	jx9_vm_exec(pVm, 0);


	/* All done, cleanup the mess left behind.
	*/
	jx9_vm_release(pVm);
	jx9_release(pEngine);

	return 0;
}