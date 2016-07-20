//////////////////////////////////////////////////////////////////
// Task.hpp														//
// Hulk renderer - Create by Tzachi Cohen 2013					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#ifndef TASK_HPP
#define TASK_HPP

#include "TaskSet.hpp"

namespace Concurrency {
class ThreadPool;

class Task
{
public:
	Task(TaskSet & taskSet) : m_taskSet(taskSet),m_pThreadPool(NULL) {}
	virtual ~Task() {};
	bool Initialize(TaskSet * taskSet);
	
	inline bool execute();

	TaskSet & m_taskSet;
	ThreadPool * m_pThreadPool;
	
	//this is the execution function to be implemented 
	//by the derivative class
	virtual bool executeImp() = 0;
protected:
	

	///////////////////////
	// private variables //
	///////////////////////
	
	
};

bool Task::execute()
{
	bool retVal  = executeImp();

	m_taskSet.decrenetTask();

	return true;
}

} //end of namespace concurrency

#endif