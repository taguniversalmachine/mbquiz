/*
 * Compile this file together with the jx9 engine source code to generate
 * the executable. For example:
 *  gcc -W -Wall -O6 -o jx9_test jx9_func_intro.c jx9.c
 */
/*
 * This simple program is a quick introduction to the foreign functions and their related
 * interfaces.
 * For an introductory course to the [jx9_create_function()] interface and the foreign function
 * mechanism in general, please refer to this page
 *        http://jx9.symisc.net/func_intro.html
 * For an introduction to the JX9 C/C++ interface, please refer to this page
 *        http://jx9.symisc.net/api_intro.html
 * For the full C/C++ API reference guide, please refer to this page
 *        http://jx9.symisc.net/c_api.html
 * For the full JX9 language reference manual, please refer to this page
 *        http://jx9.symisc.net/jx9_lang.html
 */
/* $SymiscID: jx9_func_intro.c v2.1 Win7 2012-09-06 23:30 stable <chm@symisc.net> $ */
/*
 * Make sure you have the latest release of the JX9 engine
 * from:
 *  http://jx9.symisc.net/downloads.html
 * Make sure this header file is available.
 */
#include "jx9.h"
/*
 * int shift_func(int $num)
 *  Right shift a number by one and return the result.
 * Description
 *  Our first function perform a simple right shift operation on a given number
 *  and return that number shifted by one.
 *  This function expects a single parameter which must be numeric (either integer or float
 *  or a string that looks like a number).
 * Parameter
 *  $num
 *   Number to shift by one.
 * Return value
 *   Integer: Given number shifted by one.
 * Usage example:
 *   print shift_func(150); //Output 300
 *   print shift_func(50);  //Output 100
 */
int shift_func(
	jx9_context *pCtx, /* Call Context */
	int argc,          /* Total number of arguments passed to the function */
	jx9_value **argv   /* Array of function arguments */
	)
{
	int num;
	/* Make sure there is at least one argument and is of the
	 * expected type [i.e: numeric].
	 */
	if( argc < 1 || !jx9_value_is_numeric(argv[0]) ){
		/*
		 * Missing/Invalid argument, throw a warning and return FALSE.
		 * Note that you do not need to log the function name, JX9 will
		 * automatically append the function name for you.
		 */
		jx9_context_throw_error(pCtx, JX9_CTX_WARNING, "Missing numeric argument");
		/* Return false */
		jx9_result_bool(pCtx, 0);
		return JX9_OK;
	}
	/* Extract the number */
	num = jx9_value_to_int(argv[0]);
	/* Shift by 1 */
	num <<= 1;
	/* Return the new value */
	jx9_result_int(pCtx, num);
	/* All done */
	return JX9_OK;
}
#include <time.h>
/*
 * string date_func(void)
 *  Return the current system date.
 * Description
 *  Our second function does not expects arguments and return the
 *  current system date.
 * Parameter
 *  None
 * Return value
 *   String: Current system date.
 * Usage example
 *   print date_func(); //would output: 2012-23-09 14:53:30
 */
int date_func(
	jx9_context *pCtx, /* Call Context */
	int argc,          /* Total number of arguments passed to the function */
	jx9_value **argv   /* Array of function arguments*/
	){
		time_t tt;
		struct tm *pNow;
		/* Get the current time */
		time(&tt);
		pNow = localtime(&tt);
		/*
		 * Return the current date.
		 */
		jx9_result_string_format(pCtx,
			"%04d-%02d-%02d %02d:%02d:%02d", /* printf() style format */
			pNow->tm_year + 1900, /* Year */
			pNow->tm_mday,        /* Day of the month */
			pNow->tm_mon + 1,     /* Month number */
			pNow->tm_hour, /* Hour */
			pNow->tm_min,  /* Minutes */
			pNow->tm_sec   /* Seconds */
			);
		/* All done */
		return JX9_OK;
}
/*
 * int64 sum_func(int $arg1, int $arg2, int $arg3, ...)
 *  Return the sum of the given arguments.
 * Description
 *  This function expects a variable number of arguments which must be of type
 *  numeric (either integer or float or a string that looks like a number) and
 *  returns the sum of the given numbers.
 * Parameter
 *   int $n1, n2, ... (Variable number of arguments)
 * Return value
 *   Integer64: Sum of the given numbers.
 * Usage example
 *   print sum_func(7, 8, 9, 10); //would output 34
 */
