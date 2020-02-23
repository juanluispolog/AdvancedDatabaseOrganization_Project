#include "buffer_mgr.h"
#include "storage_mgr.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>





// Global variables
int globalTimestamp = 0;



//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************

//****************************  BUFFER POOL  ****************************

//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************





//****************************************************************************

//***                           INIT BUFFER POOL

//****************************************************************************

// Buffer Manager Interface Pool Handling

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,const int numPages, ReplacementStrategy strategy, void *stratData){

    SM_FileHandle fHandle;
    if(openPageFile(pageFileName, &fHandle) == RC_OK){
        
        bm->pageFile = pageFileName;
        bm->numPages = numPages;
        bm->strategy = strategy;
        
        
        BufferInfo *bufferInfo = (BufferInfo*) malloc(sizeof(BufferInfo));
        
        bufferInfo->numFrames = 0;
        bufferInfo->pageFrame = (PageFrames*) malloc(sizeof(PageFrames) * numPages);
        bufferInfo->readCount = -1;
        bufferInfo->writeCount = 0;
        
        int count = 0;
        while(count < numPages){
            
            bufferInfo->pageFrame[count].dirty = 0;
            bufferInfo->pageFrame[count].fixCount = 0;
            bufferInfo->pageFrame[count].pageFrameNum = 0;
            bufferInfo->pageFrame[count].pageHandle.pageNum = -1;
            bufferInfo->pageFrame[count].pinned = 0;
            bufferInfo->pageFrame[count].timestamp = NULL;
            bufferInfo->pageFrame[count].timestampPage = 0;
            count++;
        }
        bm->mgmtData = (void*) bufferInfo;
        return RC_OK;
    }
    
    RC_message = "NO PAGEFILE NAME IN DISK";
    bm->pageFile = NULL;
    return RC_READ_NON_EXISTING_PAGE;
}



//****************************************************************************
//****************************************************************************

//***                           SHUT DOWN BUFFER POOL

//****************************************************************************


RC shutdownBufferPool(BM_BufferPool *const bm){

    if(bm->pageFile != NULL){

        BufferInfo *bufferInfo = (BufferInfo *) bm->mgmtData;
        PageFrames *pageFrame = (PageFrames*) bufferInfo->pageFrame;

        forceFlushPool(bm);
        
//        int i=0;
//        while(i<(bm->numPages)){
//            if(pageFrame[i].pinned != 0){
//                RC_message = "RC_STILL_PINNED_PAGES";
//                return RC_STILL_PINNED_PAGES;
//            }
//            i++;
//        }
        bm->mgmtData = NULL;
        bufferInfo->numFrames = 0;
        bufferInfo->readCount = 0;
        bufferInfo->writeCount = 0;
            
//        free(bufferInfo);
            
        return RC_OK;
    }
    
    return RC_FILE_NOT_FOUND;
}

//****************************************************************************
//****************************************************************************

//***                           FORCE BUFFER POOL

//****************************************************************************



RC forceFlushPool(BM_BufferPool *const bm){

    if(bm->pageFile == NULL){
        return RC_FILE_NOT_FOUND;
    }
    
    BufferInfo *bufferInfo = (BufferInfo *)bm->mgmtData;
    PageFrames *pageFrame = (PageFrames *) bufferInfo->pageFrame;
    
    int count = 0;
    for(count=0; count<(bm->numPages); count++){
        
        if(pageFrame[count].fixCount == 0){
            
            if(pageFrame[count].dirty == 1){
                
                SM_FileHandle fHandle;
                SM_PageHandle pageHandle = pageFrame[count].pageHandle.data;
                int pageNum = pageFrame[count].pageHandle.pageNum;
                
                if(openPageFile(bm->pageFile, &fHandle)==RC_OK){
                    
                    if(writeBlock(pageNum, &fHandle, pageHandle) == RC_OK){
                        pageFrame[count].dirty = 0;
                        bufferInfo->writeCount++;
                        closePageFile(&fHandle);
                    }
                    else {
                        closePageFile(&fHandle);
                        return RC_WRITE_FAILED;
                    }
                }
            }
        }
    }
    return RC_OK;
}



//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************

//*******************************    PAGES  ****************************

//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************




//****************************************************************************
//****************************************************************************

//***                           1- MARK DIRTY

//****************************************************************************

