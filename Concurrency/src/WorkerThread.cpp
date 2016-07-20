
#include "hulk_assert.h"

#include "WorkerThread.hpp"
#include "Task.hpp"

using namespace Concurrency;

DWORD WINAPI WorkerThread::MyThreadFunction( LPVOID lpParam )
{
	ThreadParams * pParams = static_cast<ThreadParams*> (lpParam);

	while (pParams->status != e_stopping)
	{
		if (pParams->status == e_lowLatencyMode) {
			Yield();
		}
		else {
			WaitForSingleObject(pParams->waitEvent,INFINITE);
		}
 		Task * pTask =NULL;
		bool gotNext =	pParams->pRingBuffer->getNext(pTask);
		while (gotNext) {
			pTask->execute();
			delete  pTask;
			pTask = NULL;
			gotNext =  pParams->pRingBuffer->getNext(pTask);
		}
	}

	pParams->status = e_done;
	return 0;
}

bool WorkerThread::initialize(RingBuffer<Task*> * ring )
{
	HULK_ASSERT(m_pThreadAParams == NULL, " WorkerThread class initialized twice\n");
	
	m_pThreadAParams = new ThreadParams();

	m_pThreadAParams->pRingBuffer = ring;
	m_pThreadAParams->status = e_running;
	m_pThreadAParams->waitEvent = CreateEvent(NULL,false,false,NULL);

	HANDLE threadHandle = CreateThread(NULL,0,MyThreadFunction,m_pThreadAParams,0,NULL);

	return threadHandle != NULL;
}

bool WorkerThread::invoke()
{
	SetEvent(m_pThreadAParams->waitEvent);
	return true;
}

bool WorkerThread::terminate()
{
	m_pThreadAParams->status = e_stopping;

	invoke();

	//busy wait for the thread to finish
	while (m_pThreadAParams->status != e_done);

	CloseHandle (m_pThreadAParams->waitEvent);

	delete m_pThreadAParams;
	m_pThreadAParams = NULL;

	return true;
}

void WorkerThread::enterLowLatencyMode()
{
	m_pThreadAParams->status = e_lowLatencyMode;
	invoke();
}
void WorkerThread::exitLowLatencyMode()
{
	m_pThreadAParams->status = e_running;
}