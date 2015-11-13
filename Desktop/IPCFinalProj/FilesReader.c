/**************************************************************************************************************************************
Author : Erez Atar	
Creation Date : 9.11.2015
Last modified date 9.11.2015
Description :  FilesReader - open all files from directory and send CDR lines to Q 
**************************************************************************************************************************************/
#include <stdlib.h>
#include <dirent.h> /* to open directory */
#include <string.h> /* for strlen */
#include <stdio.h>
#include <fcntl.h> 
#include <pthread.h>

#include "ADTErr.h"
#include "logger.h"
#include "logger_pub.h"
#include "GData.h"
#include "safeQueue.h"
#include "GStack.h"
#include "cdr.h"
#include "parser.h"
#include "FilesReader.h"

#define CDR_LINE_SIZE 128
#define PATH_SIZE 64
#define NUM_OF_THREADS 10
#define SIZE_STR_ERR 100 

static pthread_t s_threads[NUM_OF_THREADS];
static Stack* s_stack;
static pthread_mutex_t errFileMutex = PTHREAD_MUTEX_INITIALIZER;

static Stack* CreateStackFillFilesName(char* _directoryName,ADTErr* _err);
static void* FileReader(void* _queue);
static ADTErr DestroyFileNamesStack(void);

ADTErr InitReaders(SafeQueue* _queue,char* _path)
{
	ADTErr err;
	int i;	
	char strErr[SIZE_STR_ERR];

	if(NULL == _queue)
	{
		pthread_mutex_destroy(&errFileMutex);
		GetError(strErr,ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s",strErr);
		return ERR_NOT_INITIALIZED;
	}
	/* The function will fill the stack with name files in directory Storage */
	s_stack = CreateStackFillFilesName(_path,&err);
	if(NULL == s_stack)
	{
		pthread_mutex_destroy(&errFileMutex);
		GetError(strErr,ERR_ALLOCATION_FAILED);
		LOG_ERROR_PRINT("%s",strErr);
		return ERR_ALLOCATION_FAILED;
	}
	for(i = 0; i < NUM_OF_THREADS; i++)
	{
		if(pthread_create(&s_threads[i], NULL,FileReader,(void*)_queue) != 0)
		{
			DestroyFileNamesStack();
			pthread_mutex_destroy(&errFileMutex);
			LOG_ERROR_PRINT("Create thread %d failed",i);
			return ERR_THREAD_CANT_CREATE;
		}
		LOG_DEBUG_PRINT("Thread %d created",i);
	}
	return ERR_OK;
}

ADTErr EndReaders(SafeQueue* _queue)
{
	int i;
	ADTErr err;
	char strErr[SIZE_STR_ERR];	

	if(NULL == _queue)
	{
		GetError(strErr,ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s",strErr);
		return ERR_NOT_INITIALIZED;
	}
	for(i = 0; i < NUM_OF_THREADS; i++)
	{
		if(pthread_join(s_threads[i], NULL) != 0)
		{
			LOG_ERROR_PRINT("Join thread %d failed",i);
			return ERR_THREAD_CANT_JOIN;
		}
	}
	pthread_mutex_destroy(&errFileMutex);
	if((err = DestroyFileNamesStack()) != ERR_OK)	
	{
		GetError(strErr,err);
		LOG_ERROR_PRINT("%s",strErr);
		return err;
	}
	if(ERR_OK != (err = SendEndMsg2Queue(_queue)))
	{
		GetError(strErr,err);
		LOG_ERROR_PRINT("%s",strErr);
		return err;
	}
	return ERR_OK;
}

static Stack* CreateStackFillFilesName(char* _directoryName,ADTErr* _err)
{
	DIR* directoryP = NULL;
	struct dirent* dirData = NULL;
	Stack* stackLocal = NULL;
	int strSize = 0;
	char* path = NULL;
	char strErr[SIZE_STR_ERR];
	ADTErr error;

	if(NULL == _directoryName )
	{
		return NULL;
	}
	if(NULL == (stackLocal = StackCreate()))
	{
		if(NULL != _err)
		{
			*_err = ERR_ALLOCATION_FAILED;
		}
		GetError(strErr,ERR_ALLOCATION_FAILED);
		LOG_ERROR_PRINT("%s",strErr);
		return NULL;
	}
	if(NULL == (directoryP = opendir(_directoryName)))
	{
		StackDestroy(stackLocal);
		if(NULL != _err)
		{
			*_err = ERR_CANT_OPEN_DIR;
		}
		GetError(strErr,ERR_CANT_OPEN_DIR);
		LOG_ERROR_PRINT("%s",strErr);
		return NULL;
	}
	while((dirData = readdir(directoryP)) != NULL)
	{
		strSize = strlen(dirData->d_name);
		/* ignore tmp files */
		if(dirData->d_name[0] != '.' && dirData->d_name[strSize -1] != '~')
		{
			path = malloc(PATH_SIZE);
			strcpy(path,_directoryName);
			if(NULL == path)
			{
				if(NULL != _err)
				{	
					*_err = ERR_ALLOCATION_FAILED;
				}
				closedir(directoryP);
				StackDestroy(stackLocal);
				LOG_ERROR_PRINT("%s","strcopy to path -faild");
				return NULL;
			} 
			strcat(path,dirData->d_name);
			if(ERR_OK != (error = StackPush(stackLocal, path)))
			{
				StackDestroy(stackLocal);
				closedir(directoryP);
				if(NULL != _err)
				{	
					*_err = error;
				}
				GetError(strErr,error);
				LOG_ERROR_PRINT("%s",strErr);
				return NULL;
			}
		}
	}
	closedir(directoryP);
	if(NULL != _err)
	{
		*_err = ERR_OK;
	}
	return stackLocal;
}

static ADTErr DestroyFileNamesStack(void)
{	
	char* filePath = NULL;
	char strErr[SIZE_STR_ERR];
	ADTErr error;

	if(NULL == s_stack)
	{
		GetError(strErr,ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s",strErr);
		return ERR_NOT_INITIALIZED;
	}
	/* if stack is not empty, gets and free all the items */
	while(ERR_OK == (error = StackPop(s_stack,(void**)&filePath)))
	{
		free(filePath);
	}
	StackDestroy(s_stack);
	return ERR_OK;
}

static void* FileReader(void* _queue)
{
	char* filePath = NULL;
	SafeQueue* queue = NULL;
	char cdrLine[CDR_LINE_SIZE];
	CDR* cdr;
	FILE* fp = NULL;
	FILE* fpFileErr = NULL;
	char strErr[SIZE_STR_ERR];

	if(NULL == _queue)
	{
		GetError(strErr,ERR_NOT_INITIALIZED);
		LOG_ERROR_PRINT("%s",strErr);
		return NULL;
	}
	queue = ((SafeQueue*)_queue);
    /* loop - until the stack is empty */
	while(ERR_OK == StackPop(s_stack,(void**)&filePath))
	{
		if((fp = fopen(filePath,"r")) == NULL)
		{
			LOG_ERROR_PRINT("%s","open file failed");
			return NULL;
		}
		free(filePath);
		while(fgets(cdrLine,CDR_LINE_SIZE,fp))
		{	
			LOG_DEBUG_PRINT("read line : %s",cdrLine);
			if(ERR_OK != Parse(cdrLine,&cdr))
			{
				pthread_mutex_lock(&errFileMutex);   /** LOCK **/
				fpFileErr = fopen("ErrorLines","a");
				fprintf(fpFileErr,"ERR LINE- IMSI - %s\n",cdrLine);
				fclose(fpFileErr);
				pthread_mutex_unlock(&errFileMutex);  /** UNLOCK **/
				continue;
			}
			SendCDR2Queue(cdr,queue);
		}
		fclose(fp);
	}
	pthread_exit(NULL);
}
