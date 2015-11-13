/**************************************************************************************************
	Author: 				Tom Vizel
	Creation date: 			9.11.15
	Last modified date: 	9.11.15
	
	Description: Implementation module for SubscriberDB functions.
**************************************************************************************************/

#include <stdio.h> /* printf, snprintf */
#include <stdlib.h> /* malloc */
#include <stdbool.h> /* true/false */
#include <unistd.h> /* close */
#include <fcntl.h> /* open */
#include <string.h> /* strcmp */

#include "ADTErr.h"
#include "logger.h"
#include "logger_pub.h"
#include "GData.h"
#include "GHashMap.h"
#include "cdr.h"
#include "Subscriber.h"
#include "SubscriberDB.h"

struct SubscriberDB
{
	HashMap* m_map;
};

#define NUM_OF_BUCKETS 1000000
#define IMSI_SIZE 32
#define ERR_MSG_SIZE 128

/* Hash function */
/*static unsigned int qhashmurmur3_32(HashKey _data, size_t _ignore)
{
    const unsigned int c1 = 0xcc9e2d51;
    const unsigned int c2 = 0x1b873593;

    int nblocks;
    const unsigned int* blocks;
    const unsigned int* tail;

    unsigned int h = 0;

    int i;
    unsigned int k;
    
    size_t nBytes;
    
    if (_data == NULL)
    {
        return 0;
	}
	
	nBytes = strlen((char*)_data);
	nblocks = nBytes / 4;
	blocks = (const unsigned int *) (_data);
	tail = ((const unsigned int *)((char*)_data + (nblocks * 4)));
    
    for (i = 0; i < nblocks; i++)
    {
        k = blocks[i];

        k *= c1;
        k = (k << 15) | (k >> (32 - 15));
        k *= c2;

        h ^= k;
        h = (h << 13) | (h >> (32 - 13));
        h = (h * 5) + 0xe6546b64;
    }

    k = 0;
    switch (nBytes & 3)
    {
        case 3:
        {
            k ^= tail[2] << 16;
            break;
        }
        case 2:
       	{
            k ^= tail[1] << 8;
            break;
        }
        case 1:
        {
            k ^= tail[0];
            k *= c1;
            k = (k << 15) | (k >> (32 - 15));
            k *= c2;
            h ^= k;
            break;
        }
    }

    h ^= nBytes;

    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}*/

static unsigned int DJB2(HashKey _imsi, size_t _ignore)
{
	unsigned int hash = 0;
	int c;
	char* str = (char*)_imsi;
	
	while ((c = *str++))
	{
		hash = ((hash << 5) + hash) + c;
	}
	
	return hash;
}

static int FreeSubscribers(HashKey _imsi, Data _subscriber, void* _ignore)
{
	if (NULL == _subscriber)
	{
		return false;
	}
	
	SubscriberDestroy((Subscriber*)_subscriber);
	return true;
}

#ifdef _DEBUG
static int PrintSubscribers(HashKey _ignore, Data _data)
{
	SubscriberPrint((Subscriber*)_data);
	return true;
}
#endif /* _DEBUG */

static int CompareSubscribers(HashKey _imsi1, HashKey _imsi2)
{
	return ( ! strcmp((char*)_imsi1, (char*)_imsi2) );
	/*return SubscriberIsSame((Subscriber*)_imsi1, (Subscriber*)_imsi2);*/
}

SubscriberDB* SubscriberDBCreate(ADTErr* _err)
{
	char errMsg[ERR_MSG_SIZE];
	
	SubscriberDB* sdb = malloc(sizeof(SubscriberDB));
	if (NULL == sdb)
	{
		if (NULL != _err)
		{
			*_err = ERR_ALLOCATION_FAILED;
		}
		GetError(errMsg, ERR_ALLOCATION_FAILED);
		LOG_ERROR_PRINT("%s", errMsg);
		return NULL;
	}
	
	sdb->m_map = HashCreate(NUM_OF_BUCKETS, DJB2, CompareSubscribers);
	if (NULL == sdb->m_map)
	{
		if (NULL != _err)
		{
			*_err = ERR_ALLOCATION_FAILED;
		}
		GetError(errMsg, ERR_ALLOCATION_FAILED);
		LOG_ERROR_PRINT("%s", errMsg);
		free(sdb);
		return NULL;
	}
	
	if (NULL != _err)
	{
		*_err = ERR_OK;
	}
	LOG_DEBUG_PRINT("%s", "Successfully created subscriber database.");
	return sdb;
}

void SubscriberDBDestroy(SubscriberDB* _sdb)
{
	if (NULL == _sdb)
	{
		LOG_WARN_PRINT("%s", "Attempted to destroy uninitialized subscriber database");
		return;
	}
	
	HashForEach(_sdb->m_map, FreeSubscribers, NULL);
	HashDestroy(_sdb->m_map);
	free(_sdb);
	LOG_DEBUG_PRINT("%s", "Successfully destroyed subscriber database");
}

