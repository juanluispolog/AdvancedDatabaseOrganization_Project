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
        
    SM_FileHandle *fHandle = malloc(1 * sizeof(SM_FileHandle));

    if(openPageFile(pageFileName, fHandle) == RC_OK){
        (*bm).pageFile = pageFileName;
        (*bm).numPages = numPages;
        (*bm).strategy = strategy;
    
        //Initilizing pages to Null values
        PageInfo *pageFrame = malloc(numPages * sizeof(PageInfo));
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

        //Bookeeping page info
        (*bm).mgmtData = pageFrame;
        
        free(fHandle);
        RC_message = "BUFFER_POOL_INIT";
        return RC_OK;
    }
    
    RC_message = "BUFFER_POOL_NOT_INIT";
    return RC_FILE_NOT_FOUND;
    
}


RC forceFlushPool(BM_BufferPool *const bm){
    
    PageInfo *pageFrame = (*bm).mgmtData;
    
    int count = 0;
    
    while(count<(*bm).numPages){
        
        if((*pageFrame).fixCount == 0){
            
            if((*pageFrame).dirty == 1){
                
                SM_FileHandle *fHandle = malloc(1 * sizeof(SM_FileHandle));
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
    
    PageInfo *pageFrame = (*bm).mgmtData;
    
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
