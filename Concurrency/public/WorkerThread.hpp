//////////////////////////////////////////////////////////////////
// WorkerThread.hpp												//
// Hulk renderer - Create by Tzachi Cohen 2013					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#ifndef WORKER_THREAD_HPP
#define WORKER_THREAD_HPP

#include "RingBuffer.hpp"
#include "Task.hpp"

namespace Concurrency {

class WorkerThread
{
public:
	enum ThreadStatus {
		e_running = 0,
		e_lowLatencyMode = 1,
		e_stopping = 2,
		e_done = 3
	};

	bool initialize(RingBuffer<Task*> * ring );
	bool invoke(); 
	bool terminate();
	void enterLowLatencyMode();
	void exitLowLatencyMode();
	WorkerThread():m_pThreadAParams(NULL) {}
	~WorkerThread();

private:
	struct ThreadParams {
		RingBuffer<Task*> * pRingBuffer;
		HANDLE waitEvent;
		volatile ThreadStatus status;
	};
	////////////////////////////
	// static thread function //
	////////////////////////////
	static DWORD WINAPI MyThreadFunction( LPVOID lpParam );

	/////////////////////
	// private members //
	/////////////////////

	ThreadParams * m_pThreadAParams;
};

} //end of namespace concurrency
#endif