#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "file_handle.h"
#include "calc.h"

#define VAR_FILE "/root/variable_file"

void var_handle (struct list *new_list, char **argv){
    /*
     //search for the variable.
     //If exists, update the value.
     //If doesn't exist, add to the list.
     */
    
    char *temp_var_name, *temp_var_value;

    struct Node *temp;

    //expected command syntax : int <var_name> = <var_value>
    //extract the var_name and var_value
    
    temp_var_name = argv[1];
    temp_var_value = argv[3];

    if (new_list->size == 0){
        //if emplty list, just add the variable as head.
        list_add(new_list, -1, temp_var_name, (atoi)(temp_var_value), 1);

        return;
    }
    else {
        //if more than one element, search for the element in the list.
        temp = list_search(new_list, temp_var_name);

        if (temp == NULL){
            //if not found, add to the list.
            list_add(new_list, -1, temp_var_name, (atoi)(temp_var_value), 1);
            return;
        }
        else{
            //if found, updatet the value.
            node_update(temp, (atoi)(temp_var_value));
            //list_print(new_list);
            return;
        }
    }
}

void cal_handle (struct list *new_list, char **argv){
    /*
    //function for calculator app.
    //accepts three variables and an operand in predefined syntax.
    //calculates the answer if variables already defined, otherwise gives error message.
     */
    
    char *ans_var, *first_var, *second_var, *operation;
    struct Node *ans_node, *first_node, *second_node;
    int ans, first, second;

    //expected command syntax : cal <result_var> = $<first_var> <operand> $<second_var>
    ans_var = argv[1];
    first_var = argv[3];
    second_var = argv[5];
    operation = argv[4];

    if (first_var[0] == '$' && second_var[0] == '$'){
        // if correct syntax with '$' , extract the variable names.
        first_var = first_var + 1;
        second_var = second_var + 1;
    }
    else{
        // if syntax error : '$' missing, return with error message.
        printf("\nvariable syntax not correct.\n");
        return;
    }

    if (((first_node = list_search(new_list, first_var)) == NULL) || ((second_node = list_search(new_list, second_var)) == NULL)) {
        // if any of the two variables don't exist, return with error message.
        printf("\nvariable(s) not defined for the operation.\n");
        return;
    }
    else{
        // if variables found, fetch their values or further calculation.
        first = first_node->var_value;
        second = second_node->var_value;
    }

    if(operation[1] == '\0'){
        //if operation correctly defined, and within defined range, calculate the answer.
        char op = operation[0];

        if (op == '+'){
            ans = first + second;
        }
        else if(op == '-'){
            ans = first - second;
        }
        else if(op == '*'){
            ans = first*second;
        }
        else if(op == '/'){
            ans = first/second;
        }
        else{
            // if operation not defined, return with error message.
            printf("\noperation is out of range. (Choose one of +, -, *, / ).\n");
            return;
        }
    }
    else{
        printf("\noperation not defined correctly.\n");
        return;
    }
    
    //search for the answer variable in the list.
    ans_node = list_search(new_list, ans_var);

    if (ans_node == NULL){
        // if answer variable not defined already, add to the list.
        list_add(new_list, -1, ans_var, ans, 1);
        return;
    }
    else {
        // if answer variable defined already, update the value.
        node_update(ans_node, ans);
        return;
    }
}

void store_var (struct list *new_list){
    /*
    //Function to store the variables on the disk.
    //Gets called when the shell quits.
    */
    
    createVarFile(VAR_FILE);

    SH_FileHandle fh;
    SH_VarHandle vh;

    int fileOpenSuccess = openVarFile(VAR_FILE, &fh);
    int writeSuccess, readSuccess;

    struct Node *temp_node;

    temp_node = new_list->head;

    while (temp_node != NULL) {
        //write all the variables from the list to the file.
        //each line will have one variable and its value.
        vh = (SH_VarHandle) calloc(BLOCK_SIZE, sizeof(char));
        char temp[15];

        sprintf(temp, "%d", temp_node->var_value);
        strcat (vh, temp_node->var_name);
        strcat (vh, " ");
        strcat (vh, temp);

        writeSuccess = writeVar(fh.totalNumVars-1, &fh, vh);
        fh.totalNumVars++;

        vh = "";
        temp_node = temp_node->next;
    }
    closeVarFile(&fh);
}

void load_var (struct list *new_list){
    /*
    //function to load all the variables stored on disk to the memory
     //gets called when the shell starts.
     */
    if(access(VAR_FILE, F_OK ) == -1) {
        //check if the files exist. if not, just return.
        return;
    }
    else{
        //if exist, read variables and add to the list.
        SH_FileHandle fh;
        SH_VarHandle vh;

        int fileOpenSuccess = openVarFile(VAR_FILE, &fh);

        int readVariables = 0;
        int readVarSuccess;


        char *var_name, *var_value;
        char temp[BLOCK_SIZE];

        for (int i = 0; i < fh.totalNumVars ; ++i) {
            vh = (SH_VarHandle) calloc(BLOCK_SIZE, sizeof(char));
            readVarSuccess = readVar(i, &fh, vh);
            strcpy(temp, vh);
            var_name = strtok(temp, " ");
            var_value = strtok(NULL, " ");
            list_add(new_list, -1, var_name, (atoi)(var_value), 0);
            free(vh);
        }
        return;
    }
}

void display_var(struct list *new_list, char **argv){
    //function to display the variable.
    //expects in syntax : display $<var_name>

    char *temp_var_name;
    struct Node *temp;

    temp_var_name = argv[1];

    if (temp_var_name[0] == '$'){
        // if correct syntax, remove the $ sign.
        temp_var_name = temp_var_name + 1;
    }
    else{
        printf("\nvariable syntax not correct.\n");
        return;
    }

    //search for the variable in the list.
    temp = list_search(new_list, temp_var_name);

    if (temp == NULL){
        printf("\nvariable has not been initialized.\n");
        return;
    }
    else{
        printf("%s : %d\n",temp_var_name, temp->var_value);
        return;
    }
}