int sum_func(jx9_context *pCtx, int argc, jx9_value **argv)
{
	jx9_int64 iTotal = 0; /* Counter */
	int i;
	if( argc < 1 ){
		/*
		 * Missing arguments, throw a notice and return NULL.
		 * Note that you do not need to log the function name, JX9 will
		 * automatically append the function name for you.
		 */
		jx9_context_throw_error(pCtx, JX9_CTX_NOTICE, "Missing function arguments $arg1, $arg2, ..");
		/* Return null */
		jx9_result_null(pCtx);
		return JX9_OK;
	}
	/* Sum the arguments */
	for( i = 0; i < argc ; i++ ){
		jx9_value *pVal = argv[i];
		jx9_int64 n;
		/* Make sure we are dealing with a numeric argument */
		if( !jx9_value_is_numeric(pVal) ){
			/* Throw a notice and continue */
			jx9_context_throw_error_format(pCtx, JX9_CTX_NOTICE,
				"Arg[%d]: Expecting a numeric value", /* printf() style format */
				i
				);
			/* Ignore */
			continue;
		}
		/* Get a 64-bit integer representation and increment the counter */
		n = jx9_value_to_int64(pVal);
		iTotal += n;
	}
	/* Return the count  */
	jx9_result_int64(pCtx, iTotal);
	/* All done */
	return JX9_OK;
}
/*
 * array array_time_func(void)
 *  Return the current system time in a JSON array.
 * Description
 *  This function does not expects arguments and return the
 *  current system time in an array.
 * Parameter
 *  None
 * Return value
 *   Array holding the current system time.
 * Usage example
 *
 *   print array_time_func() ;
 *
 * When running you should see something like that:
 * JSON array(3) [14,53,30]
 */
int array_time_func(jx9_context *pCtx, int argc, jx9_value **argv)
{
	jx9_value *pArray;    /* Our JSON Array */
	jx9_value *pValue;    /* Array entries value */
	time_t tt;
	struct tm *pNow;
	/* Get the current time first */
	time(&tt);
	pNow = localtime(&tt);
	/* Create a new array */
	pArray = jx9_context_new_array(pCtx);
	/* Create a worker scalar value */
	pValue = jx9_context_new_scalar(pCtx);
	if( pArray == 0 || pValue == 0 ){
		/*
		 * If the supplied memory subsystem is so sick that we are unable
		 * to allocate a tiny chunk of memory, there is no much we can do here.
		 * Abort immediately.
		 */
		jx9_context_throw_error(pCtx, JX9_CTX_ERR, "Fatal, JX9 is running out of memory");
		/* emulate the die() construct */
		return JX9_ABORT; /* die('Fatal, JX9 is running out of memory'); */
	}
	/* Populate the array.
	 * Note that we will use the same worker scalar value (pValue) here rather than
	 * allocating a new value for each array entry. This is due to the fact
	 * that the populated array will make it's own private copy of the inserted
	 * key(if available) and it's associated value.
	 */

	jx9_value_int(pValue, pNow->tm_hour); /* Hour */
	/* Insert the hour at the first available index */
	jx9_array_add_elem(pArray, 0/* NULL: Assign an automatic index*/, pValue /* Will make it's own copy */);

	/* Overwrite the previous value */
	jx9_value_int(pValue, pNow->tm_min); /* Minutes */
	/* Insert minutes */
	jx9_array_add_elem(pArray, 0/* NULL: Assign an automatic index*/, pValue /* Will make it's own copy */);

	/* Overwrite the previous value */
	jx9_value_int(pValue, pNow->tm_sec); /* Seconds */
	/* Insert seconds */
	jx9_array_add_elem(pArray, 0/* NULL: Assign an automatic index*/, pValue /* Will make it's own copy */);

	/* Return the array as the function return value */
	jx9_result_value(pCtx, pArray);

	/* All done. Don't worry about freeing memory here, every
	 * allocated resource will be released automatically by the engine
	 * as soon we return from this foreign function.
	 */
	return JX9_OK;
}
/*
 * object object_date_func(void)
 *  Return a copy of the 'struct tm' structure in a JSON array.
 * Description
 *  This function does not expects arguments and return a copy of
 *  the 'struct tm' structure found in the 'time.h' header file.
 *  This structure hold the current system date&time.
 * Parameter
 *  None
 * Return value
 *   Associative array holding a copy of the 'struct tm' structure.
 * Usage example
 *
 *   print object_date_func();
 *
 * When running you should see something like that:
 * JSON Object(6 {
 *  "tm_year":2012,
 *  "tm_mon":12,
 *  "tm_mday":29,
 *  "tm_hour":1,
 *  "tm_min":13,
 *  "tm_sec":58
 *  }
 * )
 */
