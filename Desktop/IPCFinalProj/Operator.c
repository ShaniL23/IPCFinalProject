/**************************************************************************************************
	Author: 				Tom Vizel
	Creation date: 			9.11.15
	Last modified date: 	9.11.15
	
	Description: Implementation module for Operator functions.
**************************************************************************************************/

#include <stdio.h> /* printf, snprintf*/
#include <stdlib.h> /* malloc, NULL */
#include <string.h> /* strcpy */
#include <unistd.h> /* write */

#include "ADTErr.h"
#include "logger.h"
#include "logger_pub.h"
#include "cdr.h"
#include "Operator.h"

#define STRING_SIZE 64
#define ERR_MSG_SIZE 128

struct Operator
{
	char 			m_operatorName[STRING_SIZE];
	unsigned int 	m_incomingDuration;
	unsigned int 	m_outgoingDuration;
	unsigned int 	m_messagesReceived;
	unsigned int 	m_messagesSent;
	double			m_downloaded;
	double 			m_uploaded;
};

Operator* OperatorCreate(CDR* _cdr, ADTErr* _err)
{
	Operator* operator = NULL;
	e_callType type;
	char errMsg[ERR_MSG_SIZE];
	
	if (NULL == _cdr)
	{
		if (NULL != _err)
		{
			*_err = ERR_NOT_INITIALIZED;
		}
		GetError(errMsg, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", errMsg);
		return NULL;
	}
	
	operator = (Operator*) calloc(1, sizeof(Operator));
	if (NULL == operator)
	{
		if (NULL != _err)
		{
			*_err = ERR_ALLOCATION_FAILED;
		}
		GetError(errMsg, ERR_ALLOCATION_FAILED);
		LOG_ERROR_PRINT("%s", errMsg);
		return NULL;
	}
	
	CDRGetOpCode(_cdr, operator->m_operatorName);
	CDRGetCallType(_cdr, &type);
	switch (type)
	{
		case MOC:
		{
			CDRGetCallDuration(_cdr, &operator->m_outgoingDuration);
			LOG_DEBUG_PRINT("Operator %s updated outgoing call duration by %u seconds.", operator->m_operatorName, operator->m_outgoingDuration);
			break;
		}
		case MTC:
		{
			CDRGetCallDuration(_cdr, &operator->m_incomingDuration);
			LOG_DEBUG_PRINT("Operator %s updated incoming call duration by %u seconds.", operator->m_operatorName, operator->m_incomingDuration);
			break;
		}
		case SMS_MO:
		{
			++operator->m_messagesSent;
			LOG_DEBUG_PRINT("Operator %s logged 1 sent message.", operator->m_operatorName);
			break;
		}
		case SMS_MT:
		{
			++operator->m_messagesReceived;
			LOG_DEBUG_PRINT("Operator %s logged 1 received message.", operator->m_operatorName);
			break;
		}
		case GPRS:
		{
			CDRGetDownloadedMB(_cdr, &operator->m_downloaded);
			CDRGetUploadedMB(_cdr, &operator->m_uploaded);
			LOG_DEBUG_PRINT("Operator %s: %g MB downloaded, %g MB uploaded.", operator->m_operatorName, operator->m_downloaded, operator->m_uploaded);
			break;
		}
		default:
		{
			free(operator);
			if (NULL != _err)
			{
				*_err = ERR_ILLEGAL_INPUT;
			}
			GetError(errMsg, ERR_ILLEGAL_INPUT);
			LOG_ERROR_PRINT("%s", errMsg);
			return NULL;
		}
	}
	
	if (NULL != _err)
	{
		*_err = ERR_OK;
	}
	LOG_DEBUG_PRINT("%s", "Successfully created operator!");
	return operator;
}

void OperatorDestroy(Operator* _operator)
{
	if (NULL == _operator)
	{
		LOG_WARN_PRINT("%s", "Attempted to destroy uninitialized operator.");
		return;
	}
	
	free(_operator);
	LOG_DEBUG_PRINT("%s", "Successfully destroyed operator.");
}

int	OperatorIsSame(const Operator* _op1, const Operator* _op2)
{
	if (NULL == _op1 || NULL == _op2)
	{
		return (_op1 == _op2);
	}
	
	LOG_DEBUG_PRINT("Compared 2 operators: %s and %s.", _op1->m_operatorName, _op2->m_operatorName);
	return ( ! strcmp(_op1->m_operatorName, _op2->m_operatorName) );
}

ADTErr OperatorUpdate(Operator* _destination, const Operator* _source)
{
	char errMsg[ERR_MSG_SIZE];
	
	if (NULL == _destination || NULL == _source)
	{
		GetError(errMsg, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_NOT_INITIALIZED;
	}
	
	if ( strcmp(_destination->m_operatorName, _source->m_operatorName) )
	{
		GetError(errMsg, ERR_ILLEGAL_INPUT);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_ILLEGAL_INPUT;
	}
	
	_destination->m_incomingDuration += _source->m_incomingDuration;
	_destination->m_outgoingDuration += _source->m_outgoingDuration;
	_destination->m_messagesReceived += _source->m_messagesReceived;
	_destination->m_messagesSent += _source->m_messagesSent;
	_destination->m_downloaded += _source->m_downloaded;
	_destination->m_uploaded += _source->m_uploaded;
	
	LOG_DEBUG_PRINT("Successfully updated %s.", _destination->m_operatorName);
	return ERR_OK;
}

ADTErr OperatorGetName(const Operator* _operator, char* _operatorName)
{
	char errMsg[ERR_MSG_SIZE];
	
	if (NULL == _operator)
	{
		GetError(errMsg, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_NOT_INITIALIZED;
	}
	if (NULL == _operatorName)
	{
		GetError(errMsg, ERR_ILLEGAL_INPUT);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_ILLEGAL_INPUT;
	}
	
	strcpy(_operatorName, _operator->m_operatorName);
	LOG_DEBUG_PRINT("Retrieved operator name: %s", _operator->m_operatorName);
	return ERR_OK;
}

ADTErr OperatorPrintToFile(const Operator* _operator, const int _fileDescriptor)
{
	int nBytes;
	char buf[STRING_SIZE];
	char errMsg[ERR_MSG_SIZE];
	
	if (_fileDescriptor < 0)
	{
		LOG_ERROR_PRINT("%s", "Invalid file descriptor for output!");
		return ERR_ILLEGAL_INPUT;
	}
	
	if (NULL == _operator)
	{
		LOG_WARN_PRINT("%s", "Operator data unavailable!");
		nBytes = snprintf(buf, STRING_SIZE, "Operator data unavailable!\n\n");
		write(_fileDescriptor, buf, nBytes);
		return ERR_NOT_INITIALIZED;
	}
	
	nBytes = snprintf(buf, STRING_SIZE, "Operator: %s\n", _operator->m_operatorName);
	if (write(_fileDescriptor, buf, nBytes) < 0)
	{
		GetError(errMsg, ERR_FILE_WRITE);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_FILE_WRITE;
	}
	nBytes = snprintf(buf, STRING_SIZE, "----------------------\n");
	if (write(_fileDescriptor, buf, nBytes) < 0)
	{
		GetError(errMsg, ERR_FILE_WRITE);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_FILE_WRITE;
	}
	nBytes = snprintf(buf, STRING_SIZE, "Total incoming calls duration: %u\n", _operator->m_incomingDuration);
	if (write(_fileDescriptor, buf, nBytes) < 0)
	{
		GetError(errMsg, ERR_FILE_WRITE);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_FILE_WRITE;
	}
	nBytes = snprintf(buf, STRING_SIZE, "Total outgoing calls duration: %u\n", _operator->m_outgoingDuration);
	if (write(_fileDescriptor, buf, nBytes) < 0)
	{
		GetError(errMsg, ERR_FILE_WRITE);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_FILE_WRITE;
	}
	nBytes = snprintf(buf, STRING_SIZE, "Total messages received: %u\n", _operator->m_messagesReceived);
	if (write(_fileDescriptor, buf, nBytes) < 0)
	{
		GetError(errMsg, ERR_FILE_WRITE);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_FILE_WRITE;
	}
	nBytes = snprintf(buf, STRING_SIZE, "Total messages sent: %u\n", _operator->m_messagesSent);
	if (write(_fileDescriptor, buf, nBytes) < 0)
	{
		GetError(errMsg, ERR_FILE_WRITE);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_FILE_WRITE;
	}
	nBytes = snprintf(buf, STRING_SIZE, "Total downloaded data: %g [MB]\n", _operator->m_downloaded);
	if (write(_fileDescriptor, buf, nBytes) < 0)
	{
		GetError(errMsg, ERR_FILE_WRITE);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_FILE_WRITE;
	}
	nBytes = snprintf(buf, STRING_SIZE, "Total uploaded data: %g [MB]\n\n", _operator->m_uploaded);
	if (write(_fileDescriptor, buf, nBytes) < 0)
	{
		GetError(errMsg, ERR_FILE_WRITE);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_FILE_WRITE;
	}
	
	LOG_DEBUG_PRINT("%s", "Successfully printed operator to file.");
	return ERR_OK;
}

#ifdef _DEBUG
void OperatorPrint(const Operator* _operator)
{
	if (NULL == _operator)
	{
		printf("Operator data unavailable!\n");
		return;
	}
	
	printf("Operator: %s\n", _operator->m_operatorName);
	printf("----------------------\n");
	printf("Total incoming calls duration: %u\n", _operator->m_incomingDuration);
	printf("Total outgoing calls duration: %u\n", _operator->m_outgoingDuration);
	printf("Total messages received: %u\n", _operator->m_messagesReceived);
	printf("Total messages sent: %u\n", _operator->m_messagesSent);
	printf("Total downloaded data: %g [MB]\n", _operator->m_downloaded);
	printf("Total uploaded data: %g [MB]\n\n", _operator->m_uploaded);
}
#endif /* _DEBUG */
