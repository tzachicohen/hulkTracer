

#include "gtest\gtest.h"

#include "RingBuffer.hpp"
#include "ThreadPool.hpp"
#include "MemoryPool.hpp"

const int c_inputSize = 1024*1024*5;
const unsigned short c_bufferSize = 1024*60;
const int c_consumerCount =3;


using namespace Concurrency;

struct ThreadData
{
	HANDLE  endEvent;
	RingBuffer<int> * buffer;
	std::vector<int> vec;
};

DWORD WINAPI producerThreadFunction( LPVOID lpParam )
{
	ThreadData * data = (ThreadData*) lpParam;

	for (int i = 0 ; i< c_inputSize; )
	{
		if (data->buffer->insert(i))
			i++;
	}
	SetEvent(data->endEvent);
	
	return 0;
}

DWORD WINAPI consumerThreadFunction( LPVOID lpParam )
{
	ThreadData * data = (ThreadData*) lpParam;
	data->vec.reserve(c_inputSize);
	int i = 0;
	while (i < (c_inputSize - c_consumerCount))
	{
		int tmp;
		if (data->buffer->getNext(tmp))
		{
			data->vec.push_back(tmp);
			i = tmp;
		}
	}
	SetEvent(data->endEvent);
	
	return 0;
}

TEST(Concurrency, RingBuffer) {
	clock_t start,end;
	HANDLE * pEvents = new HANDLE[c_consumerCount+1];
	ThreadData * data  = new ThreadData[c_consumerCount+1];
	RingBuffer<int> * pRing = new RingBuffer<int>();
	pRing->initialize(c_bufferSize);
	for (int i =0 ; i < c_consumerCount+1 ; i++)
	{
		data[i].endEvent = pEvents[i] = CreateEvent(NULL,FALSE,FALSE,NULL);
		data[i].buffer = pRing;
	}
	start = clock();
	//producer thread
	CreateThread(NULL,0,producerThreadFunction,&(data[0]),0,0);

	//create consumer threads
	for (int i =1 ; i <= c_consumerCount ; i++)
	{
			CreateThread(NULL,0,consumerThreadFunction,&(data[i]),0,0);
	}
	
	WaitForMultipleObjects(c_consumerCount+1,pEvents,TRUE,INFINITE);

	end = clock();

	//validate the input
	int * buffer = new int [c_inputSize];
	memset(buffer,0,sizeof(int)*c_inputSize);
	for (int i =1 ; i <= c_consumerCount ; i++)
	{
		for (std::vector<int>::iterator it = data[i].vec.begin(); it != data[i].vec.end();++it)
		{
			int a = *it;
			if (buffer[a] ==0)
				buffer[a] = i;
			else
				printf ("consumer thread %d and %d got the same packet slot %d\n",i,buffer[a],a);
		}
	}
	bool success = true;
	for (int i =0 ; i <= c_inputSize -c_consumerCount ; i++)
	{
		if (buffer[i] == 0)
		{
			printf ("found value %d at slot %d fail !\n",buffer[i],i);
			success = false;
			//return 0;
		}
	}

	float timeDiff =( (float ) end)-start / CLOCKS_PER_SEC;
	EXPECT_TRUE(success);
	
	//clean up memory
	delete pRing;
	delete [] buffer;
	delete [] data;
	delete [] pEvents;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//this test is calculating the fibonaci heap using the 
//thread pool tasking system

volatile long g_answer = 0;

class Fib : public Task
{
public:
	Fib(TaskSet & taskSet): Task(taskSet) {}
	unsigned int m_num;
	virtual bool executeImp()
	{
		if (1 == m_num){
			InterlockedIncrement(&g_answer);
		}
		if (m_num > 1) {
			Fib * p2 = new Fib(m_taskSet);
			p2->m_num = m_num-1;
			m_pThreadPool->enqueueTask(p2);
			
			Fib * p1 = new Fib(m_taskSet);
			p1->m_num = m_num-2;
			m_pThreadPool->enqueueTask(p1);
		}
		return true;
	}
};
TEST(Concurrency, threadPool) {
	ThreadPool pool;
	TaskSet taskSet;
	
	bool status = pool.initialize();
	EXPECT_TRUE(status);
	
	Fib * pTask = new Fib(taskSet);
	pTask->m_num = 20 ; 
	pool.enqueueTask(pTask);

	taskSet.waitFortaskSet();

	EXPECT_EQ(6765 , g_answer);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
TEST(Concurrency, MemoryPool) {
	Concurrency::MemoryPool<ULONG_PTR> pool;
	pool.initialize(10,10);
	ULONG_PTR * arr[30];
	for (int i= 0 ; i < 30; i++) {
		arr[i] = pool.allocate();
		*(arr[i]) = i;
	}
	for (int i= 0 ; i < 30; i++) {
		pool.free(arr[i]);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
class MemPoolCheck : public Task
{
public:
	MemPoolCheck(TaskSet & taskSet): Task(taskSet) {}
	Concurrency::MemoryPool<DWORD>* m_pool;
	virtual bool executeImp()
	{
		DWORD * arr[30];
		DWORD threadId = GetCurrentThreadId();
		for (int j =0 ; j < 100000; j++) {
			for (int i= 0 ; i < 30; i++) 
			{
				arr[i] = m_pool->allocate();
				*(arr[i]) = threadId;
			}
			for (int i= 0 ; i < 30; i++) {
				EXPECT_EQ(*(arr[i]) , threadId);
				m_pool->free(arr[i]);
			}
		}
		return true;
	}
};

TEST(Concurrency, MemoryPoolMT) {
	
	Concurrency::MemoryPool<DWORD> MemPool;
	ThreadPool pool;
	TaskSet taskSet;

	MemPool.initialize(10,10);
	bool status = pool.initialize();
	EXPECT_TRUE(status);
	
	MemPoolCheck * pTask1 = new MemPoolCheck(taskSet);
	pTask1->m_pool = &MemPool;
	MemPoolCheck * pTask2 = new MemPoolCheck(taskSet);
	pTask2->m_pool = &MemPool;
	MemPoolCheck * pTask3 = new MemPoolCheck(taskSet);
	pTask3->m_pool = &MemPool;
	MemPoolCheck * pTask4 = new MemPoolCheck(taskSet);
	pTask4->m_pool = &MemPool;
	pool.enqueueTask(pTask1);
	pool.enqueueTask(pTask2);
	pool.enqueueTask(pTask3);
	pool.enqueueTask(pTask4);

	taskSet.waitFortaskSet();

}

