//
//  storage_mgr.c
//  ASS1_StorageManager
//
//  Created by Juan Luis Polo and Eduardo Moreno on Spring 2020.
//  Copyright © 2020 Juan Luis Polo and Eduardo Moreno. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "storage_mgr.h"
#include "dberror.h"


//File Pointer
FILE *fp;


//
//
//
/* manipulating page files */
//
//
//


extern void initStorageManager (void){
    printf("Store Manager initialized");
    free(fp);
}


extern RC createPageFile (char *fileName){
    //Opening the file in read/write mode
    fp = fopen(fileName, "w+");
    if (fp != NULL) {
        //calloc() function initializes to zero a dynamic variable type SM_PageHandle
        SM_PageHandle newPage = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
        //Writing the new page in memory
        //Checking if the new page is correctly written
        if(fwrite(newPage, sizeof(char), PAGE_SIZE, fp) == PAGE_SIZE){
            fclose(fp);
            free(newPage);
            return RC_OK;
        }
        else return RC_WRITE_FAILED;
    }
    else return RC_FILE_NOT_FOUND;
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle){
    
    fp = fopen(fileName,"r+");
    
    if(fp != NULL){
        // File Name
        fHandle->fileName = fileName;
        
        fseek(fp, 0, SEEK_END);         // fseek() move pointer to the end of file
        int lastByte = ftell(fp);       // ftell() returns current file position
        int totalNumPages = lastByte / PAGE_SIZE;
        if(fseek(fp, 0, SEEK_SET) == 0){    // set file position to the beginning of file
            // Writing information of the opened file
            (*fHandle).totalNumPages = totalNumPages;
            (*fHandle).curPagePos = 0;
            (*fHandle).mgmtInfo = fp;
            RC_message = "FILE OPENED CORRECTLY";
            return RC_OK;
        }
        else{
            RC_message = "POINTER NOT SET TO BEGINNING OF FILE";
            return RC_READ_NON_EXISTING_PAGE;
        }
    }
    else{
        RC_message = "FILE NOT FOUND";
        return RC_FILE_NOT_FOUND;
    }
}


extern RC closePageFile (SM_FileHandle *fHandle){
    if((*fHandle).mgmtInfo != NULL){
        if(fclose((*fHandle).mgmtInfo) == 0){ // if file is already closed then return 0
            RC_message = "FILE CLOSED CORRECTLY";
            return RC_OK;
        }
        else {
            RC_message = "FILE NOT FOUND";
            return RC_FILE_NOT_FOUND;
        }
    }
    else{
        RC_message = "FILE HANDLE NOT INIT";
        return RC_FILE_HANDLE_NOT_INIT;
    }
}


extern RC destroyPageFile (char *fileName){
    if(fileName != NULL){           //Checking if the argument is valid
        if(remove(fileName) == 0){    //remove file with "fileName" name
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


extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    //Checking if the pageNum is lower than the existing number of pages and
    //if pageNum is greater than 0
    
//    (*fHandle).totalNumPages < pageNum || 
    if(pageNum < 0){
        RC_message = "READ NON EXISTING PAGE";
        return RC_READ_NON_EXISTING_PAGE;
    }
    else{
        fp = fopen((*fHandle).fileName, "r");
        if (fp != NULL) {
            //Setting the fp pointer to the "pageNum" block:
            if(fseek(fp, (pageNum*PAGE_SIZE), SEEK_SET) == 0){
                //Reading the content of the given block
                fread(memPage, sizeof(char), PAGE_SIZE, fp);
                //Storing the current position of the pointer
                (*fHandle).curPagePos = (int)ftell(fp)/PAGE_SIZE;
                if(fclose(fp) == 0){
                    RC_message = "BLOCK CORRECTLY READ";
                    return RC_OK;
                }
                else{
                    RC_message = "FILE NOT FOUND";
                    return RC_FILE_NOT_FOUND;
                }
            }
            else{
                RC_message = "READ NON EXISTING PAGE";
                return RC_READ_NON_EXISTING_PAGE;
            }
        }
        else{
            RC_message = "FILE NOT FOUND";
            return RC_FILE_NOT_FOUND;
        }
    }
}


extern int getBlockPos (SM_FileHandle *fHandle){
    //Using readBlock() function to get the current page position
    return (*fHandle).curPagePos;
}


extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    //Using readBlock() function to get the first page
    RC firstBlock = readBlock(0, fHandle, memPage);
    return firstBlock;
}


extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    //Using readBlock() function to get the previous page
    RC previousBlock = readBlock((*fHandle).curPagePos - 1, fHandle, memPage);
    return previousBlock;
}


extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    //Using readBlock() function to get the current page
    RC currentBlock = readBlock((*fHandle).curPagePos, fHandle, memPage);
    return currentBlock;
}


extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    //Using readBlock() function to get the next page
    RC nextBlock = readBlock((*fHandle).curPagePos + 1, fHandle, memPage);
    return nextBlock;
}


extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    //Using readBlock() function to get the last page
    RC lastBlock = readBlock((*fHandle).totalNumPages, fHandle, memPage);
    return lastBlock;
}


//
//
//
/* writing blocks to a page file */
//
//
//


extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    //Checking if the pageNum is lower than the existing number of pages and
    //if pageNum is greater than 0
    
//    (*fHandle).totalNumPages < pageNum || 
    if(pageNum < 0){
        RC_message = "WRITE FAILED";
        return RC_WRITE_FAILED;
    }
    else{
        fp = fopen((*fHandle).fileName, "r+");
        if (fp != NULL) {
            //Setting the fp pointer to the "pageNum" block:
            if(fseek(fp, (pageNum*PAGE_SIZE), SEEK_SET) == 0){
                //Writing the given page to file
                if(fwrite(memPage, sizeof(char), PAGE_SIZE, fp) == PAGE_SIZE){
                    //Storing the current position of the pointer
                    (*fHandle).curPagePos = (int)ftell(fp)/PAGE_SIZE;
                    fclose(fp);
                    RC_message = "BLOCK WRITTEN";
                    return RC_OK;
                }
                else{
                    RC_message = "WRITE FAILED";
                    return RC_WRITE_FAILED;
                }
            }
            else{
                RC_message = "READ NON EXISTING MESSAGE";
                return RC_READ_NON_EXISTING_PAGE;
            }
        }
        else{
            RC_message = "FILE NOT FOUND";
            return RC_FILE_NOT_FOUND;
        }
    }
}


extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
    //Using writeBlock() function to write a page to disk
    return writeBlock((*fHandle).curPagePos, fHandle, memPage);
}


//extern RC appendEmptyBlock (SM_FileHandle *fHandle){
//    //calloc() function initializes to zero a dynamic variable type SM_PageHandle
//    SM_PageHandle newPage = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
//    //Using writeBlock function to append the newPage at the end of the file (see writeBlock)
//    //If the write is OK, it increases the number of total pages of the file handle and frees the
//    //dynamic variable newPage.
//    if(writeBlock((*fHandle).totalNumPages+1, fHandle, newPage) == RC_OK){
//        (*fHandle).totalNumPages++;
////        free(newPage);
//        return RC_OK;
//    }
//    else {
////        free(newPage);
//        return RC_WRITE_FAILED;
//    }
//
//}


//extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
//
//    if((*fHandle).mgmtInfo == NULL){
//        RC_message = "RC_FILE_HANDLE_NOT_INIT";
//        return RC_FILE_HANDLE_NOT_INIT;
//    }
//    //If file has same number of pages
//    if((*fHandle).totalNumPages == numberOfPages){
//        RC_message = "FILE HAS SAME NUMBER OF PAGES";
//        return RC_OK;
//    }
//    //Else, empty blocks are added until the file has same number of pages
//    else{
//        RC_message = "FILE HAS LESS NUMBER OF PAGES";
//        do{
//            appendEmptyBlock(fHandle);
//        } while(numberOfPages > (*fHandle).totalNumPages);
//
//        RC_message = "PAGES HAVE BEEN ADDED AND FILE HAS SAME NUMBER OF PAGES";
//        return RC_OK;
//    }
//}




RC appendEmptyBlock (SM_FileHandle *fHandle){

    FILE *fileptr = fopen((*fHandle).fileName, "w");

    SM_PageHandle buffer = (SM_PageHandle) calloc(1,PAGE_SIZE);

    // Move the pointer to the EOF
    fseek(fileptr,0,SEEK_END);

    //Write the Empty block
    fwrite(buffer, sizeof(buffer),1,fileptr);

    //Update Current Page Position and Total Nº of Pages
    fHandle->totalNumPages +=1;
    fHandle->curPagePos = fHandle->totalNumPages;

    free(buffer);
    return RC_OK;
}


RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle){
    if(numberOfPages <= fHandle->totalNumPages){
        return RC_OK;
    }

    //Append the empty blocks until the desired numberOfPages
    while(fHandle->totalNumPages < numberOfPages){
        appendEmptyBlock(fHandle);
    }

    return RC_OK;
}



