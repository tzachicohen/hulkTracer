//////////////////////////////////////////////////////////////////
// TaskSet.hpp													//
// Hulk renderer - Create by Tzachi Cohen 2013					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////
#ifndef TASK_SET_THREAD_HPP
#define TASK_SET_THREAD_HPP

#include <Windows.h>

namespace Concurrency {

class TaskSet
{
public:
	TaskSet() : m_event(NULL),m_refCount(0) {m_event = CreateEvent(NULL,FALSE,FALSE,NULL);}
	~TaskSet() { CloseHandle(m_event);}
	
	inline DWORD waitFortaskSet(); 
	inline void incremetTask();
	inline void decrenetTask();
private:
	HANDLE m_event;
	volatile unsigned long m_refCount;
};



DWORD TaskSet::waitFortaskSet()
{
	return WaitForSingleObject(m_event,INFINITE);
}

void TaskSet::incremetTask()
{
	InterlockedIncrement(&m_refCount);
}

void TaskSet::decrenetTask()
{
	unsigned long refCount = InterlockedDecrement(&m_refCount);
	if (0 == refCount) {
		SetEvent(m_event);
	}
}

} // end of namespace concurrency
#endif