int object_date_func(jx9_context *pCtx, int argc /* unused */, jx9_value **argv /* unused */)
{
	jx9_value *pObject;    /* Our JSON object */
	jx9_value *pValue;    /* Objecr entries value */
	time_t tt;
	struct tm *pNow;
	/* Get the current time first */
	time(&tt);
	pNow = localtime(&tt);
	/* Create a new JSON object */
	pObject = jx9_context_new_array(pCtx);
	/* Create a worker scalar value */
	pValue = jx9_context_new_scalar(pCtx);
	if( pObject == 0 || pValue == 0 ){
		/*
		 * If the supplied memory subsystem is so sick that we are unable
		 * to allocate a tiny chunk of memory, there is no much we can do here.
		 * Abort immediately.
		 */
		jx9_context_throw_error(pCtx, JX9_CTX_ERR, "Fatal, JX9 is running out of memory");
		/* emulate the die() construct */
		return JX9_ABORT; /* die('Fatal, JX9 is running out of memory'); */
	}
	/* Populate the array.
	 * Note that we will use the same worker scalar value (pValue) here rather than
	 * allocating a new value for each array entry. This is due to the fact
	 * that the populated array will make it's own private copy of the inserted
	 * key(if available) and it's associated value.
	 */

	jx9_value_int(pValue, pNow->tm_year + 1900); /* Year */
	/* Insert Year */
	jx9_array_add_strkey_elem(pObject, "tm_year", pValue /* Will make it's own copy */);

	/* Overwrite the previous value */
	jx9_value_int(pValue, pNow->tm_mon + 1); /* Month [1-12]*/
	/* Insert month number */
	jx9_array_add_strkey_elem(pObject, "tm_mon", pValue /* Will make it's own copy */);

	/* Overwrite the previous value */
	jx9_value_int(pValue, pNow->tm_mday); /* Day of the month [1-31] */
	/* Insert the day of the month */
	jx9_array_add_strkey_elem(pObject, "tm_mday", pValue /* Will make it's own copy */);

	jx9_value_int(pValue, pNow->tm_hour); /* Hour */
	/* Insert the hour */
	jx9_array_add_strkey_elem(pObject, "tm_hour", pValue /* Will make it's own copy */);

	/* Overwrite the previous value */
	jx9_value_int(pValue, pNow->tm_min); /* Minutes */
	/* Insert minutes */
	jx9_array_add_strkey_elem(pObject, "tm_min", pValue /* Will make it's own copy */);

	/* Overwrite the previous value */
	jx9_value_int(pValue, pNow->tm_sec); /* Seconds */
	/* Insert seconds */
	jx9_array_add_strkey_elem(pObject, "tm_sec", pValue /* Will make it's own copy */);

	/* Return the JSON object as the function return value */
	jx9_result_value(pCtx, pObject);
	/* All done. Don't worry about freeing memory here, every
	 * allocated resource will be released automatically by the engine
	 * as soon we return from this foreign function.
	 */
	return JX9_OK;
}
/*
 * array array_string_split(string $str)
 *  Return a copy of each string character in an array.
 * Description
 *  This function splits a given string to its
 *  characters and return the result in an array.
 * Parameter
 *  $str
 *     Target string to split.
 * Return value
 *   Array holding string characters.
 * Usage example
 *
 *   print array_str_split('Hello');
 *
 * When running you should see something like that:
 *   JSON Array(5 ["H","e","l","l","o"])
 */