// Buffer Manager Interface Access Pages
RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){

    BufferInfo *bufferInfo = (BufferInfo*) bm->mgmtData;
    PageFrames *pageFrame = (PageFrames*) bufferInfo->pageFrame;
    
    for(int i = 0; i < bm->numPages; i++){
        if(pageFrame[i].pageHandle.pageNum == page->pageNum){
            pageFrame[i].dirty = 1;
            return RC_OK;
        }
    }
    
    return RC_READ_NON_EXISTING_PAGE;


}

//****************************************************************************
//****************************************************************************

//***                           2- UNPIN PAGE

//****************************************************************************

RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
    
    BufferInfo *bufferInfo = (BufferInfo*) bm->mgmtData;
    PageFrames *pageFrame = (PageFrames*) bufferInfo->pageFrame;
    
    
    for(int i = 0; i < bm->numPages; i++){
        
        if(pageFrame[i].pageHandle.pageNum == page->pageNum){
            
            if(pageFrame[i].fixCount > 1){
                pageFrame[i].fixCount -= 1;
                pageFrame[i].pinned = 1;
            }
            else{
                pageFrame[i].fixCount = 0;
                pageFrame[i].pinned = 0;
            }
            return RC_OK;
        }
    }
    return RC_PAGE_NOT_FOUND;
}

//****************************************************************************
//****************************************************************************

//***                           3- FORCE PAGE

//****************************************************************************

RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){


    SM_FileHandle fHandle;
    BufferInfo *bufferInfo = (BufferInfo*) bm->mgmtData;
    PageFrames *pageFrame = (PageFrames*) bufferInfo->pageFrame;

    
    for(int i = 0; i < bm->numPages; i++){
        
        if(pageFrame[i].pageHandle.pageNum == page->pageNum){
            
            if(openPageFile(bm->pageFile, &fHandle)==RC_OK){
                
                if(writeBlock(page->pageNum, &fHandle, page->data) == RC_OK){
                    bufferInfo->writeCount++;
                    pageFrame[i].dirty = 0;
                    return RC_OK;
                }
            closePageFile(&fHandle);
            }
        }
    }
    return RC_PAGE_NOT_FOUND;
}

//****************************************************************************
//****************************************************************************

//***                           4- PIN PAGE

//****************************************************************************


RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum){

    if(bm->pageFile == NULL){
        return RC_FILE_NOT_FOUND;
    }
    
    BufferInfo *bufferInfo = (BufferInfo*) bm->mgmtData;
    PageFrames *pageFrame = (PageFrames*) bufferInfo->pageFrame;

    //IF BUFFER NOT EMPTY
    if(bufferInfo->numFrames > 0){
        //LOOKING FOR THE PAGE
        int i = 0;
        for(i=0; i<(bm->numPages); i++){
            
            // IF PAGE ALREADY IN BUFFER
            if(pageFrame[i].pageHandle.pageNum == pageNum){
                globalTimestamp++;
                page->pageNum = pageNum;
                page->data = pageFrame[i].pageHandle.data;
                
                pageFrame[i].pinned = 1;
                pageFrame[i].fixCount +=1;
                pageFrame[i].timestamp = time(0);
                pageFrame[i].timestampPage = globalTimestamp;
                
                return RC_OK;
            }
        }
    }
    
    // EMPTY BUFFER OR PAGE NOT IN BUFFER
    
    // Reading page from PageFile (disk)
    SM_FileHandle fHandle;
    SM_PageHandle pageHandle = (SM_PageHandle) malloc(PAGE_SIZE);

    openPageFile(bm->pageFile, &fHandle);
    if(readBlock(pageNum, &fHandle, pageHandle) != RC_OK){
        ensureCapacity(pageNum, &fHandle);
        if(readBlock(pageNum, &fHandle, pageHandle) != RC_OK){
            return RC_READ_NON_EXISTING_PAGE;
        }
    }
    closePageFile(&fHandle);

    // BLOCK READ AND STORED IN pageHandle
    globalTimestamp++;
    bufferInfo->readCount += 1;
    page->data = pageHandle;
    page->pageNum = pageNum;
    
    // IF SPACE IN BUFFER
    if(bufferInfo->numFrames < bm->numPages){
        pageFrame[bufferInfo->numFrames].fixCount += 1;
        pageFrame[bufferInfo->numFrames].pinned = 1;
        pageFrame[bufferInfo->numFrames].pageHandle.pageNum = pageNum;
        pageFrame[bufferInfo->numFrames].pageHandle.data = pageHandle;
        pageFrame[bufferInfo->numFrames].timestamp = time(0);
        pageFrame[bufferInfo->numFrames].timestampPage = globalTimestamp;
        bufferInfo->numFrames += 1;
        return RC_OK;
    }
    
    
    if(bm->strategy==RS_FIFO){
        return FIFO_Strategy(bm,&fHandle,pageHandle,pageNum);
    }
    if(bm->strategy==RS_LRU){
        return LRU_Strategy(bm,&fHandle,pageHandle,pageNum);
    }
    else{
        return RC_WRONG_STRATEGY;
    }

}

