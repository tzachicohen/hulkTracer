//////////////////////////////////////////////////////////////////
// MemoryPool.hpp												//
// Hulk renderer - Create by Tzachi Cohen 2013					//
// all rights reserved											//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////
#ifndef MEMORY_POOL_HPP
#define MEMORY_POOL_HPP
#include <vector>

#include <Windows.h>


namespace Concurrency {
//this is a thread safe memory pool
template<class T>
class MemoryPool
{
public:
	MemoryPool();
	~MemoryPool();

	inline bool initialize(size_t size = c_defaultSize,size_t blockSize = c_defaultblockSize,bool DynamicGrow= true);
	inline void deInitialize();

	inline T * allocate();
	inline void free(T * );
	inline size_t getPoolSize() const {return m_poolSize;}
private:
	static const unsigned int c_defaultPoolSize = 4096;
    static const unsigned int c_defaultblockSize = 512;
    void insertNewBlock();
    struct FreeBlock
    {
        T object;
        FreeBlock * nextSlot;
    };

	//pointer to the head of the list
    FreeBlock* m_pListHead;
    std::vector<FreeBlock*>     m_blocks;
	size_t         m_poolSize;
    size_t         m_blockSize;
    CRITICAL_SECTION m_cs;
	bool m_dynamicGrow;
};

template<class T>
MemoryPool<T>::MemoryPool()
	:m_pListHead(NULL)
	,m_poolSize(0)
{
    InitializeCriticalSection(&m_cs);
}

template<class T>
MemoryPool<T>::~MemoryPool()
{
	deInitialize();
    DeleteCriticalSection(&m_cs);
}

template<class T>
T *  MemoryPool<T>::allocate()
{
    T* retVal = NULL;
	EnterCriticalSection(&m_cs);
    if (m_dynamicGrow && NULL == m_pListHead ) {
        insertNewBlock();
    }
    //we may run out of memory completely;
    if (m_pListHead) {
        retVal = & (m_pListHead->object);
        m_pListHead = m_pListHead->nextSlot;
    }
    LeaveCriticalSection(&m_cs);
	return retVal;
}

template<class T>
void MemoryPool<T>::free(T * pMem)
{
	if (NULL == pMem)
		return ;

	EnterCriticalSection(&m_cs);
	FreeBlock * pNewBlock = reinterpret_cast<FreeBlock*> (pMem);
    pNewBlock->nextSlot = m_pListHead;
    m_pListHead = pNewBlock;
	LeaveCriticalSection(&m_cs);
}
template<class T>
void MemoryPool<T>::insertNewBlock()
{
     FreeBlock *pBlock = new FreeBlock[m_blockSize];
     if (pBlock) {
         pBlock[m_blockSize-1].nextSlot  = m_pListHead;
         for ( size_t i = 0 ; i < m_blockSize-1 ;i++) {
              pBlock[i].nextSlot =  & (pBlock[i+1]);
         }
         m_poolSize += m_blockSize;
         m_blocks.push_back(pBlock);
         m_pListHead = pBlock;
     }
}

template<class T>
bool MemoryPool<T>::initialize(size_t poolSize,size_t blockSize,bool dynamicGrow)
{
	deInitialize();

	bool retVal = false;
    m_blockSize = blockSize;
	m_poolSize = 0 ;
	m_dynamicGrow = dynamicGrow;
    while (m_poolSize < poolSize)
    {
        insertNewBlock();
    }
   
	return retVal;
}

template<class T>
void MemoryPool<T>::deInitialize()
{
	for (unsigned int i = 0 ; i < m_blocks.size() ; i++) {
        delete [] m_blocks[i];
    }
    m_blocks.clear();
    m_poolSize = 0;
    m_pListHead = NULL;
}

} // end of namespace Concurrency

#endif



