/*
 * Compile this file together with the jx9 engine source code to generate
 * the executable. For example:
 *  gcc -W -Wall -O6 -o jx9_test jx9_const_intro.c jx9.c
 */
/*
 * For an introductory course to the constant expansion meachanism, you can refer
 * to the following tutorial:
 *        http://jx9.symisc.net/const_intro.html
 * For an introduction to the JX9 C/C++ interface, please refer to this page
 *        http://jx9.symisc.net/api_intro.html
 * For the full C/C++ API reference guide, please refer to this page
 *        http://jx9.symisc.net/c_api.html
 * For the full JX9 language reference manual, please refer to this page
 *        http://jx9.symisc.net/jx9_lang.html
 *
 * This simple program is a quick introduction to the constant expansion
 * mechanism introduced by the JX9 scripting engine.
 * The constant expansion mechanism under JX9 is extremely powerful yet
 * simple and work as follows:
 * Each registered constant have a C procedure associated with it.
 * This procedure known as the constant expansion callback is responsible of expanding
 * the invoked constant to the desired value, for example:
 * The C procedure associated with the "__PI__" constant expands to 3.14 (the value of PI).
 * the "__OS__" constant procedure expands to the name of the host Operating
 * Systems (Windows, Linux, ...), the "__TIME__" constant expands to the current system time
 * and so on.
 */
/* $SymiscID: jx9_const_intro.c v1.9 FreeBSD 2012-12-27 14:34 stable <chm@symisc.net> $ */
/*
 * Make sure you have the latest release of the JX9 engine
 * from:
 *  http://jx9.symisc.net/downloads.html
 * Make sure this header file is available.
 */
#include "jx9.h"
/*
 * __PI__: expand the value of PI (3.14...)
 * Our first constant is the __PI__ constant.The following procedure
 * is the callback associated with the __PI__ identifier. That is, when
 * the __PI__ identifier is seen in the running script, this procedure
 * gets called by the underlying JX9 virtual machine.
 * This procedure is responsible of expanding the constant identifier to
 * the desired value (3.14 in our case).
 */
static void PI_Constant(
	jx9_value *pValue, /* Store expanded value here */
	void *pUserData    /* User private data (unused in our case) */
	){
		/* Expand the value of PI */
		jx9_value_double(pValue, 3.1415926535898);
}
/*
 * __TIME__: expand the current local time.
 * Our second constant is the __TIME__ constant.
 * When the __TIME__ identifier is seen in the running script, this procedure
 * gets called by the underlying JX9 virtual machine.
 * This procedure is responsible of expanding the constant identifier to
 * the desired value (local time in our case).
 */
#include <time.h>
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
/*
 * __OS__: expand the name of the Host Operating System.
 * Our last constant is the __OS__ constant.
 * When the __OS__ identifier is seen in the running script, this procedure
 * gets called by the underlying JX9 virtual machine.
 * This procedure is responsible of expanding the constant identifier to
 * the desired value (OS name in our case).
 */
static void OS_Constant(jx9_value *pValue, void *pUserData /* Unused */ )
{
#ifdef __WINNT__
	jx9_value_string(pValue, "Windows", -1 /*Compute input length automatically */);
#else
	/* Assume UNIX */
	jx9_value_string(pValue, "UNIX", -1 /*Compute input length automatically */);
#endif /* __WINNT__ */
}
/*
 * Test now the constant expansion mechanism:
 * The following is the JX9 program to execute.
 *    print '__PI__   value: ' .. __PI__ .. JX9_EOL;
 *    print '__TIME__ value: ' .. __TIME__  .. JX9_EOL;
 *    print '__OS__   value: ' .. __OS__ .. JX9_EOL;
 * When running, you should see something like that:
 *	__PI__   value: 3.1415926535898
 *  __TIME__ value: 15:02:27
 *  __OS__   value: UNIX
 */
#define JX9_PROG \
 "print '__PI__   value: ' .. __PI__ ..   JX9_EOL;"\
 "print '__TIME__ value: ' .. __TIME__ .. JX9_EOL;"\
 "print '__OS__   value: ' .. __OS__ ..   JX9_EOL;"
#include <stdio.h>
#include <stdlib.h>
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
#ifdef __WINNT__
#include <Windows.h>
#else
/* Assume UNIX */
#include <unistd.h>
#endif
/*
 * The following define is used by the UNIX built and have
 * no particular meaning on windows.
 */
#ifndef STDOUT_FILENO
#define STDOUT_FILENO	1
#endif
/*
 * VM output consumer callback.
 * Each time the virtual machine generates some outputs, the following
 * function gets called by the underlying virtual machine to consume
 * the generated output.
 * All this function does is redirecting the VM output to STDOUT.
 * This function is registered later via a call to jx9_vm_config()
 * with a configuration verb set to: JX9_VM_CONFIG_OUTPUT.
 */
static int Output_Consumer(const void *pOutput, unsigned int nOutputLen, void *pUserData/* Unused */)
{
#ifdef __WINNT__
	BOOL rc;
	rc = WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), pOutput, (DWORD)nOutputLen, 0, 0);
	if( !rc ){
		/* Abort processing */
		return JX9_ABORT;
	}
#else
	ssize_t nWr;
	nWr = write(STDOUT_FILENO, pOutput, nOutputLen);
	if( nWr < 0 ){
		/* Abort processing */
		return JX9_ABORT;
	}
#endif /* __WINT__ */
	/* All done, VM output was redirected to STDOUT */
	return JX9_OK;
}
/*
 * Main program: Register the constants defined above, compile and execute
 * our JX9 test program.
 */
int main(void)
{
	jx9 *pEngine; /* JX9 engine */
	jx9_vm *pVm;  /* Compiled JX9 program */
	int rc;


	/* Allocate a new JX9 engine instance */
	rc = jx9_init(&pEngine);
	if( rc != JX9_OK ){
		/*
		 * If the supplied memory subsystem is so sick that we are unable
		 * to allocate a tiny chunk of memory, there is no much we can do here.
		 */
		Fatal("Error while allocating a new JX9 engine instance");
	}


	/* Compile the JX9 test program defined above */
	rc = jx9_compile(
		pEngine,  /* JX9 engine */
		JX9_PROG, /* JX9 test program */
		-1        /* Compute input length automatically*/,
		&pVm     /* OUT: Compiled JX9 program */
		);


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


	/* Now we have our program compiled, it's time to register our constants
	 * and their associated C procedure.
	 */
	rc = jx9_create_constant(pVm, "__PI__", PI_Constant, 0);
	if( rc != JX9_OK ){
		Fatal("Error while installing the __PI__ constant");
	}

	rc = jx9_create_constant(pVm, "__TIME__", TIME_Constant, 0);
	if( rc != JX9_OK ){
		Fatal("Error while installing the __TIME__ constant");
	}

	rc = jx9_create_constant(pVm, "__OS__", OS_Constant, 0);
	if( rc != JX9_OK ){
		Fatal("Error while installing the __OS__ constant");
	}


	/*
	 * Configure our VM:
	 *  Install the VM output consumer callback defined above.
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