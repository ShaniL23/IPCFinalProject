/**************************************************************************************************************************************
Author : Erez Atar	
Creation Date : 9.11.2015
Last modified date 9.11.2015
Description : Files - open all files from directory and send CDR lines to Q
								-- header file --
**************************************************************************************************************************************/
#ifndef __FILESREADER_H__
#define __FILESREADER_H__

/* the function create n threads. every thread reads lines from file in directory "Stroage" and send line by line to Q.
   the thread will send lines to Q until we dont have files any more
 */
ADTErr InitReaders(SafeQueue* _queue,char* _path);

ADTErr EndReaders(SafeQueue* _queue);

#endif /* __FILESREADER_H__ */
