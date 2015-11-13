/**************************************************************************************************
	Author: 				Tom Vizel
	Creation date: 			10.11.15
	Last modified date: 	10.11.15
	
	Description: Unit test module for operator.
**************************************************************************************************/

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "ADTErr.h"
#include "cdr.h"
#include "Operator.h"

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

static void CreateNULL(void)
{
	ADTErr err;
	Operator* op = OperatorCreate(NULL, &err);
	PRINT_STATEMENT( (ERR_NOT_INITIALIZED == err) && (NULL == op) );
}

static void CreateOK(void)
{
	ADTErr err;
	CDR* cdr1 = CDR1Init();
	Operator* operator = NULL;
	
	operator = OperatorCreate(cdr1, &err);
	/*OperatorPrint(operator);*/
	PRINT_STATEMENT( (NULL != operator) && (ERR_OK == err) );
	OperatorDestroy(operator);
}

static void UpdateNULL(void)
{
	PRINT_STATEMENT( ERR_NOT_INITIALIZED == OperatorUpdate(NULL, NULL) );	
}

static void UpdateOK(void)
{
	ADTErr err;
	CDR* cdr1 = CDR1Init();
	Operator* op1 = NULL;
	CDR* cdr2 = CDR2Init();
	Operator* op2 = NULL;
	
	op1 = OperatorCreate(cdr1, NULL);
	op2 = OperatorCreate(cdr2, NULL);
	err = OperatorUpdate(op1, op2);
	/*OperatorPrint(op1);*/
	PRINT_STATEMENT( ERR_OK == err );
	OperatorDestroy(op1);
	OperatorDestroy(op2);
}

static void UpdateIllegal(void)
{
	ADTErr err;
	CDR* cdr1 = CDR1Init();
	Operator* op1 = NULL;
	CDR* cdr3 = CDR3Init();
	Operator* op3 = NULL;
	
	op1 = OperatorCreate(cdr1, NULL);
	op3 = OperatorCreate(cdr3, NULL);
	err = OperatorUpdate(op1, op3);
	PRINT_STATEMENT( ERR_ILLEGAL_INPUT == err );
	OperatorDestroy(op1);
	OperatorDestroy(op3);
}

static void IsSameDifferent(void)
{
	CDR* cdr1 = CDR1Init();
	Operator* op1 = NULL;
	CDR* cdr3 = CDR3Init();
	Operator* op3 = NULL;
	
	op1 = OperatorCreate(cdr1, NULL);
	op3 = OperatorCreate(cdr3, NULL);
	PRINT_STATEMENT( ! OperatorIsSame(op1, op3) );
	OperatorDestroy(op1);
	OperatorDestroy(op3);
}

static void IsSameSame(void)
{
	CDR* cdr1 = CDR1Init();
	Operator* op1 = NULL;
	CDR* cdr2 = CDR2Init();
	Operator* op2 = NULL;
	
	op1 = OperatorCreate(cdr1, NULL);
	op2 = OperatorCreate(cdr2, NULL);
	PRINT_STATEMENT( OperatorIsSame(op1, op2) );
	OperatorDestroy(op1);
	OperatorDestroy(op2);
}

static void GetNameNull(void)
{
	PRINT_STATEMENT( ERR_NOT_INITIALIZED == OperatorGetName(NULL, NULL) );
}

static void GetNameOK(void)
{
	ADTErr err;
	CDR* cdr1 = CDR1Init();
	Operator* op1 = NULL;
	char buf[20];
	
	op1 = OperatorCreate(cdr1, NULL);
	err = OperatorGetName(op1, buf);
	PRINT_STATEMENT( (ERR_OK == err) && ( ! strcmp(buf, "Cellcom") ) );
	OperatorDestroy(op1);
}

int main()
{
	CreateNULL();
	CreateOK();
	
	UpdateNULL();
	UpdateOK();
	UpdateIllegal();
	
	IsSameDifferent();
	IsSameSame();
	
	GetNameNull();
	GetNameOK();
	
	/* PrintToFile tested in testOpDB */
	
	return 0;
}
