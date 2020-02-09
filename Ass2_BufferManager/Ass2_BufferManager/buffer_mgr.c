//
//  buffer_mgr.c
//  Ass2_BufferManager
//
//  Created by Juan Luis Polo and Eduardo Moreno on Spring 2020.
//  Copyright © 2020 Juan Luis Polo and Eduardo Moreno. All rights reserved.
//

#include <stdio.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "storage_mgr.c"


#define PAGE_H
#include <stdbool.h>

typedef struct Page {
    //1 means dirty, 0 means clean
    bool isDirty;
    bool isPinned;
    int fixCount;
    int pageNumber;
    char *pageValue;
} Page;


// NOTES
// INIT -> ALL PAGE FRAMES MUST BE EMPTYS

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
const int numPages, ReplacementStrategy strategy, void *stratData){
        
    SM_FileHandle *fHandle = malloc(1 * sizeof(SM_FileHandle));

    if(openPageFile(pageFileName, fHandle)==RC_OK){
    
        //PageFile
        (*bm).pageFile = pageFileName;
    
        //Number of Pages
        (*bm).numPages = numPages;
    
        //Replacement strategy
        (*bm).strategy = strategy;
    
        //Bookeeping info
        (*bm).mgmtData = fHandle;
        
        RC_message = "BUFFER_POOL_INIT";
        return RC_OK;
    }
    
    RC_message = "BUFFER_POOL_NOT_INIT";
    return RC_FILE_NOT_FOUND;
    
}

RC shutdownBufferPool(BM_BufferPool *const bm){
    
    if(forceFlushPool(bm) == RC_OK){
        
        
        
    }
    
    
    //
}

RC forceFlushPool(BM_BufferPool *const bm){
    
    //(*bm).pageFile
}

//extern RC openPageFile (char *fileName, SM_FileHandle *fHandle);


//typedef struct BM_BufferPool {
    //char *pageFile;
    //int numPages;
    //ReplacementStrategy strategy;
    //void *mgmtData; // use this one to store the bookkeeping info your buffer
    // manager needs for a buffer pool
//} BM_BufferPool;




