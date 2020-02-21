//
//  buffer_mgr.c
//  Ass2_BufferManager
//
//  Created by Juan Luis Polo and Eduardo Moreno on Spring 2020.
//  Copyright © 2020 Juan Luis Polo and Eduardo Moreno. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
    
    SM_FileHandle *fHandle = NULL;
    FIFO_Manager *fifoManager = (FIFO_Manager *)malloc(sizeof(FIFO_Manager));
    LRU_Manager *lruManager = (LRU_Manager *)malloc(sizeof(LRU_Manager));

    if(openPageFile(pageFileName, fHandle) == RC_OK){
        bm->pageFile = pageFileName;
        bm->numPages = numPages;
        bm->strategy = strategy;
    
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
            fifoManager->fifoPageFrames = pageFrame;
            
            int *llenos = (int *)malloc(numPages * sizeof(int));
            int *vacios = (int *)malloc(numPages * sizeof(int));
            
            fifoManager->readCount = 0;
            fifoManager->writeCount = 0;
            
            for(i=0; i<numPages; i++){
                llenos[count] = -1;
                vacios[count] = i;
            }
            fifoManager->llenos = llenos;
            fifoManager->vacios = vacios;
            
            bm->mgmtData = fifoManager;
        }
        
        if (strategy == RS_LRU){
            lruManager->lruPageFrames = pageFrame;
            
            int *index = (int *)malloc(numPages * sizeof(int));
//            int *lruQueue = (int *)malloc(numPages * sizeof(int));
            
            lruManager->readCount = 0;
            lruManager->writeCount = 0;
            
            for(i=0; i<numPages; i++){
                index[count] = -1;
//                lruQueue[count] = -1;
            }
            lruManager->index = index;
//            (*lruManager).lruQueue = lruQueue;
            
            bm->mgmtData = lruManager;
        }
        
        free(fHandle);
        RC_message = "BUFFER_POOL_INIT";
        return RC_OK;
    }
    
    RC_message = "BUFFER_POOL_NOT_INIT";
    return RC_FILE_NOT_FOUND;
    
}


RC forceFlushPool(BM_BufferPool *const bm){
//
//    PageFrame *pageFrame = (*bm).mgmtData;
//
    FIFO_Manager *fifoManager = (FIFO_Manager *)bm->mgmtData;
    PageFrame *pageFrames = (PageFrame *) fifoManager->fifoPageFrames;
    
    int count = 0;
    while(count<(bm->numPages)){
        
        if(pageFrames[count].fixCount == 0){
            
            if(pageFrames[count].dirty == 1){
                
                forcePage(bm, pageFrames[count].pageHandle);
                
//                SM_FileHandle *fHandle = malloc(sizeof(SM_FileHandle));
//                if(openPageFile(bm->pageFile, fHandle)==RC_OK){
//
//                    writeBlock(pageFrames[count].pageHandle->pageNum, fHandle, pageFrames[count].pageHandle->data);
//                    pageFrames[count].dirty = 0;
//                    count++;
//                }
//                free(fHandle);
                
            }
        }
    }
    
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



//
//
//
/* Buffer Manager Interface Access Pages */
//
//
//

RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page){
    
    PageFrame *pageFrames = NULL;

    if ((*bm).strategy == RS_FIFO){
        FIFO_Manager *fifoManager = bm->mgmtData;
        pageFrames = fifoManager->fifoPageFrames;
    }
        
        
    if ((*bm).strategy == RS_LRU){
        LRU_Manager *lruManager = bm->mgmtData;
        pageFrames = lruManager->lruPageFrames;
    }
    
    int i=0;
    PageNumber searchedPage = page->pageNum;
    
    for (i=0; i<(bm->numPages); i++) {
        if(pageFrames[i].pageHandle->pageNum == searchedPage){
            pageFrames[i].dirty = 1;
        }
    }
    
    return RC_OK;
}


RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){
    
    PageFrame *pageFrames = NULL;
    SM_FileHandle *fHandle = NULL;
    FIFO_Manager *fifoManager = NULL;
    LRU_Manager *lruManager = NULL;
    
    if ((*bm).strategy == RS_FIFO){
        fifoManager = (*bm).mgmtData;
        pageFrames = fifoManager->fifoPageFrames;
    }
        
    if ((*bm).strategy == RS_LRU){
        lruManager = (*bm).mgmtData;
        pageFrames = lruManager->lruPageFrames;
    }
    
    int i=0;
    PageNumber searchedPage = page->pageNum;
    
    if(openPageFile((*bm).pageFile, fHandle) == RC_OK){
        for (i=0; i<(*bm).numPages; i++) {
            if(pageFrames[i].pageHandle->pageNum == searchedPage){
                if(writeBlock(pageFrames[i].pageHandle->pageNum, fHandle, pageFrames[i].pageHandle->data) == RC_OK){
                    pageFrames[i].dirty = 0;
                    if ((*bm).strategy == RS_FIFO){
                        fifoManager->writeCount++;
                    }
                        
                    if ((*bm).strategy == RS_LRU){
                        lruManager->writeCount++;
                    }
                }
            }
        }
        closePageFile(fHandle);
    }
    
    return RC_OK;
    
}




RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page){
    
    if (bm->strategy == RS_FIFO) {
        unpinPageFIFO(bm, page);
    }
    
    if (bm->strategy == RS_LRU) {
        LRU_Manager *lruManager = (LRU_Manager *)bm->mgmtData;
//        int *index = (int *)lruManager->index;
    }
    
    return 0;
}




RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page,
            const PageNumber pageNum){
   
    if (bm->strategy == RS_FIFO) {
        pinPageFIFO(bm, page, pageNum);
    }
    
    if (bm->strategy == RS_LRU) {
        pinPageLRU(bm, page, pageNum);
    }
    
//    // Pegamos la copia del llenos modificada en el struct fifoManager
//    fifoManager->llenos = llenos;
//    // Pegamos la copia del vacios modificada en el struct fifoManager
//    fifoManager->vacios = vacios;
//    // Pegamos la copia local del pageFrames modificada en el struct fifoManager
//    fifoManager->fifoPageFrames = pageFrames;
//    // Como fifoManager es una copia, volvemos a copiarla en el struct bm
//    bm->mgmtData = fifoManager;
    
    return 0;
}



//
//
//
/* Statistic Interface */
//
//
//

PageNumber *getFrameContents (BM_BufferPool *const bm){
    
    PageFrame *pageFrames = NULL;
    
    if ((*bm).strategy == RS_FIFO){
        FIFO_Manager *fifoManager = (*bm).mgmtData;
        pageFrames = fifoManager->fifoPageFrames;
    }
        
    if ((*bm).strategy == RS_LRU){
        LRU_Manager *lruManager = (*bm).mgmtData;
        pageFrames = lruManager->lruPageFrames;
    }
    
    int *frameContents = (int *)malloc((bm->numPages)*sizeof(int));
    int i = 0;
    for (i=0; i<(bm->numPages); i++) {
        frameContents[i] = pageFrames[i].pageHandle->pageNum;
    }
    
    return frameContents;
}


bool *getDirtyFlags (BM_BufferPool *const bm){
    
    PageFrame *pageFrames = NULL;
    
    if ((*bm).strategy == RS_FIFO){
        FIFO_Manager *fifoManager = (*bm).mgmtData;
        pageFrames = fifoManager->fifoPageFrames;
    }
        
    if ((*bm).strategy == RS_LRU){
        LRU_Manager *lruManager = (*bm).mgmtData;
        pageFrames = lruManager->lruPageFrames;
    }
    
    bool *dirtyFlags = (bool *)malloc((bm->numPages)*sizeof(bool));
    int i = 0;
    for (i=0; i<(bm->numPages); i++) {
        if (pageFrames[i].dirty == 1) {
            dirtyFlags[i] = true;
        }
        else{
            dirtyFlags[i] = false;
        }
        
    }
    
    return dirtyFlags;
}




