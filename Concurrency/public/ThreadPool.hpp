//////////////////////////////////////////////////////////////////
// ThreadPool.hpp												//
// Hulk renderer - Create by Tzachi Cohen 2013					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include <vector>

#include "WorkerThread.hpp"
#include "RingBuffer.hpp"
#include "Task.hpp"

namespace Concurrency {

class ThreadPool
{
public:
	ThreadPool():m_lowLatencyMode(false) {}
	~ThreadPool();

	bool initialize();
	bool cleanup();

	bool enqueueTask(Task * pTask);
	void enterLowLatencyMode();
	void leaveLowLatencyMode();
private:
	/////////////////////
	// private members //
	/////////////////////
	std::vector<WorkerThread*> m_workerThreads;
	RingBuffer<Task*> m_ringBuffer;
	bool m_lowLatencyMode;
};

} //end of namespace concurrency
#endif