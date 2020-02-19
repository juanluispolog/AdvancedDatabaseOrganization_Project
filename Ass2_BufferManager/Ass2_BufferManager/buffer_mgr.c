//
//  buffer_mgr.c
//  Ass2_BufferManager
//
//  Created by Juan Luis Polo and Eduardo Moreno on Spring 2020.
//  Copyright © 2020 Juan Luis Polo and Eduardo Moreno. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"


//
//
//
/* Buffer Manager Interface Pool Handling */
//
//
//

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
                  const int numPages, ReplacementStrategy strategy,
                  void *stratData){
    int i = 0;
    
    SM_FileHandle *fHandle = (SM_FileHandle *)malloc(sizeof(SM_FileHandle));
    FIFO_Manager *fifoManager = (FIFO_Manager *)malloc(sizeof(FIFO_Manager));
    LRU_Manager *lruManager = (LRU_Manager *)malloc(sizeof(LRU_Manager));

    if(openPageFile(pageFileName, fHandle) == RC_OK){
        (*bm).pageFile = pageFileName;
        (*bm).numPages = numPages;
        (*bm).strategy = strategy;
    
        //Initilizing pages to Null values
        PageFrame *pageFrame = (PageFrame *)malloc(numPages * sizeof(PageFrame));
        int count = 0;
        while(count < numPages){
            pageFrame[count].dirty = 0;
            pageFrame[count].pinned = 0;
            pageFrame[count].fixCount = 0;
            pageFrame[count].pageFrameNum = count;
            pageFrame[count].pageHandle->pageNum = NO_PAGE;
            pageFrame[count].pageHandle->data = NULL;
            count++;
        }
        
        
        
        if (strategy == RS_FIFO){
            //Bookeeping page info
            (*fifoManager).fifoPageFrames = pageFrame;
            
            int *fifoID = (int *)malloc(numPages * sizeof(int));
            int *fifoQueue = (int *)malloc(numPages * sizeof(int));
            
            for(i=0; i<numPages; i++){
                fifoID[count] = -1;
                fifoQueue[count] = -1;
            }
            (*fifoManager).PageID = fifoID;
            (*fifoManager).fifoQueue = fifoQueue;
            
            (*bm).mgmtData = fifoManager;
        }
        
        if (strategy == RS_LRU){
            (*lruManager).lruPageFrames = pageFrame;
            
            int *lruID = (int *)malloc(numPages * sizeof(int));
            int *lruQueue = (int *)malloc(numPages * sizeof(int));
            
            for(i=0; i<numPages; i++){
                lruID[count] = -1;
                lruQueue[count] = -1;
            }
            (*lruManager).PageID = lruID;
            (*lruManager).lruQueue = lruQueue;
            
            (*bm).mgmtData = lruManager;
        }
        
        free(fHandle);
        RC_message = "BUFFER_POOL_INIT";
        return RC_OK;
    }
    
    RC_message = "BUFFER_POOL_NOT_INIT";
    return RC_FILE_NOT_FOUND;
    
}


RC forceFlushPool(BM_BufferPool *const bm){
    
    PageFrame *pageFrame = (*bm).mgmtData;
    
    int count = 0;
    
    while(count<(*bm).numPages){
        
        if((*pageFrame).fixCount == 0){
            
            if((*pageFrame).dirty == 1){
                
                SM_FileHandle *fHandle = malloc(sizeof(SM_FileHandle));
                if(openPageFile((*bm).pageFile, fHandle)==RC_OK){
                    
                    writeBlock(pageFrame[count].pageHandle->pageNum, fHandle, pageFrame[count].pageHandle->data);
                    pageFrame[count].dirty = 0;
                    count++;
                }
                free(fHandle);
                
            }
        }
    }
    
    free(pageFrame);
    return RC_OK;
}


RC shutdownBufferPool(BM_BufferPool *const bm){
    
    PageFrame *pageFrame = (*bm).mgmtData;
    
    //Using function forceFlushPool() for write to disk all dirty pages in buffer befor erasing the buffer
    forceFlushPool(bm);
    
    //Check if there is any page that is being pinned by any user(s)
    int i = 0;
    while (i < (*bm).numPages) {
        if ((*pageFrame).pinned == 1) {
            RC_message = "RC PINNED PAGE";
            return RC_IM_KEY_NOT_FOUND;
        }
    }
    
    //If there is no used page frames, then erase all variables related too buffer pool
    free(pageFrame);
    free((*bm).mgmtData);
    
    
    return RC_OK;
    
}










RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){
    
    PageFrame *pageFrames = (PageFrame *)malloc((*bm).numPages * sizeof(PageFrame));

    if ((*bm).strategy == RS_FIFO){
        //        PageFrame *pageFrame = (PageFrame *)malloc((*bm).numPages * sizeof(PageFrame));
        //        pageFrame = (*bm).mgmtData->fifoPageFrames;
        FIFO_Manager *fifoManager = (*bm).mgmtData;
        pageFrames = fifoManager->fifoPageFrames;
    }
        
        
    if ((*bm).strategy == RS_LRU){
        LRU_Manager *lruManager = (*bm).mgmtData;
        pageFrames = lruManager->lruPageFrames;
    }
    
    int i=0;
    PageNumber searchedPage = page->pageNum;
    
    for (i=0; i<(*bm).numPages; i++) {
        if(pageFrames[i].pageHandle->pageNum == searchedPage){
            pageFrames[i].dirty = 1;
        }
    }
    
    return RC_OK;
    
// HAY QUE HACER FREE DE LOS PUNTEROS
    
}







RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){
    
    
    PageFrame *pageFrames = (PageFrame *)malloc((*bm).numPages * sizeof(PageFrame));
    SM_FileHandle *fHandle = malloc(sizeof(SM_FileHandle));
    
    if ((*bm).strategy == RS_FIFO){
        FIFO_Manager *fifoManager = (*bm).mgmtData;
        pageFrames = fifoManager->fifoPageFrames;
    }
        
    if ((*bm).strategy == RS_LRU){
        LRU_Manager *lruManager = (*bm).mgmtData;
        pageFrames = lruManager->lruPageFrames;
    }
    
    int i=0;
    PageNumber searchedPage = page->pageNum;
    
    if(openPageFile((*bm).pageFile, fHandle) == RC_OK){
        for (i=0; i<(*bm).numPages; i++) {
            if(pageFrames[i].pageHandle->pageNum == searchedPage){
                    writeBlock(pageFrames[i].pageHandle->pageNum, fHandle, pageFrames[i].pageHandle->data);
            }
        }
    }
    
    free(pageFrames);
    return RC_OK;
    
}
