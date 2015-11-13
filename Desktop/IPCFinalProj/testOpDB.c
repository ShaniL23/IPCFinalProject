/**************************************************************************************************
	Author: 				Tom Vizel
	Creation date: 			10.11.15
	Last modified date: 	10.11.15
	
	Description: Unit test module for Operator database.
**************************************************************************************************/

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#include "ADTErr.h"
#include "cdr.h"
#include "Operator.h"
#include "OperatorDB.h"

#define PRINT_STATEMENT(_statement) PrintStatement(_statement, __FUNCTION__)

static void PrintStatement(int _statement, const char* _funcName)
{
	printf("%s ", _funcName);
	printf(_statement ? "PASS!\n" : "FAIL!\n");
}

CDR* CDR1Init(void)
{
	CDR* cdr1 = NULL;
	
	cdr1 = CDRCreate(NULL);
	CDRInsertIMSI(cdr1, "111111111");
	CDRInsertOpCode(cdr1, "Cellcom");
	CDRInsertCallType(cdr1, MOC);
	CDRInsertCallDuration(cdr1, 300);
	
	return cdr1;
}

CDR* CDR2Init(void)
{
	CDR* cdr2 = NULL;
	
	cdr2 = CDRCreate(NULL);
	CDRInsertIMSI(cdr2, "111111111");
	CDRInsertOpCode(cdr2, "Cellcom");
	CDRInsertCallType(cdr2, SMS_MO);
	
	return cdr2;
}

CDR* CDR3Init(void)
{
	CDR* cdr3 = NULL;
	
	cdr3 = CDRCreate(NULL);
	CDRInsertIMSI(cdr3, "111111112");
	CDRInsertOpCode(cdr3, "Orange");
	CDRInsertCallType(cdr3, GPRS);
	CDRInsertDownloadedMB(cdr3, 1.3);
	CDRInsertUploadedMB(cdr3, 0.42);
	
	return cdr3;
}

static void CreateOK(void)
{
	ADTErr err;
	OperatorDB* odb = OperatorDBCreate(&err);
	
	PRINT_STATEMENT( (NULL != odb) && (ERR_OK == err) );
	OperatorDBDestroy(odb);
}

static void InsertNULL(void)
{
	OperatorDB* odb = OperatorDBCreate(NULL);
	
	PRINT_STATEMENT( ERR_ILLEGAL_INPUT == OperatorDBInsert(odb, NULL) );
	OperatorDBDestroy(odb);
}

static void InsertOK(void)
{
	CDR* cdr1 = CDR1Init();
	Operator* op1 = OperatorCreate(cdr1, NULL);
	OperatorDB* odb = OperatorDBCreate(NULL);
	
	PRINT_STATEMENT( ERR_OK == OperatorDBInsert(odb, op1) );
	/*OperatorDBPrint(odb);*/
	OperatorDBDestroy(odb);
	CDRDestroy(cdr1);
}

static void InsertAlreadyExists(void)
{
	CDR* cdr1 = CDR1Init();
	CDR* cdr2 = CDR2Init();
	Operator* op1 = OperatorCreate(cdr1, NULL);
	Operator* op2 = OperatorCreate(cdr2, NULL);
	OperatorDB* odb = OperatorDBCreate(NULL);
	
	OperatorDBInsert(odb, op1);
	PRINT_STATEMENT( ERR_ALREADY_EXISTS == OperatorDBInsert(odb, op2) );
	/*OperatorDBPrint(odb);*/
	OperatorDBDestroy(odb);
	/*OperatorDestroy(op1);
	OperatorDestroy(op2);*/
	CDRDestroy(cdr1);
	CDRDestroy(cdr2);
}

static void InsertDifferent(void)
{
	CDR* cdr1 = CDR1Init();
	CDR* cdr3 = CDR3Init();
	Operator* op1 = OperatorCreate(cdr1, NULL);
	Operator* op3 = OperatorCreate(cdr3, NULL);
	OperatorDB* odb = OperatorDBCreate(NULL);
	
	OperatorDBInsert(odb, op1);
	PRINT_STATEMENT( ERR_OK == OperatorDBInsert(odb, op3) );
	/*OperatorDBPrint(odb);*/
	OperatorDBDestroy(odb);
	/*OperatorDestroy(op1);
	OperatorDestroy(op3);*/
	CDRDestroy(cdr1);
	CDRDestroy(cdr3);
}

