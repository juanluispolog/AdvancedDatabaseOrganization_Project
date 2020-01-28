//
//  storage_mgr.c
//  ASS1_StorageManager
//
//  Created by Juan Luis Polo and Eduardo Moreno on Spring 2020.
//  Copyright © 2020 Juan Luis Polo and Eduardo Moreno. All rights reserved.
//

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage_mgr.h"
#include "dberror.h"


FILE *fp;


//
//
//
/* manipulating page files */
//
//
//


extern void initStorageManager (void){
    printf("Initializing the Store Manager");
    printf("Store Manager initilized");
    fp = NULL;
}


RC createPageFile (char *fileName){
    
    //Opening the file in read/write mode
    fp = fopen(fileName, "w+");
    
    if (fp != NULL) {
        //Reserving space in memory for a new page (with size PAGE_SIZE)
        SM_PageHandle newPage = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
        
        //Writing and checking if
        if (fwrite(newPage, sizeof(SM_PageHandle), PAGE_SIZE, fp) < PAGE_SIZE) {
            //free(newPage);
            return RC_WRITE_FAILED;
        }
        else fclose(fp);
        
        return RC_OK;
        
    }
    else return RC_FILE_NOT_FOUND;
    
}



extern RC openPageFile (char *fileName, SM_FileHandle *fHandle){
    
    fp = fopen(fileName,"r+");
    
    if(fp == NULL){
        RC_message = "FILE NOT FOUND";
        return RC_FILE_NOT_FOUND;
    }
    
    else{
        
        // File Name
        (*fHandle).fileName = fileName;
        
        // Total Number of Pages
        fseek(fp,0,SEEK_END);               // SEEK_END denotes the end of the file
        int lastByte = ftell(fp);           // ftell returns current fileposition
        int totalNumPages = lastByte / PAGE_SIZE;
        rewind(fp);                         // set file position to the beginning
        (*fHandle).totalNumPages = totalNumPages;
        
        // Current Page File Position
        (*fHandle).curPagePos = 0;
        
        // mgmt Info
        (*fHandle).mgmtInfo = fp;
        
        RC_message = "FILE OPENED CORRECTLY";
        return RC_OK;
    }
}

extern RC closePageFile (SM_FileHandle *fHandle){
    
    if(fp != NULL){
        
        if((*fHandle).mgmtInfo == NULL){
            RC_message = "RC_FILE_HANDLE_NOT_INIT";
            return RC_FILE_HANDLE_NOT_INIT;
        }
        else{
            fclose(fp);                   // close file
            if(fclose((*fHandle).mgmtInfo)==0){ // if Handle is closed then return 0
                RC_message = "FILE CLOSED CORRECTLY";
                return RC_OK;}
        }
    }
    
    RC_message = "FILE NOT FOUND";
    return RC_FILE_NOT_FOUND;
}


extern RC destroyPageFile (char *fileName){

    if(fileName != NULL){
        if(remove(fileName)==0){
            RC_message = "FILE REMOVED CORRECTLY";
            return RC_OK;
        }
    }
    
    RC_message = "FILE NOT FOUND";
    return RC_FILE_NOT_FOUND;
}



//
//
//
/* reading blocks from disc */
//
//
//


RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    
//  Checking of the number of pages is < pageNum
    if (fHandle->totalNumPages < pageNum || pageNum < 0) {
        return RC_READ_NON_EXISTING_PAGE;
    }
    else{
        fp = fopen(fHandle->fileName, "r");
//      Checking if the file has been read properly
        if (fp != NULL) {
//          Seek for block # pageNum
            if(fseek(fp, pageNum*PAGE_SIZE, SEEK_SET) == 0){
//              Read block # pageNum
                fread(memPage, PAGE_SIZE, sizeof(char), fp);
                fclose(fp);
                return RC_OK;
            }
            else return RC_READ_NON_EXISTING_PAGE;
        }
        else return RC_FILE_NOT_FOUND;
    }
}

int getBlockPos (SM_FileHandle *fHandle){
    return fHandle->curPagePos;
}

RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    RC firstBlock = readBlock(1, fHandle, memPage);
    return firstBlock;
}

RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    RC previousBlock = readBlock(fHandle->curPagePos - 1, fHandle, memPage);
    return previousBlock;
}

RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    RC currentBlock = readBlock(fHandle->curPagePos, fHandle, memPage);
    return currentBlock;
}

RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    RC nextBlock = readBlock(fHandle->curPagePos + 1, fHandle, memPage);
    return nextBlock;
    
}

RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    RC lastBlock = readBlock(fHandle->totalNumPages, fHandle, memPage);
    return lastBlock;
    
}




//
//
//
/* writing blocks to a page file */
//
//
//


