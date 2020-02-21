#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include "cs3743p1.h"

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

     fseek(pHashFile->pFile, RBA, SEEK_SET);

    if (fread(pRecord, pHashFile->hashHeader.iRecSize, 1L, pHashFile->pFile) != 1)
        return RC_LOC_NOT_FOUND;

    return RC_OK;
}


int writeRec(HashFile *pHashFile, int iRBN, void *pRecord){
    long RBA = iRBN * pHashFile->hashHeader.iRecSize;

    fseek(pHashFile->pFile, RBA, SEEK_SET);

    if (fwrite(pRecord, pHashFile->hashHeader.iRecSize, 1L, pHashFile->pFile) != 1)
        return RC_LOC_NOT_WRITTEN;

    return RC_OK;
}
int insertBook(HashFile *pHashFile, Book *pBook){
        int RBN = hash(pBook->szBookId, pHashFile->hashHeader.iMaxPrimary);
    // void *pRecord;
    // pRecord = pBook;

    Book book;
    //int read = readRec(pHashFile, RBN, &readBook)
    int rec_num = readRec(pHashFile, RBN, &book);
    if(rec_num == RC_LOC_NOT_FOUND || book.szBookId[0] == '\0'){
        //printf("wrote it \n");
        return writeRec(pHashFile, RBN, pBook);

    }
    if(strcmp(book.szBookId, pBook->szBookId)==0){
        printf("1\n");
        return RC_REC_EXISTS;
    }
    printf("1.1\n");
    return RC_SYNONYM;
}
int readBook(HashFile *pHashFile, Book *pBook, int *piRBN){
    int RBN;               //record of book number
	Book Book1; 		//Book struc 
	// hash func caled
      	RBN = hash(pBook -> szBookId, pHashFile->hashHeader.iMaxPrimary);

	//using rectec to read record
	int read = readRec(pHashFile, *piRBN, &Book1);

    if(read !=RC_OK)
        printf("2\n");
        return read;
	//if book match return rec_ok else return not found
	if(strcmp(Book1.szBookId, pBook -> szBookId)== 0)
	{
        *pBook = Book1;
         printf("3\n");
		return RC_OK;
	}else
	{
         printf("4\n");
		return RC_REC_NOT_FOUND;
	}

}