static void GetOK(void)
{
	ADTErr err;
	CDR* cdr1 = CDR1Init();
	Operator* op1 = OperatorCreate(cdr1, NULL);
	OperatorDB* odb = OperatorDBCreate(NULL);
	Operator* get = NULL;
	
	OperatorDBInsert(odb, op1);
	err = OperatorDBGet(odb, "Cellcom", &get);
	/*OperatorPrint(get);*/
	PRINT_STATEMENT( (ERR_OK == err) && OperatorIsSame(op1, get) );
	OperatorDBDestroy(odb);
	CDRDestroy(cdr1);
}

static void RemoveOK(void)
{
	ADTErr err;
	CDR* cdr1 = CDR1Init();
	CDR* cdr3 = CDR3Init();
	Operator* op1 = OperatorCreate(cdr1, NULL);
	Operator* op3 = OperatorCreate(cdr3, NULL);
	OperatorDB* odb = OperatorDBCreate(NULL);
	Operator* get = NULL;
	
	OperatorDBInsert(odb, op1);
	OperatorDBInsert(odb, op3);
	err= OperatorDBRemove(odb, "Cellcom", &get);
	/*OperatorPrint(get);*/
	/*OperatorDBPrint(odb);*/
	PRINT_STATEMENT( (ERR_OK == err) && OperatorIsSame(op1, get) );
	OperatorDBDestroy(odb);
	CDRDestroy(cdr1);
	CDRDestroy(cdr3);
}

static void PrintToFileOK(void)
{
	CDR* cdr1 = CDR1Init();
	CDR* cdr3 = CDR3Init();
	Operator* op1 = OperatorCreate(cdr1, NULL);
	Operator* op3 = OperatorCreate(cdr3, NULL);
	OperatorDB* odb = OperatorDBCreate(NULL);
	
	OperatorDBInsert(odb, op1);
	OperatorDBInsert(odb, op3);
	PRINT_STATEMENT( ERR_OK == OperatorDBPrintToFile(odb, "Test.txt") );
	OperatorDBDestroy(odb);
	CDRDestroy(cdr1);
	CDRDestroy(cdr3);
}

static void GetNotInitialized(void)
{
	PRINT_STATEMENT( ERR_NOT_INITIALIZED == OperatorDBGet(NULL, NULL, NULL) );
}

static void GetIllegalInput(void)
{
	ADTErr err1;
	ADTErr err2;
	char key[10] = "111";
	Operator* op = NULL;
	OperatorDB* odb = OperatorDBCreate(NULL);
	
	err1 = OperatorDBGet(odb, key, NULL);
	err2 = OperatorDBGet(odb, NULL, &op);
	PRINT_STATEMENT( (ERR_ILLEGAL_INPUT == err1) && (ERR_ILLEGAL_INPUT == err2) );
	OperatorDBDestroy(odb);
}

static void RemoveNotInitialized(void)
{
	PRINT_STATEMENT( ERR_NOT_INITIALIZED == OperatorDBRemove(NULL, NULL, NULL) );
}

static void RemoveIllegalInput(void)
{
	ADTErr err1;
	ADTErr err2;
	char key[10] = "111";
	Operator* op = NULL;
	OperatorDB* odb = OperatorDBCreate(NULL);
	
	err1 = OperatorDBRemove(odb, key, NULL);
	err2 = OperatorDBRemove(odb, NULL, &op);
	PRINT_STATEMENT( (ERR_ILLEGAL_INPUT == err1) && (ERR_ILLEGAL_INPUT == err2) );
	OperatorDBDestroy(odb);
}

static void GetNotFound(void)
{
	CDR* cdr1 = CDR1Init();
	Operator* op1 = OperatorCreate(cdr1, NULL);
	OperatorDB* odb = OperatorDBCreate(NULL);
	Operator* get = NULL;
	
	OperatorDBInsert(odb, op1);
	PRINT_STATEMENT( ERR_NOT_FOUND == OperatorDBGet(odb, "Orange", &get) );
	OperatorDBDestroy(odb);
	CDRDestroy(cdr1);
}

static void RemoveNotFound(void)
{
	CDR* cdr1 = CDR1Init();
	Operator* op1 = OperatorCreate(cdr1, NULL);
	OperatorDB* odb = OperatorDBCreate(NULL);
	Operator* get = NULL;
	
	OperatorDBInsert(odb, op1);
	PRINT_STATEMENT( ERR_NOT_FOUND == OperatorDBRemove(odb, "Orange", &get) );
	OperatorDBDestroy(odb);
	CDRDestroy(cdr1);
}

int main()
{
	CreateOK();
	
	InsertNULL();
	InsertOK();
	InsertAlreadyExists();
	InsertDifferent();
	
	GetNotInitialized();
	GetIllegalInput();
	GetOK();
	GetNotFound();
	
	RemoveNotInitialized();
	RemoveIllegalInput();
	RemoveOK();
	RemoveNotFound();
	
	PrintToFileOK();
	
	return 0;
}
