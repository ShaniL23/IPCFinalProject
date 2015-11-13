/**************************************************************************************************
	Author: 				Tom Vizel
	Creation date: 			29.10.15
	Last modified date: 	29.10.15
	
	Description: Implementation for various ADT errors.
**************************************************************************************************/

#include <string.h>
#include <errno.h>

#include "ADTErr.h"

ADTErr GetError(char* _msg, ADTErr _error)
{
	if (NULL == _msg)
	{
		return ERR_ILLEGAL_INPUT;
	}
	
	switch(_error)
	{
		/* General ADT errors */
		case ERR_OK:
		{
			strcpy(_msg, "No error!");
			break;
		}
		case ERR_GENERAL:
		{
			strcpy(_msg, "General error.");
			break;
		}
		case ERR_NOT_INITIALIZED:
		{
			strcpy(_msg, "ADT is not initialized!");
			break;
		}
		case ERR_ALLOCATION_FAILED:
		{
			strcpy(_msg, "Memory allocation failed!");
			break;
		}
		case ERR_REALLOCATION_FAILED:
		{
			strcpy(_msg, "Memory reallocation failed!");
			break;
		}
		case ERR_UNDERFLOW:
		{
			strcpy(_msg, "Underflow.");
			break;
		}
		case ERR_OVERFLOW:
		{
			strcpy(_msg, "Overflow.");
			break;
		}
		case ERR_WRONG_INDEX:
		{
			strcpy(_msg, "Wrong index.");
			break;
		}
		case ERR_ILLEGAL_INPUT:
		{
			strcpy(_msg, "Illegal input!");
			break;
		}
		/* Hash errors */
		case ERR_ALREADY_EXISTS:
		{
			strcpy(_msg, "Cannot insert: Already exists in database!");
			break;
		}
		case ERR_NOT_FOUND:
		{
			strcpy(_msg, "Cannot retrieve from DB: does not exist!");
			break;
		}
		/* File errors */
		case ERR_FILE_OPEN:
		{
			strcpy(_msg, "Failed to open file: ");
			strcat(_msg, strerror(errno));
			break;
		}
		case ERR_FILE_CLOSE:
		{
			strcpy(_msg, "Failed to close file: ");
			strcat(_msg, strerror(errno));
			break;
		}
		/* Parser module errors */
		case ERR_PARSING_FAILED:
		{
			strcpy(_msg, "Failed to Parse string");
			break;
		}

		case ERR_SENDING2Q_FAILED:
		{
			strcpy(_msg, "Failed to send CDR to queue");
			break;
		}

		case ERR_INSERTION2CDR_FAILED:
		{
			strcpy(_msg, "Failed to insert string to CDR");
			break;
		}

		/* Thread errors */
		case ERR_THREAD_CANT_CREATE:
		{
			strcpy(_msg, "Failed to create thread: ");
			strcat(_msg, strerror(errno));
			break;
		}
		case ERR_THREAD_CANT_JOIN:
		{
			strcpy(_msg, "Failed to join thread: ");
			strcat(_msg, strerror(errno));
			break;
		}

		/* Dir Errors */
		case ERR_CANT_OPEN_DIR:
		{
			strcpy(_msg, "cannot open directory: ");
			strcat(_msg, strerror(errno));
			break;
		}
		
		/* DBManager Errors */
		case ERR_DATA_PREP_FAILED:
		{
			strcpy(_msg, "Failed to prep data\n");
			break;
		}
		case ERR_DB_UPDATE_FAILED:
		{
			strcpy(_msg, "Failed to update database\n");
			break;
		}	
		case ERR_SIGACTION:
		{
			strcpy(_msg, "Planting signal handler failed\n");
			break;
		}		

		/* DEFAULT */
		default:
		{
			strcpy(_msg, "Unknown error type!");
			break;
		}
	}
	
	return ERR_OK;
}