int *getFixCounts (BM_BufferPool *const bm){
    PageFrame *pageFrames = NULL;
    
    if ((*bm).strategy == RS_FIFO){
        FIFO_Manager *fifoManager = (*bm).mgmtData;
        pageFrames = fifoManager->fifoPageFrames;
    }
        
    if ((*bm).strategy == RS_LRU){
        LRU_Manager *lruManager = (*bm).mgmtData;
        pageFrames = lruManager->lruPageFrames;
    }
    
    int *fixCounts = (int *)malloc((bm->numPages)*sizeof(int));
    int i = 0;
    for (i=0; i<(bm->numPages); i++) {
        fixCounts[i] = pageFrames[i].pageHandle->pageNum;
        if (fixCounts[i] == -1) {
            fixCounts[i] = 0;
        }
        else fixCounts[i] = pageFrames[i].fixCount;
    }
    
    return fixCounts;
}



int getNumReadIO (BM_BufferPool *const bm){
    
    int readCounts = 0;
    
    if ((*bm).strategy == RS_FIFO){
        FIFO_Manager *fifoManager = (*bm).mgmtData;
        readCounts = fifoManager->readCount;
    }
        
    if ((*bm).strategy == RS_LRU){
        LRU_Manager *lruManager = (*bm).mgmtData;
        readCounts = lruManager->readCount;
    }
    
    return readCounts;
}



int getNumWriteIO (BM_BufferPool *const bm){
    int writeCounts = 0;
    
    if ((*bm).strategy == RS_FIFO){
        FIFO_Manager *fifoManager = (*bm).mgmtData;
        writeCounts = fifoManager->writeCount;
    }
        
    if ((*bm).strategy == RS_LRU){
        LRU_Manager *lruManager = (*bm).mgmtData;
        writeCounts = lruManager->writeCount;
    }
    
    return writeCounts;
}









//
//
//
/* Other Needed Functions*/
//
//
//


int searchPageFramePosition(BM_BufferPool *const bm, const PageNumber pageNum){

    FIFO_Manager *fifoManager = (FIFO_Manager *)bm->mgmtData;
    PageFrame *pageFrames = (PageFrame *) fifoManager->fifoPageFrames;
    
    int index=0;
    while(pageNum != pageFrames[index].pageHandle->pageNum){
        index++;
        if (index >= bm->numPages) {
            return NO_PAGE;
        }
    }
    return index;
}