int array_string_split_func(jx9_context *pCtx, int argc, jx9_value **argv)
{
	jx9_value *pArray;    /* Our JSON Array */
	jx9_value *pValue;    /* Array entries value */
	const char *zString, *zEnd;  /* String to split */
	int nByte;            /* String length */
	/* Make sure there is at least one argument and is of the
	 * expected type [i.e: string].
	 */
	if( argc < 1 || !jx9_value_is_string(argv[0]) ){
		/*
		 * Missing/Invalid argument, throw a warning and return FALSE.
		 * Note that you do not need to log the function name, JX9 will
		 * automatically append the function name for you.
		 */
		jx9_context_throw_error(pCtx, JX9_CTX_WARNING, "Missing string to split");
		/* Return false */
		jx9_result_bool(pCtx, 0);
		return JX9_OK;
	}
	/* Extract the target string.
	 * Note that zString is null terminated and jx9_value_to_string() never
	 * fail and always return a pointer to a null terminated string.
	 */
	zString = jx9_value_to_string(argv[0], &nByte /* String length */);
	if( nByte < 1 /* Empty string [i.e: '' or ""] */ ){
		jx9_context_throw_error(pCtx, JX9_CTX_NOTICE, "Empty string");
		/* Return false */
		jx9_result_bool(pCtx, 0);
		return JX9_OK;
	}
	/* Create our array */
	pArray = jx9_context_new_array(pCtx);
	/* Create a scalar worker value */
	pValue = jx9_context_new_scalar(pCtx);
	/* Split the target string */
	zEnd = &zString[nByte]; /* Delimit the string */
	while( zString < zEnd ){
		int c = zString[0];
		/* Prepare the character for insertion */
		jx9_value_string(pValue, (const char *)&c, (int)sizeof(char));
		/* Insert the character */
		jx9_array_add_elem(pArray, 0/* NULL: Assign an automatic index */, pValue /* Will make it's own copy*/);
		/* Erase the previous data from the worker variable */
		jx9_value_reset_string_cursor(pValue);
		/* Next character */
		zString++;
	}
	/* Return our array as the function return value */
	jx9_result_value(pCtx, pArray);
	/* All done. Don't worry about freeing memory here, every
	 * allocated resource will be released automatically by the engine
	 * as soon we return from this foreign function.
	 */
	return JX9_OK;
}
/*
 * Container for the foreign functions defined above.
 * These functions will be registered later using a call
 * to [jx9_create_function()].
 */
static const struct foreign_func {
	const char *zName; /* Name of the foreign function*/
	int (*xProc)(jx9_context *, int, jx9_value **); /* Pointer to the C function performing the computation*/
}aFunc[] = {
	{"shift_func", shift_func},
	{"date_func", date_func},
	{"sum_func",  sum_func  },
	{"array_time_func", array_time_func},
	{"array_str_split", array_string_split_func},
	{"object_date_func", object_date_func}
};
/*
 * Test our implementation:
 *
 * The following is the JX9 program to execute.
 *
 *  print 'shift_func(150) = ' .. shift_func(150) .. JX9_EOL;
 *  print 'sum_func(7,8,9,10) = ' .. sum_func(7,8,9,10) .. JX9_EOL;
 *  print 'date_func(5) = ' .. date_func() .. JX9_EOL;
 *  print 'array_time_func() =' .. array_time_func() .. JX9_EOL;
 *  print 'object_date_func() =' ..  JX9_EOL;
 *  dump(object_date_func());
 *  print 'array_str_split('Hello') ='  .. JX9_EOL;
 *  dump(array_str_split('Hello'))
 *
 * When running, you should see something like that:
 *
 * shift_func(150) = 300
 * sum_func(7,8,9,10) = 34
 * date_func(5) = 2012-29-12 01:13:58
 * array_time_func() = [1,13,58]
 * object_date_func() =
 * JSON Object(6 {
 *  "tm_year":2012,
 *  "tm_mon":12,
 *  "tm_mday":29,
 *  "tm_hour":1,
 *  "tm_min":13,
 *  "tm_sec":58
 *  }
 * )
 * array_str_split('Hello') =
 * JSON Array(5 ["H","e","l","l","o"])
 *
 */
#define JX9_PROG \
	 "print 'shift_func(150) = ' .. shift_func(150) .. JX9_EOL;"\
     "print 'sum_func(7,8,9,10) = ' .. sum_func(7,8,9,10) .. JX9_EOL;"\
     "print 'date_func(5) = ' .. date_func() .. JX9_EOL;"\
	 "print 'array_time_func() =' .. array_time_func() .. JX9_EOL;"\
	 "print 'object_date_func() =' ..JX9_EOL;"\
	 "dump(object_date_func());"\
	 "print 'array_str_split(\\'Hello\\') =' .. JX9_EOL;"\
	 "dump(array_str_split('Hello'));"

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
 * Main program: Register the foreign functions defined above, compile and execute
 * our JX9 test program.
 */
int main(void)
{
	jx9 *pEngine; /* JX9 engine */
	jx9_vm *pVm;  /* Compiled JX9 program */
	int rc;
	int i;

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

	/* Now we have our program compiled, it's time to register
	 * our foreign functions.
	 */
	for( i = 0 ; i < (int)sizeof(aFunc)/sizeof(aFunc[0]) ;  ++i ){
		/* Install the foreign function */
		rc = jx9_create_function(pVm, aFunc[i].zName, aFunc[i].xProc, 0 /* NULL: No private data */);
		if( rc != JX9_OK ){
			Fatal("Error while registering foreign functions");
		}
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
	 * Report run-time errors such as unexpected numbers of arguments and so on.
	 */
	jx9_vm_config(pVm, JX9_VM_CONFIG_ERR_REPORT);

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