#include <stdio.h>
#include <stdlib.h>
#include "file_handle.h"


int createVarFile (char *fileName){

    FILE *pf=NULL;
    char* buff = (char *)calloc(BLOCK_SIZE,sizeof(char));
    pf = fopen(fileName,"w+");                  //create the file.
    int returnV = -1;
    if(fprintf(pf, "%s",buff)==0) {
        returnV = 0;
    } else{
        returnV = 1;
    }

    free(buff);                                 //release the space.
    fclose(pf);                                 //close the file.

    return returnV;
}

int openVarFile (char *fileName, SH_FileHandle *fHandle){
    FILE *pf=NULL;
    long len;
    long tailPointer = -1;
    pf=fopen(fileName, "r+");
    int returnV;
    if(pf==NULL)
        returnV = 0;
    else
    {
        fseek(pf,0L,SEEK_END);
        tailPointer = ftell(pf);                //offset of the tail of the file.
        if(tailPointer == -1)                   // if failed to fetch the offset, return failed.
            returnV = 0;
        else{
            len=tailPointer+1;                  //length from the tail to the head of the page file = bytes of the page file.
            fHandle ->fileName = fileName;      //assign the fHandle's attributions.
            fHandle ->totalNumVars =(int) (len/(BLOCK_SIZE))+1;
            fHandle ->curVarPos =0;
            fHandle ->mgmtInfo = pf;
            returnV = 0;                      // succeed to initialize the file handler
        }
    }
    return returnV;
}

int closeVarFile (SH_FileHandle *fHandle){
    if(fclose(fHandle->mgmtInfo)==EOF)          //if could not find the file ,return NOT FOUND.
        return 0;
    else
        return 1;
}

int readVar (int varNum, SH_FileHandle *fHandle, SH_VarHandle memVar){

    int seekSuccess;
    size_t readVarSize;
    FILE *readFile;

    if (varNum > fHandle->totalNumVars || varNum < 0){
        return 0;
    }

    readFile = fHandle->mgmtInfo ;

    seekSuccess = fseek(readFile, varNum*BLOCK_SIZE*sizeof(char), SEEK_SET);

    if (seekSuccess == 0){
        readVarSize = fread(memVar, sizeof(char), BLOCK_SIZE, readFile);
        fHandle->curVarPos = varNum;
        return 1;
    }
    else{
        return 0;
    }
}

int writeVar (int varNum, SH_FileHandle *fHandle, SH_VarHandle memVar){
    if(fHandle->totalNumVars<=varNum||varNum<0) {
        return 0;
    }
    if(fHandle->mgmtInfo==NULL){                 //if no such a pointer exists. return NOT FOUND.
        return 0;
    }
    if(fseek(fHandle->mgmtInfo,BLOCK_SIZE*varNum*sizeof(char),SEEK_SET)!=0){
        return 0;
    }
    //set the pointer's value to be the start of the pageNumth's block.
    if (!fprintf(fHandle->mgmtInfo, "%s",  memVar)) {
        return 0;                 //if failed to write the block return WRITE_FAILED.
    }
    //write this block's content from the mempage.
    fHandle->curVarPos=varNum;                //save the current page's position.
    return 1;
}


