//////////////////////////////////////////////////////////////////
// RingBuffer.hpp												//
// Hulk renderer - Create by Tzachi Cohen 2013					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////

#ifndef RING_BUFFER_HPP
#define RING_BUFFER_HPP

#include <math.h>
#include <Windows.h>

namespace Concurrency {
// @brief Block-free ring buffer implemenation
// @brief THE RING BUFFER SUPPORTS MULTIPLE CONSUMERS AND A MULTIPLE PRODUCERS.
// @tparam T Object-type to be saved within the ring buffer.
template <typename T>
class RingBuffer
{
public:
	///////////////////////////////////////
	// public initialization and cleanup //
	///////////////////////////////////////
    RingBuffer();
	~RingBuffer() { cleanup();DeleteCriticalSection(&m_producerCS);}
	bool initialize(unsigned short ringBufferSize);
	//////////////////////
	// public interface //
	///////////////////////
   
	bool getNext(T & obj);
	bool insert(const T & obj);
private:
	struct ABACounter{
		unsigned short tranactionId;
		unsigned short consumerIndex;
	};

	union Consumer {
		ABACounter abaCounter;
		volatile long interlockedVar;
	};

	bool canInsert();

	template <typename T2> T2 incrementIndex(T2  index) {
	index++;
	if (index == c_ringBufferSize)
		index = 0;
	return index;
	}
	////////////////////////////////////
    // read only cache line for       //
	// producer and consumer threads  //
	////////////////////////////////////
	T * m_ringBuffer;
	unsigned short c_ringBufferSize;

	 char m_cachePad1[128];
	/////////////////////////////////////
    // read/write cache line for       //
	// producer thread                 //
	/////////////////////////////////////
	//! producer is an index in the ring buffer array
	volatile long m_producer;
	//! caches the amount of free space in the buffer. reduces cache misses
	//! by reducing access to 'm_consumer' from producer thread 
	long m_freeSpace;

	CRITICAL_SECTION m_producerCS;
	char m_cachePad2[128];
	/////////////////////////////////////
    // read/write cache line for       //
	// consumer threads                //
	/////////////////////////////////////
	volatile Consumer m_consumer;

	void cleanup();
    // do not allow copying
    RingBuffer(const RingBuffer&);
    RingBuffer& operator=(const RingBuffer&);
};

template <typename T>
RingBuffer<T>::RingBuffer()
:m_ringBuffer(NULL) 
,m_producer(0)
,c_ringBufferSize(0)
,m_freeSpace(0)
{ 
	m_consumer.interlockedVar = 0;
	memset(m_cachePad1,0,sizeof(m_cachePad1));
	memset(m_cachePad2,0,sizeof(m_cachePad1));
	InitializeCriticalSection(&m_producerCS);
}

template <typename T>
bool RingBuffer<T>::initialize(unsigned short  ringBufferSize)
{
	bool retVal = false;

	cleanup();
	m_ringBuffer = new T [ringBufferSize];
	if (m_ringBuffer)
	{
		c_ringBufferSize = ringBufferSize;
		retVal = true;
	}
	
	return retVal;
}

template <typename T>
void RingBuffer<T>::cleanup()
{
	if (m_ringBuffer)
	{
		delete [] m_ringBuffer;
		m_ringBuffer = NULL;
	}
	m_producer = 0;
	m_consumer.interlockedVar = 0;
	m_freeSpace = 0;
	c_ringBufferSize = 0;
	
}

template <typename T>
bool RingBuffer<T>::insert(const T & obj)
{
	bool retVal = false;
	EnterCriticalSection(&m_producerCS);
	if (canInsert())
	{
		m_ringBuffer[m_producer] = obj;
		m_producer = incrementIndex(m_producer);
		retVal = true;
	}
	LeaveCriticalSection(&m_producerCS);
	return retVal;
}

template <typename T>
bool RingBuffer<T>::getNext( T & obj)
{
	//cache the producer variable on the stack
	Consumer consumer;
   consumer.interlockedVar = m_consumer.interlockedVar;
   int producer = m_producer;
  
   //while the buffer is not empty
   while (producer != consumer.abaCounter.consumerIndex)
   {
	    obj = m_ringBuffer[consumer.abaCounter.consumerIndex];
		Consumer newConsumer={0};
		newConsumer.abaCounter.consumerIndex = incrementIndex(consumer.abaCounter.consumerIndex);
		newConsumer.abaCounter.tranactionId = consumer.abaCounter.tranactionId+1;

		if (consumer.interlockedVar == InterlockedCompareExchange(&(m_consumer.interlockedVar),
			newConsumer.interlockedVar,consumer.interlockedVar))
		{
		   return true;
		}
		consumer.interlockedVar = m_consumer.interlockedVar;
		producer = m_producer;
		
   }
   return false;
}

template <typename T>
bool  RingBuffer<T>::canInsert()
{
	if (m_freeSpace > 1)
	{
		m_freeSpace--;
		return true;
	}
	//cache the volatile variable on the stack;
	long consumer = m_consumer.abaCounter.consumerIndex;

	//there will alway be one unutilized cell in the array
	//to disinguish between the case it is completely full and completely empty
	m_freeSpace = consumer - m_producer - 1  ;
	if ( m_freeSpace <= -1 )
	{
		m_freeSpace = c_ringBufferSize + m_freeSpace;
	}
	return (m_freeSpace > 0) ;
}

} // end of namespace Concurrency
#endif // RING_BUFFER_HPP