//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************

//*******************************    INTERFACE  ****************************

//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************




//****************************************************************************
//****************************************************************************

//***                           1- GET FRAME CONTENTS

//****************************************************************************
PageNumber *getFrameContents (BM_BufferPool *const bm){


    BufferInfo *bufferInfo = (BufferInfo*) bm->mgmtData;
    PageFrames *pageFrame = (PageFrames*) bufferInfo->pageFrame;

    PageNumber *frameContents = (PageNumber*)malloc(sizeof(PageNumber) * bm->numPages);


    for (int i = 0; i < bm->numPages; i++){
        
        frameContents[i] = pageFrame[i].pageHandle.pageNum;
    }
    return frameContents;

}


//****************************************************************************
//****************************************************************************

//***                           2- GET DIRTY FLAGS

//****************************************************************************

bool *getDirtyFlags (BM_BufferPool *const bm){

    bool *dirtyFlags = malloc(bm->numPages * sizeof(dirtyFlags));
    BufferInfo *bufferInfo = (BufferInfo*) bm->mgmtData;
    PageFrames *pageFrame = (PageFrames*) bufferInfo->pageFrame;

    int i = 0;
    for(i = 0; i < bm->numPages; i++){
        if(pageFrame[i].dirty == 1){
            dirtyFlags[i] = true;
        }
        else{
            dirtyFlags[i] = false;
        }
    }
    return dirtyFlags;
}


//****************************************************************************
//****************************************************************************

//***                           3- GET FIX COUNTS

//****************************************************************************

int *getFixCounts (BM_BufferPool *const bm){

    int *fixCounts = (int * )malloc(sizeof(fixCounts) * bm->numPages);
    BufferInfo *bufferInfo = (BufferInfo*) bm->mgmtData;
    PageFrames *pageFrame = (PageFrames*) bufferInfo->pageFrame;


    for(int i = 0; i < bm->numPages; i++){
        fixCounts[i] = pageFrame[i].fixCount;
    }
    return fixCounts;
}


//****************************************************************************
//****************************************************************************

//***                           4- GET NUM READ IO

//****************************************************************************

int getNumReadIO (BM_BufferPool *const bm){
    
    BufferInfo *bufferInfo = (BufferInfo*) bm->mgmtData;
    bufferInfo->readCount += 1;
    return  bufferInfo->readCount;

}



//****************************************************************************
//****************************************************************************

//***                           5- GET NUM WRITE IO

//****************************************************************************

int getNumWriteIO (BM_BufferPool *const bm){

    BufferInfo *bufferInfo = (BufferInfo*) bm->mgmtData;
    return  bufferInfo->writeCount;
}






//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************

//*******************************   STRATEGY  ****************************

//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************






//****************************************************************************
//****************************************************************************

//***                           FIFO STRATEGY

//****************************************************************************



RC FIFO_Strategy(BM_BufferPool *bm, SM_FileHandle *fHandle, SM_PageHandle pageHandle, const PageNumber pageNum) {

    BufferInfo *bufferInfo = (BufferInfo*) bm->mgmtData;
    PageFrames *pageFrame = (PageFrames*) bufferInfo->pageFrame;
    SM_FileHandle fh = *fHandle;

    int index = bufferInfo->readCount % bm->numPages;

    int i = 0;
    for(i = 0; i<bm->numPages; i++){
        if(pageFrame[index].fixCount == 0){
            if(pageFrame[index].dirty == 1){
                
                if(openPageFile(bm->pageFile, fHandle) != RC_OK){
                    closePageFile(fHandle);
                    return RC_WRITE_FAILED;
                }
                else{
                    if(writeBlock(pageFrame[index].pageHandle.pageNum, &fh, pageFrame[index].pageHandle.data) != RC_OK){
                        closePageFile(fHandle);
                        RC_message = "RC_WRITE_FAILED";
                        return RC_WRITE_FAILED;
                    }
                    else{
                        bufferInfo->writeCount += 1;
                        //            free(pageFrame[index].pageHandle.data);
                    }
                }
            }

            pageFrame[index].pinned = 1;
            pageFrame[index].fixCount = 1;
            pageFrame[index].dirty = 0;
            pageFrame[index].pageHandle.pageNum = pageNum;
            pageFrame[index].pageHandle.data = pageHandle;
            return RC_OK;
            
        }
        index++;
        index = (index % bm->numPages == 0) ? 0 : index;
    }

    printf("Imposible to save page file, because all the frames have fix_count>0\n");
    return RC_WRITE_FAILED;


}


