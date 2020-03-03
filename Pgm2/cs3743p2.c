#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include "cs3743p2.h"

// Functions written by the student
int hashCreate(char szFileNm[], HashHeader *pHashHeader)
{
    FILE *file1;
    if(file1 = fopen(szFileNm, "rb")){
        fclose(file1);
        return RC_FILE_EXISTS;
    }else{
         file1 = fopen (szFileNm, "wb");
         fwrite(pHashHeader, pHashHeader->iRecSize, 1L, file1);
    }

    fclose(file1);
    return RC_OK;
}

 int hashOpen(char szFileNm[], HashFile *pHashFile){
    //  FILE *file = fopen("file.bin", "rb");
    //  if(file!= NULL){
    //      fread(&pHashFile, sizeof(1), 1, file);
    //  }
    pHashFile->pFile = fopen (szFileNm, "rb+");
    if(pHashFile->pFile==NULL)
        return RC_FILE_NOT_FOUND;
    fseek(pHashFile->pFile, 0, SEEK_SET);
    if(fread(&(pHashFile->hashHeader), pHashFile->hashHeader.iRecSize, 1L, pHashFile->pFile)!=1)
        return RC_LOC_NOT_FOUND;

    return RC_OK;

 }



int readRec(HashFile *pHashFile, int iRBN, void *pRecord){
    long RBA = 0;
    RBA = iRBN * pHashFile->hashHeader.iRecSize;
    //printf("RBA(from readRec func) = %lu\n", RBA);
    fseek(pHashFile->pFile, RBA, SEEK_SET);

    if (fread(pRecord, pHashFile->hashHeader.iRecSize, 1L, pHashFile->pFile) != 1)
        return RC_LOC_NOT_FOUND;

    return RC_OK;
}


int writeRec(HashFile *pHashFile, int iRBN, void *pRecord){
    long RBA = iRBN * pHashFile->hashHeader.iRecSize;

    fseek(pHashFile->pFile, RBA, SEEK_SET);

    if (fwrite(pRecord, pHashFile->hashHeader.iRecSize, 1, pHashFile->pFile) != 1)
        return RC_LOC_NOT_WRITTEN;

    return RC_OK;
}

// Determine if it exists by probing. We use a probing K value of 1.  
// If it does already exist, return RC_REC_EXISTS.  (Do not update it.)
// Limit the probing to pHashFile->hashHeader.iMaxProbes.  For example, if iMaxProbes is 3, 
// you can look at the original hash location and at most two additional records.  
// We are only looking at adjacent records below it. 
// if there isn't an empty slot and we have probed a total of iMaxProbes times 
// (including looking at the hashed location), return RC_TOO_MANY_COLLISIONS. 
// If it doesn't exist and there is an empty slot (maybe because we haven't yet written to that slot),
//  write it to that empty slot.  
// Also, we will not probe past the header record’s iMaxOvRBN.  
// This would also cause you to return RC_TOO_MANY_COLLISIONS.

int insertBook(HashFile *pHashFile, Book *pBook){
        int RBN = hash(pBook->szBookId, pHashFile->hashHeader.iMaxPrimary);
        Book book;
        int write_R;
        int rec_num = readRec(pHashFile, RBN, &book);
        int addRBN =-1;
        //printf("maxover++++++++=%d \n", pHashFile->hashHeader.iMaxOvRBN);
        //printf("imaxPrimary+++++++++++++%d\n", pHashFile->hashHeader.iMaxPrimary);
        if(rec_num == RC_LOC_NOT_FOUND || book.szBookId[0]=='\0') {
            //printf("1++++++++\n");
            write_R = writeRec(pHashFile, RBN, pBook);
            return RC_OK;
        }else if(rec_num==RC_OK && strcmp(book.szBookId, pBook->szBookId)==0){
            return RC_REC_EXISTS;
        }else{ 
           // printf("imaxprobes = %d\n", pHashFile->hashHeader.iMaxProbes);
            for(int i =0; i<pHashFile->hashHeader.iMaxProbes; i++){
                    if(RBN+i > pHashFile->hashHeader.iMaxOvRBN){

                        return RC_TOO_MANY_COLLISIONS;
                    }
                    rec_num = readRec(pHashFile, RBN+i, &book);
                    if(rec_num ==RC_OK && strcmp(book.szBookId, pBook->szBookId)==0){
                        return RC_REC_EXISTS;
                    }else if(book.szBookId[0] =='\0' || rec_num == RC_LOC_NOT_FOUND){
                        if(addRBN ==-1)
                        {
                            addRBN = RBN+i;
                        }
                    }
            }
            if(addRBN!=-1){
                rec_num = writeRec(pHashFile, addRBN, pBook);
                return RC_OK;
            }else{
                
                return RC_TOO_MANY_COLLISIONS;
            }
        }
}
            
int readBook(HashFile *pHashFile, Book *pBook, int *piRBN){
    int RBN;         //record of book number
	Book Book; 		//Book struc 
    RBN = hash(pBook->szBookId, pHashFile->hashHeader.iMaxPrimary);
    int read_num = readRec(pHashFile, RBN, &Book);
    if(strcmp(Book.szBookId, pBook->szBookId)==0)
    {
        *piRBN = RBN;
        *pBook = Book;
        return RC_OK;
    }
    //probing with a K value of 1.  If it does exist,
    // return the book via pBook, 
    //set *piRBN to its actual RBN (where it was found),  
    //and return RC_OK.
    for(int i =0; i<pHashFile->hashHeader.iMaxProbes; i++){
        //  read_num give a check number, check read_num after readRec
            if(RBN+i > pHashFile->hashHeader.iMaxPrimary)
            {
                return RC_REC_NOT_FOUND; // went past max amount of records
            }

        int read_num = readRec(pHashFile, RBN+i, &Book);
        if(read_num ==RC_LOC_NOT_FOUND){
            return RC_REC_NOT_FOUND;
        }
        if(read_num == RC_OK || strcmp(Book.szBookId, pBook->szBookId)==0)
        {
            *piRBN = RBN+i;
            *pBook = Book;
            return RC_OK;
        }
    }
    return RC_REC_NOT_FOUND;
}

int updateBook(HashFile *pHashFile, Book *pBook){
    Book book;
    int RBN = hash(pBook->szBookId, pHashFile->hashHeader.iMaxPrimary);
        for(int i =0; i<pHashFile->hashHeader.iMaxPrimary; i++){
            if(RBN+i > pHashFile->hashHeader.iMaxPrimary)
            {
                return RC_REC_NOT_FOUND; // went past max amount of records
            }
            int read_num = readRec(pHashFile, RBN+i, &book);
            if(strcmp(book.szBookId, pBook->szBookId)==0)
            {
                //*pBook = book;
                writeRec(pHashFile, RBN+i, &book);
                return RC_OK;
            }
            if(read_num== RC_REC_NOT_FOUND){
                return RC_REC_NOT_FOUND;
            }
    }
}


int deleteBook(HashFile *pHashFile, Book *pBook){
    RC_NOT_IMPLEMENTED;

}