ADTErr SubscriberDBInsert(SubscriberDB* _sdb, const Subscriber* _sub)
{
	ADTErr err;
	char errMsg[ERR_MSG_SIZE];
	char* imsi = NULL;
	
	if (NULL == _sdb)
	{
		GetError(errMsg, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_NOT_INITIALIZED;
	}
	if (NULL == _sub)
	{
		GetError(errMsg, ERR_ILLEGAL_INPUT);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_ILLEGAL_INPUT;
	}
	
	imsi = (char*) malloc(IMSI_SIZE * sizeof(char));
	if (NULL == imsi)
	{
		GetError(errMsg, ERR_ALLOCATION_FAILED);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_ALLOCATION_FAILED;
	}
	
	SubscriberGetIMSI(_sub, imsi);
	err = HashInsert(_sdb->m_map, (const HashKey)imsi, (const Data)_sub);
	if (ERR_OK != err)
	{
		GetError(errMsg, err);
		LOG_WARN_PRINT("%s", errMsg);
	}
	else
	{
		LOG_DEBUG_PRINT("%s", "Successfully inserted to database.");
	}
	
	return err;
}

ADTErr SubscriberDBGet(const SubscriberDB* _sdb, const char* _imsi , Subscriber** _sub)
{
	char errMsg[ERR_MSG_SIZE];
	ADTErr err;
	
	if (NULL == _sdb)
	{
		GetError(errMsg, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_NOT_INITIALIZED;
	}
	if (NULL == _imsi || NULL == _sub)
	{
		GetError(errMsg, ERR_ILLEGAL_INPUT);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_ILLEGAL_INPUT;
	}
	
	*_sub = HashFind(_sdb->m_map, (const HashKey)_imsi);
	err = (NULL == *_sub) ? ERR_NOT_FOUND : ERR_OK;
	if (ERR_OK != err)
	{
		GetError(errMsg, err);
		LOG_WARN_PRINT("%s", errMsg);
	}
	else
	{
		LOG_DEBUG_PRINT("%s", "Successfully retrieved from database.");
	}
	return err;
}

ADTErr SubscriberDBRemove(SubscriberDB* _sdb, const char* _imsi, Subscriber** _sub)
{
	char errMsg[ERR_MSG_SIZE];
	ADTErr err;
	
	if (NULL == _sdb)
	{
		GetError(errMsg, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_NOT_INITIALIZED;
	}
	if (NULL == _imsi || NULL == _sub)
	{
		GetError(errMsg, ERR_ILLEGAL_INPUT);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_ILLEGAL_INPUT;
	}
	
	HashRemove(_sdb->m_map, (const HashKey)_imsi, (Data*)_sub);
	err = (NULL == *_sub) ? ERR_NOT_FOUND : ERR_OK;
	if (ERR_OK != err)
	{
		GetError(errMsg, err);
		LOG_WARN_PRINT("%s", errMsg);
	}
	else
	{
		LOG_DEBUG_PRINT("%s", "Successfully removed from database.");
	}
	return err;
}

static int PrintAllToFile(HashKey _key, Data _subscriber, void* _fileDesc)
{
	SubscriberPrintToFile((Subscriber*)_subscriber, *(int*)_fileDesc);
	return true;
}

ADTErr SubscriberDBPrintToFile(const SubscriberDB* _sdb, const char* _fileName)
{
	int fileDesc;
	char errMsg[ERR_MSG_SIZE];
	
	if (NULL == _sdb)
	{
		GetError(errMsg, ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_NOT_INITIALIZED;
	}
	if (NULL == _fileName)
	{
		GetError(errMsg, ERR_ILLEGAL_INPUT);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_ILLEGAL_INPUT;
	}
	
	fileDesc = open(_fileName, O_RDWR | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
	if (fileDesc < 0)
	{
		GetError(errMsg, ERR_FILE_OPEN);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_FILE_OPEN;
	}
	
	if ( ! HashForEach(_sdb->m_map, PrintAllToFile, (void*)&fileDesc) )
	{
		GetError(errMsg, ERR_GENERAL);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_GENERAL;
	}
	
	if (-1 == close(fileDesc))
	{
		GetError(errMsg, ERR_FILE_CLOSE);
		LOG_ERROR_PRINT("%s", errMsg);
		return ERR_FILE_CLOSE;
	}
	
	LOG_DEBUG_PRINT("Successfully wrote to file %s.", _fileName);
	return ERR_OK;
}

#ifdef _DEBUG
void SubscriberDBPrint(const SubscriberDB* _sdb)
{
	if (NULL == _sdb)
	{
		printf("Subscriber DB is not initialized!\n");
		return;
	}
	
	HashPrint(_sdb->m_map, PrintSubscribers);
}
#endif /* _DEBUG */