//****************************************************************************
//****************************************************************************

//***                          LRU STRATEGY

//****************************************************************************

//RC LRU_Strategy(BM_BufferPool *bm, SM_FileHandle *fHandle, SM_PageHandle pageHandle, const PageNumber pageNum) {
//
//    BufferInfo *bufferInfo = (BufferInfo*) bm->mgmtData;
//    PageFrames *pageFrame = (PageFrames*) bufferInfo->pageFrame;
//
//    int index = -1;
//    time_t t = NULL;
//
//    int i = 0;
//    for(i=0; i < (bm->numPages); i++){
//        if(pageFrame[i].fixCount == 0){
//            if(t == NULL || pageFrame[i].timestamp < t){
//                index = i;
//                t = pageFrame[i].timestamp;
////                continue;
//            }
//        }
//    }
//
//    if(index == -1) {
//        RC_message = "RC_WRITE_FAILED : fixCount > 0 all pages";
//        return RC_WRITE_FAILED;
//    }
//
//    if(pageFrame[index].dirty == 1){
//
//        openPageFile(bm->pageFile, fHandle);
//        RC code = writeBlock(pageFrame[index].pageHandle.pageNum, fHandle, pageHandle);
//
//        if(code == RC_WRITE_FAILED){
//            openPageFile(bm->pageFile, fHandle);
//            code = writeBlock(pageFrame[index].pageHandle.pageNum, fHandle, pageHandle);
//        }
//        if(code == RC_OK){
//            bufferInfo->writeCount++;
//        }
//        closePageFile(fHandle);
//    }
//
//    free(pageFrame[index].pageHandle.data);
//
//    pageFrame[index].pinned = 1;
//    pageFrame[index].fixCount = 1;
//    pageFrame[index].dirty = 0;
//    pageFrame[index].pageHandle.pageNum = pageNum;
//    pageFrame[index].pageHandle.data = fHandle;
//    pageFrame[index].timestamp = time(0);
//    return RC_OK;
//
//}


RC LRU_Strategy(BM_BufferPool *bm, SM_FileHandle *fHandle, SM_PageHandle pageHandle, const PageNumber pageNum) {

    BufferInfo *bufferInfo = (BufferInfo*) bm->mgmtData;
    PageFrames *pageFrame = (PageFrames*) bufferInfo->pageFrame;
    int i, index, leastTimestamp;

    // Interating through all the page frames in the buffer pool.
    for(i = 0; i < (bm)->numPages; i++)
    {
        // Finding page frame whose fixCount = 0 i.e. no client is using that page frame.
        if(pageFrame[i].fixCount == 0)
        {
            index = i;
            leastTimestamp = pageFrame[i].timestampPage;
            break;
        }
    }

    // Finding the page frame having minimum hitNum (i.e. it is the least recently used) page frame
    for(i = index + 1; i < (bm)->numPages; i++)
    {
        if(pageFrame[i].timestampPage < leastTimestamp)
        {
            index = i;
            leastTimestamp = pageFrame[i].timestampPage;
        }
    }

    // If page in memory has been modified (dirtyBit = 1), then write page to disk
    if(pageFrame[index].dirty == 1)
    {
        SM_FileHandle fh;
        openPageFile(bm->pageFile, &fh);
        writeBlock(pageFrame[index].pageHandle.pageNum, &fHandle, pageFrame[index].pageHandle.data);
        
        // Increase the writeCount which records the number of writes done by the buffer manager.
        bufferInfo->writeCount++;
        globalTimestamp++;
    }
    
    pageFrame[index].pageHandle.data = pageHandle;
    pageFrame[index].pageHandle.pageNum = pageNum;
    pageFrame[index].pinned = 1;
    pageFrame[index].fixCount = 1;
    pageFrame[index].dirty = 0;
    pageFrame[index].timestampPage = globalTimestamp;
    
    return RC_OK;
}
