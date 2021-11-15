
#ifndef _MEMORYPOOL_H_
#define _MEMORYPOOL_H_

#include "common.h"

typedef struct {
        uint8_t* buffer;
        uint32_t size;
} MemoryBlock;

#if 0
typedef std::map<uint8_t*,uint8_t*> MemoryMap;
typedef std::list<uint8_t*>               MemoryList;
#else
typedef std::map<uint8_t*, MemoryBlock> MemoryMap;
typedef std::list<MemoryBlock> MemoryList;
#endif

class CMemoryPool {
    public:
        CMemoryPool(uint32_t chunksNum, uint32_t chunkSize = 0);

        ~CMemoryPool();

        uint8_t* GetMemory(uint32_t size);

        bool FreeMemory(uint8_t* buffer);

        bool FreeAllMemory();

    private:
        uint8_t* GetUnusedMemory();

    private:
        uint8_t* m_memory;
        uint32_t m_chunkSize;
        uint32_t m_chunksNum;
        MemoryMap m_memoryMap;
        MemoryMap m_extendMemoryMap;
        MemoryList m_freeMemList;
        PMutex m_mutex;
        bool m_bAutoSizeMode;
};

#endif //_MEMORYPOOL_H_
