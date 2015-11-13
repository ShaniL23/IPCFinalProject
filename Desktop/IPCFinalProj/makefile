CC = gcc
CFLAGS = -c -pedantic -ansi -Wall -Werror -std=gnu99 -D _LOGGER
OBJS =  ADTErr.o Billing.o cdr.o DataManager.o FilesReader.o GHashMap.o GLList.o GStack.o Operator.o OperatorDB.o parser.o queue.o safeQueue.o Subscriber.o SubscriberDB.o semaphore.o RunBilling.o logger.o

OP_OBJS = ADTErr.o logger.o cdr.o Operator.o OperatorTest.o
SUB_OBJS = ADTErr.o logger.o cdr.o Subscriber.o SubscriberTest.o
OPDB_OBJS = ADTErr.o logger.o cdr.o Operator.o GLList.o GHashMap.o OperatorDB.o OperatorDBTest.o
SUBDB_OBJS = ADTErr.o logger.o cdr.o Subscriber.o GLList.o GHashMap.o SubscriberDB.o SubscriberDBTest.o
UNIT_OBJS = $(OBJS) logger.o OperatorTest.o SubscriberTest.o SubscriberDBTest.o OperatorDBTest.o

LOG = logger.h logger_pub.h

RunBilling : $(OBJS)
	$(CC) -o RunBilling $(OBJS) -pthread

logger.o : logger.c $(LOG) ADTErr.h
	$(CC) -o logger.o $(CFLAGS) logger.c

ADTErr.o : ADTErr.c ADTErr.h
	$(CC) -o ADTErr.o $(CFLAGS) ADTErr.c

semaphore.o : semaphore.c semaphore.h ADTErr.h
	$(CC) -o semaphore.o $(CFLAGS) semaphore.c

queue.o : queue.c queue.h ADTErr.h GData.h
	$(CC) -o queue.o $(CFLAGS) queue.c

safeQueue.o : safeQueue.c safeQueue.h queue.h ADTErr.h semaphore.h GData.h
	$(CC) -o safeQueue.o -c $(CFLAGS) safeQueue.c

GLList.o : GLList.c GLList.h GData.h
	$(CC) -o GLList.o $(CFLAGS) GLList.c

GStack.o : GStack.c GStack.h ADTErr.h GData.h GLList.h
	$(CC) -o GStack.o $(CFLAGS) GStack.c

cdr.o : cdr.c cdr.h ADTErr.h $(LOG)
	$(CC) -o cdr.o $(CFLAGS) cdr.c

parser.o : parser.c parser.h ADTErr.h GData.h safeQueue.h cdr.h $(LOG)
	$(CC) -o parser.o $(CFLAGS) parser.c

FilesReader.o : FilesReader.c FilesReader.h ADTErr.h GData.h safeQueue.h GStack.h cdr.h parser.h $(LOG)
	$(CC) -o FilesReader.o $(CFLAGS) FilesReader.c

Billing.o : Billing.c ADTErr.h Billing.h DataManager.h GData.h safeQueue.h cdr.h Operator.h Subscriber.h OperatorDB.h SubscriberDB.h $(LOG)
	$(CC) -o Billing.o $(CFLAGS) Billing.c

DataManager.o : DataManager.c DataManager.h ADTErr.h safeQueue.h cdr.h Operator.h Subscriber.h OperatorDB.h SubscriberDB.h GData.h $(LOG)
	$(CC) -o DataManager.o $(CFLAGS) DataManager.c

GHashMap.o : GHashMap.c GHashMap.h ADTErr.h GData.h GLList.h
	$(CC) -o GHashMap.o $(CFLAGS) GHashMap.c

Operator.o : Operator.c Operator.h ADTErr.h cdr.h $(LOG)
	$(CC) -o Operator.o $(CFLAGS) Operator.c

OperatorDB.o : OperatorDB.c OperatorDB.h ADTErr.h GLList.h GHashMap.h cdr.h Operator.h $(LOG)
	$(CC) -o OperatorDB.o $(CFLAGS) OperatorDB.c

Subscriber.o : Subscriber.c Subscriber.h ADTErr.h cdr.h $(LOG)
	$(CC) -o Subscriber.o $(CFLAGS) Subscriber.c

SubscriberDB.o : SubscriberDB.c SubscriberDB.h ADTErr.h GLList.h GHashMap.h cdr.h Subscriber.h $(LOG)
	$(CC) -o SubscriberDB.o $(CFLAGS) SubscriberDB.c

RunBilling.o : RunBilling.c ADTErr.h safeQueue.h Billing.h DataManager.h FilesReader.h SubscriberDB.h Subscriber.h Operator.h OperatorDB.h $(LOG)
	$(CC) -c $(CFLAGS) RunBilling.c
	
UNITS: OperatorDBUNIT OperatorUNIT SubscriberDBUNIT SubscriberUNIT

OperatorUNIT: $(OP_OBJS)
	$(CC) -o OperatorUNIT $(OP_OBJS)

OperatorTest.o: testOperator.c ADTErr.h cdr.h Operator.h
	$(CC) -o OperatorTest.o $(CFLAGS) -D _DEBUG testOperator.c
	
SubscriberUNIT: $(SUB_OBJS)
	$(CC) -o SubscriberUNIT $(SUB_OBJS)

SubscriberTest.o: testSubscriber.c ADTErr.h cdr.h Subscriber.h
	$(CC) -o SubscriberTest.o $(CFLAGS) -D _DEBUG testSubscriber.c
	
OperatorDBUNIT: $(OPDB_OBJS)
	$(CC) -o OperatorDBUNIT $(OPDB_OBJS)

OperatorDBTest.o: testOpDB.c ADTErr.h cdr.h Operator.h OperatorDB.h
	$(CC) -o OperatorDBTest.o $(CFLAGS) -D _DEBUG testOpDB.c
	
SubscriberDBUNIT: $(SUBDB_OBJS)
	$(CC) -o SubscriberDBUNIT $(SUBDB_OBJS)

SubscriberDBTest.o: testSubDB.c ADTErr.h cdr.h Subscriber.h SubscriberDB.h
	$(CC) -o SubscriberDBTest.o $(CFLAGS) -D _DEBUG testSubDB.c

clean :
	rm -f $(OBJS)
	
UnitClean:
	rm -f $(UNIT_OBJS) 

rebuild : clean RunBilling
