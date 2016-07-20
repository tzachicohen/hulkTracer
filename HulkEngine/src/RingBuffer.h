

#pragma once
#include "util.hpp"

class RingBuffer
{
public:
    RingBuffer(uint bufferSize = 1024*1024)
        :
        m_bufferSize(bufferSize),
        m_consumer(0),
        m_producer(0){}

    uint GetContiguousFreeSpace()
    {
        uint size1, size2;
        getContiguousFreeSpaces(size1, size2); 
        return MAX(size1, size2);
    }
    bool contiguousInsert(uint size, OUT uint & offest)
    {
        bool retVal = false;
        uint size1, size2;
        getContiguousFreeSpaces(size1, size2);
        if (size <= size1)
        {
            offest = m_producer;
            m_producer += size;
            retVal = true;
        }
        else if (size <= size2)
        {
            offest = m_producer;
            m_producer = size;
            retVal = true;
        }
        return retVal;
    }
    uint getOccuppiedSpace()
    {
        uint result = m_producer - m_consumer  ;
        if (result > m_producer)
        {
            result += (m_bufferSize -1);
        }
        return result;
    }
    void free(uint size)
    {
        m_consumer += size;
        //wrap around
        if (m_consumer > m_bufferSize)
        {
            m_consumer = size;
        }
    }
    void reset(uint newBufferSize)
    {
        m_consumer = m_producer = 0;
        m_bufferSize = newBufferSize;
    }
    uint getBufferSize() { return m_bufferSize; }
private:
    void getContiguousFreeSpaces(uint & size1, uint & size2)
    {
        if (m_producer >= m_consumer)
        {
            size1 = m_bufferSize - m_producer - 1;
            size2 = m_consumer - 1;
        }
        if (m_producer < m_consumer)
        {
            size1 = m_consumer - m_producer - 1;
            size2 = 0;
        }
    }

    //when producer equals consumer 
    uint m_consumer;
    uint m_producer;
    uint m_bufferSize;
};