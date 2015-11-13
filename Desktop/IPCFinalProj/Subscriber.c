/**************************************************************************************************
	Author: 				Tom Vizel
	Creation date: 			9.11.15
	Last modified date: 	9.11.15
	
	Description: Implementation module for subscriber functions.
**************************************************************************************************/

#include <stdio.h> /* printf, snprintf*/
#include <stdlib.h> /* malloc, NULL */
#include <string.h> /* strcpy */
#include <unistd.h> /* write */

#include "ADTErr.h"
#include "logger.h"
#include "logger_pub.h"
#include "cdr.h"
#include "Subscriber.h"

#define STRING_SIZE 64
#define ERR_MSG_SIZE 128

struct Subscriber
{
	char 			m_imsi[STRING_SIZE];
	unsigned int 	m_incomingDuration;
	unsigned int 	m_outgoingDuration;
	unsigned int 	m_messagesReceived;
	unsigned int 	m_messagesSent;
	double			m_downloaded;
	double 			m_uploaded;
};

Subscriber* SubscriberCreate(CDR* _cdr, ADTErr* _err)
{
	Subscriber* sub = NULL;
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
	
	sub = (Subscriber*) calloc(1, sizeof(Subscriber));
	if (NULL == sub)
	{
		if (NULL != _err)
		{
			*_err = ERR_ALLOCATION_FAILED;
		}
		GetError(errMsg, ERR_ALLOCATION_FAILED);
		LOG_ERROR_PRINT("%s", errMsg);
		return NULL;
	}
	
	CDRGetIMSI(_cdr, sub->m_imsi);
	CDRGetCallType(_cdr, &type);
	switch (type)
	{
		case MOC:
		{
			CDRGetCallDuration(_cdr, &sub->m_outgoingDuration);
			LOG_DEBUG_PRINT("Subscriber %s updated outgoing call duration by %u seconds.", sub->m_imsi, sub->m_outgoingDuration);
			break;
		}
		case MTC:
		{
			CDRGetCallDuration(_cdr, &sub->m_incomingDuration);
			LOG_DEBUG_PRINT("Subscriber %s updated incoming call duration by %u seconds.", sub->m_imsi, sub->m_incomingDuration);
			break;
		}
		case SMS_MO:
		{
			++sub->m_messagesSent;
			LOG_DEBUG_PRINT("Subscriber %s logged 1 sent message.", sub->m_imsi);
			break;
		}
		case SMS_MT:
		{
			++sub->m_messagesReceived;
			LOG_DEBUG_PRINT("Subscriber %s logged 1 received message.", sub->m_imsi);
			break;
		}
		case GPRS:
		{
			CDRGetDownloadedMB(_cdr, &sub->m_downloaded);
			CDRGetUploadedMB(_cdr, &sub->m_uploaded);
			LOG_DEBUG_PRINT("Subscriber %s: %g MB downloaded, %g MB uploaded.", sub->m_imsi, sub->m_downloaded, sub->m_uploaded);
			break;
		}
		default:
		{
			free(sub);
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
	LOG_DEBUG_PRINT("%s", "Successfully created subscriber!");
	return sub;
}

void SubscriberDestroy(Subscriber* _sub)
{
	if (NULL == _sub)
	{
		LOG_WARN_PRINT("%s", "Attempted to destroy uninitialized subscriber.");
		return;
	}
	
	free(_sub);
	LOG_DEBUG_PRINT("%s", "Successfully destroyed subscriber.");
}

ADTErr SubscriberUpdate(Subscriber* _destination, const Subscriber* _source)
{
	char errMsg[ERR_MSG_SIZE];
	
	if (NULL == _destination || NULL == _source)
	{
		GetError(errMsg, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_NOT_INITIALIZED;
	}
	
	if (strcmp(_destination->m_imsi, _source->m_imsi))
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
	
	LOG_DEBUG_PRINT("Successfully updated %s.", _destination->m_imsi);
	return ERR_OK;
}

int	SubscriberIsSame(const Subscriber* _sub1, const Subscriber* _sub2)
{
	if (NULL == _sub1 || NULL == _sub2)
	{
		return (_sub1 == _sub2);
	}
	
	LOG_DEBUG_PRINT("Compared 2 subscribers: %s and %s.", _sub1->m_imsi, _sub2->m_imsi);
	return ( ! strcmp(_sub1->m_imsi, _sub2->m_imsi) );
}

ADTErr SubscriberGetIMSI(const Subscriber* _sub, char* _imsi)
{
	char errMsg[ERR_MSG_SIZE];
	
	if (NULL == _sub)
	{
		GetError(errMsg, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_NOT_INITIALIZED;
	}
	if (NULL == _imsi)
	{
		GetError(errMsg, ERR_ILLEGAL_INPUT);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_ILLEGAL_INPUT;
	}
	
	strcpy(_imsi, _sub->m_imsi);
	LOG_DEBUG_PRINT("Retrieved subscriber imsi: %s", _sub->m_imsi);
	return ERR_OK;
}

ADTErr SubscriberPrintToFile(const Subscriber* _sub, const int _fileDescriptor)
{
	int nBytes;
	char buf[STRING_SIZE];
	char errMsg[ERR_MSG_SIZE];
	
	if (_fileDescriptor < 0)
	{
		LOG_ERROR_PRINT("%s", "Invalid file descriptor for output!");
		return ERR_ILLEGAL_INPUT;
	}
	
	if (NULL == _sub)
	{
		LOG_WARN_PRINT("%s", "Subscriber data unavailable!");
		nBytes = snprintf(buf, STRING_SIZE, "Subscriber data unavailable!\n\n");
		write(_fileDescriptor, buf, nBytes);
		return ERR_NOT_INITIALIZED;
	}
	
	nBytes = snprintf(buf, STRING_SIZE, "IMSI: %s\n", _sub->m_imsi);
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
	nBytes = snprintf(buf, STRING_SIZE, "Total incoming calls duration: %u\n", _sub->m_incomingDuration);
	if (write(_fileDescriptor, buf, nBytes) < 0)
	{
		GetError(errMsg, ERR_FILE_WRITE);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_FILE_WRITE;
	}
	nBytes = snprintf(buf, STRING_SIZE, "Total outgoing calls duration: %u\n", _sub->m_outgoingDuration);
	if (write(_fileDescriptor, buf, nBytes) < 0)
	{
		GetError(errMsg, ERR_FILE_WRITE);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_FILE_WRITE;
	}
	nBytes = snprintf(buf, STRING_SIZE, "Total messages received: %u\n", _sub->m_messagesReceived);
	if (write(_fileDescriptor, buf, nBytes) < 0)
	{
		GetError(errMsg, ERR_FILE_WRITE);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_FILE_WRITE;
	}
	nBytes = snprintf(buf, STRING_SIZE, "Total messages sent: %u\n", _sub->m_messagesSent);
	if (write(_fileDescriptor, buf, nBytes) < 0)
	{
		GetError(errMsg, ERR_FILE_WRITE);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_FILE_WRITE;
	}
	nBytes = snprintf(buf, STRING_SIZE, "Total downloaded data: %g [MB]\n", _sub->m_downloaded);
	if (write(_fileDescriptor, buf, nBytes) < 0)
	{
		GetError(errMsg, ERR_FILE_WRITE);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_FILE_WRITE;
	}
	nBytes = snprintf(buf, STRING_SIZE, "Total uploaded data: %g [MB]\n\n", _sub->m_uploaded);
	if (write(_fileDescriptor, buf, nBytes) < 0)
	{
		GetError(errMsg, ERR_FILE_WRITE);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_FILE_WRITE;
	}
	
	LOG_DEBUG_PRINT("%s", "Successfully printed subscriber to file.");
	return ERR_OK;
}

#ifdef _DEBUG
void SubscriberPrint(const Subscriber* _sub)
{
	if (NULL == _sub)
	{
		printf("Subscriber data unavailable!\n\n");
		return;
	}
	
	printf("IMSI: %s\n", _sub->m_imsi);
	printf("----------------------\n");
	printf("Total incoming calls duration: %u\n", _sub->m_incomingDuration);
	printf("Total outgoing calls duration: %u\n", _sub->m_outgoingDuration);
	printf("Total messages received: %u\n", _sub->m_messagesReceived);
	printf("Total messages sent: %u\n", _sub->m_messagesSent);
	printf("Total downloaded data: %g [MB]\n", _sub->m_downloaded);
	printf("Total uploaded data: %g [MB]\n\n", _sub->m_uploaded);
}
#endif /* _DEBUG */