RC pinPageFIFO (BM_BufferPool *const bm, BM_PageHandle *const page,
                const PageNumber pageNum){
    
    SM_FileHandle *fHandle = NULL;
    FIFO_Manager *fifoManager = (FIFO_Manager *)bm->mgmtData;
    PageFrame *pageFrames = (PageFrame *) fifoManager->fifoPageFrames;
    int *llenos = (int *)fifoManager->llenos;
    int *vacios = (int *)fifoManager->vacios;
    
//    // buscar la pagina en el buffer a ver si esta ya pinneada
//    int i = 0, pinned = 0;
//    for (i=0; i<(bm->numPages); i++) {
//        if(pageFrames[i].pageHandle->pageNum == pageNum){
//            pinned = 1;
//            break;
//        }
//    }
    
    // busco en el pageFrames el pageNum que quiero unpinnear
    int searchedPage = searchPageFramePosition(bm, pageNum);
    if (searchedPage != -1){
        pageFrames[searchedPage].fixCount++;
        RC_message = "RC_ALREADY_PINNED_PAGE";
        return RC_OK;
    }
    
//    if(pinned == 1){ // si ya esta en el buffer
//        pageFrames[i].fixCount++;
//        RC_message = "RC_ALREADY_PINNED_PAGE";
//        return RC_OK;
//        // hay que devolver el data
//    }
    else{ // si no esta en el buffer
        
        if(vacios[0] != -1){ // si hay algun espacio en el buffer libre,
            // encolo la posicion en llenos
            int j = 0;
            while (llenos[j] != -1) {
                j++;
            }
            llenos[j] = vacios[0];
            
            // desplazo los valores en la cola de vacios y meto un -1 al final
            int k = 0;
            for (k=0; k<(bm->numPages)-1; k++) {
                vacios[k] = vacios[k+1];
            }
            k++;
            vacios[k] = -1;
            
            // ponemos la nueva información del pageFrame
            pageFrames[llenos[j]].dirty = 0;
            pageFrames[llenos[j]].fixCount = 1;
            pageFrames[llenos[j]].pinned = 1;
            pageFrames[llenos[j]].pageFrameNum = llenos[j];
            pageFrames[llenos[j]].pageHandle->pageNum = pageNum;
            if(readBlock(pageNum, fHandle, pageFrames[llenos[j]].pageHandle->data) == RC_OK){
                fifoManager->readCount++;
                RC_message = "RC_FIRST_TIME_PINNED_PAGE";
                return RC_OK;
            }
            else{
                RC_message = "ERROR AL PEGAR PAGE";
                return RC_ERROR_WHEN_PINNING_PAGE;
            }
        }
        
        
        else{ // no hay hueco en el buffer
            
            int j = 0, flag = 0;
            for (j=0; j<(bm->numPages); j++) {
                if(pageFrames[llenos[j]].fixCount == 0){
                    if (pageFrames[llenos[j]].dirty == 1) {
                        forcePage(bm, pageFrames[llenos[j]].pageHandle);
                    }
                    flag = 1;
                    break;
                }
            }
            
            if (flag == 0) { // si no hay hueco en el buffer
                // todos los fixCount son mayores que 0
                RC_message = "NO AVAILABLE SPACE IN BUFFER";
                return RC_NOT_AVAILABLE_SPACE_IN_BUFFER;
            }
            
            // si hay algun hueco en el buffer:
            
            // modificamos el vector de llenos desplazandolo a la izq
            int k = 0, freeIndex = llenos[j];
            for (k=j; k<(bm->numPages)-1; j++) {
                llenos[k] = llenos[k+1];
            }
            k++;
            llenos[k] = freeIndex;
            
            // ponemos la nueva información del pageFrame
            pageFrames[freeIndex].dirty = 0;
            pageFrames[freeIndex].fixCount = 1;
            pageFrames[freeIndex].pinned = 1;
            pageFrames[freeIndex].pageFrameNum = freeIndex;
            pageFrames[freeIndex].pageHandle->pageNum = page->pageNum;
            if(readBlock(page->pageNum, fHandle, pageFrames[freeIndex].pageHandle->data) == RC_OK){
                fifoManager->readCount++;
                RC_message = "RC_FIRST_TIME_PINNED_PAGE";
                return RC_OK;
            }
            else{
                RC_message = "ERROR AL PEGAR PAGE";
                return RC_ERROR_WHEN_PINNING_PAGE;
            }
            
        }
        
    }
    
}




RC unpinPageFIFO (BM_BufferPool *const bm, BM_PageHandle *const page){

    FIFO_Manager *fifoManager = (FIFO_Manager *)bm->mgmtData;
    int *llenos = (int *)fifoManager->llenos;
    int *vacios = (int *)fifoManager->vacios;

    // busco en el pageFrames el pageNum que quiero unpinnear
    int freeFrame = searchPageFramePosition(bm, page->pageNum);
    if (freeFrame == -1){
        RC_message = "RC_NOT_FOUND_IN_PINNED_QUEUE";
        return RC_NOT_FOUND_IN_PINNED_QUEUE;
    }
    
    // modificamos el struct que almacena la informacion del pageFrame
    PageFrame *pageFrames = (PageFrame *)fifoManager->fifoPageFrames;
    if (pageFrames[freeFrame].fixCount-- == 0) {
        pageFrames[freeFrame].pinned = 0;
    }
    
    if (pageFrames[freeFrame].pinned == 0){
        // busco en llenos el index que quiero unpinnear
        int i = 0, j = 0, k = 0;
        while (llenos[i] != freeFrame) {
            i++;
        }

        // Modifico la cola de llenos a partir del valor que busco
        for (j=i; j<(bm->numPages)-1; j++) {
            llenos[j] = llenos[j+1];
        }
        j++;
        llenos[j] = -1;

        // Metemos en el primer -1 de unpinned el valor del index del page que liberamos
        while (vacios[k] != -1) {
            k++;
        }
        vacios[k] = freeFrame;
    }
    
    RC_message = "RC_OK";
    return RC_OK;


}



