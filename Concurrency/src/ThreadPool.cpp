
#include "hulk_assert.h"

#include "ThreadPool.hpp"

using namespace Concurrency;

bool ThreadPool::initialize()
{
	//if we are already initialized
	if (m_workerThreads.size() > 0 ) {
		return true;
	}

	m_ringBuffer.initialize( 32768);

	SYSTEM_INFO info;
	memset(&info,0,sizeof(info));

	GetSystemInfo(&info);
	//open a worker thread for each processor
	bool retVal = true;
	for (unsigned int i = 0 ; i < info.dwNumberOfProcessors; i++)
	{
		WorkerThread * pThread = new WorkerThread();
		retVal &= pThread->initialize(&m_ringBuffer);
		m_workerThreads.push_back(pThread);
	}
	return retVal;
}

bool ThreadPool::cleanup()
{
	bool retVal = true;
	for (unsigned int i = 0 ; i < m_workerThreads.size() ; i++)
	{
		WorkerThread * pThread = m_workerThreads[i];
		//todo : improve eficiency by making two step termination
		retVal  &= pThread->terminate();
	}
	m_workerThreads.clear();
	return retVal;
}

bool ThreadPool::enqueueTask(Task * pTask)
{
	//register the thread pool handling the task
	pTask->m_pThreadPool = this;
	//increment the task set counter
	pTask->m_taskSet.incremetTask();

	bool retVal = m_ringBuffer.insert(pTask);
	if (retVal && !m_lowLatencyMode) {
		for (unsigned int i = 0 ; i < m_workerThreads.size() ; i++)
		{
			WorkerThread * pThread = m_workerThreads[i];
			//wake up all the thread so the non busy one will catch it first
			retVal &= pThread->invoke();
		}
	}
	return retVal;
}

ThreadPool::~ThreadPool()
{
	cleanup();
}

void ThreadPool::enterLowLatencyMode()
{
	m_lowLatencyMode = true;
	for (unsigned int i = 0 ; i < m_workerThreads.size() ; i++)
	{
			m_workerThreads[i]->enterLowLatencyMode();
	}
}

void ThreadPool::leaveLowLatencyMode()
{
	m_lowLatencyMode = false;
	for (unsigned int i = 0 ; i < m_workerThreads.size() ; i++)
	{
			m_workerThreads[i]->exitLowLatencyMode();
	}
}