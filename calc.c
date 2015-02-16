#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "file_handle.h"
#include "calc.h"

#define VAR_FILE "variable_file.txt"

void var_handle (struct list *new_list, char **argv){

    char *temp_var_name, *temp_var_value;

    struct Node *temp;

    if(!argv[1] || !argv[2] || !argv[3] || strcmp(argv[2], "=")!=0){
        printf("variable syntax not correct. Please follow below syntax (space seperated):\n");
        printf("int <var_name> = <var_value>\n");
        return;
    }

    //search for the variable.
    //If exists, update the value.
    //If doesn't exist, add to the list in the beginning.

    temp_var_name = argv[1];
    temp_var_value = argv[3];

    if (temp_var_name[0] == '$'){
        printf("variable syntax not correct. Please follow below syntax (space seperated):\n");
        printf("int <var_name> = <var_value>\n");
        return;
    }

    size_t length = strlen(temp_var_value);

    for (int i=0; i<length; i++)
        if (!isdigit(temp_var_value[i]))
        {
            printf ("Entered value is not a number.\n");
            return;
        }

    if (new_list->size == 0){
        list_add(new_list, -1, temp_var_name, (atoi)(temp_var_value), 1);

        return;
    }
    else {
        temp = list_search(new_list, temp_var_name);

        if (temp == NULL){
            list_add(new_list, -1, temp_var_name, (atoi)(temp_var_value), 1);
            return;
        }
        else{
            node_update(temp, (atoi)(temp_var_value));
            return;
        }
    }
}

void cal_handle (struct list *new_list, char **argv){
    //to support : cal a = $b + $c
    //return 1;

    char *ans_var, *first_var, *second_var, *operation;
    struct Node *ans_node, *first_node, *second_node;
    int ans, first, second;

    ans_var = argv[1];
    first_var = argv[3];
    second_var = argv[5];
    operation = argv[4];

    int f_num = 1, s_num = 1;
    size_t length;

    //Answer variable

    if (ans_var[0] == '$' || strcmp(argv[2], "=")!= 0){
        printf("variable syntax not correct. Please follow below syntax (space seperated):\n");
        printf("cal <result_var> = [$<first_var> | <numeric_value>] <operation> [$<second_var> | <numeric_value>]\n");
        return;
    }

    //First variable

    length = strlen(first_var);

    for (int i=0; i<length; i++)
        if (!isdigit(first_var[i]))
        {
            f_num = 0;
            break;
        }

    if (f_num == 0){

        if (first_var[0] == '$'){
            first_var = first_var + 1;
        }
        else {
            printf("variable syntax not correct. Please follow below syntax (space seperated):\n");
            printf("cal <result_var> = [$<first_var> | <numeric_value>] <operation> [$<second_var> | <numeric_value>]\n");
            return;
        }

        if ((first_node = list_search(new_list, first_var)) == NULL) {
            printf("variable(s) not defined for the operation.\n");
            return;
        }
        else {
            first = first_node->var_value;
        }
    }
    else{
        first = atoi(first_var);
    }

    //Second variable

    length = strlen(second_var);

    for (int i=0; i<length; i++)
        if (!isdigit(second_var[i]))
        {
            s_num = 0;
            break;
        }

    if (s_num == 0){

        if (second_var[0] == '$'){
            second_var = second_var + 1;
        }
        else {
            printf("variable syntax not correct. Please follow below syntax (space seperated):\n");
            printf("cal <result_var> = [$<first_var> | <numeric_value>] <operation> [$<second_var> | <numeric_value>]\n");
            return;
        }

        if ((second_node = list_search(new_list, second_var)) == NULL) {
            printf("variable(s) not defined for the operation.\n");
            return;
        }
        else {
            second = second_node->var_value;
        }
    }
    else{
        second = atoi(second_var);
    }
    //test

    if(operation[1] == '\0'){
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
            printf("operation is out of range. (Choose one of +, -, *, / ).\n");
            return;
        }
    }
    else{
        printf("operation not defined correctly.\n");
        return;
    }

    ans_node = list_search(new_list, ans_var);

    if (ans_node == NULL){
        list_add(new_list, -1, ans_var, ans, 1);
        return;
    }
    else {
        node_update(ans_node, ans);
        return;
    }
}

void store_var (struct list *new_list){

    createVarFile(VAR_FILE);

    SH_FileHandle fh;
    SH_VarHandle vh;

    int fileOpenSuccess = openVarFile(VAR_FILE, &fh);
    int writeSuccess, readSuccess;

    struct Node *temp_node;

    temp_node = new_list->head;

    while (temp_node != NULL) {

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
    //check if file exist, if not - create it and return.

    if(access(VAR_FILE, F_OK ) == -1) {
        return;
    }
    else{
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

    char *temp_var_name;
    struct Node *temp;

    temp_var_name = argv[1];

    if (temp_var_name[0] == '$'){
        temp_var_name = temp_var_name + 1;
    }
    else{
        printf("variable syntax not correct. Please follow below syntax (space seperated):\n");
        printf("display $<var_name>\n");
        return;
    }

    temp = list_search(new_list, temp_var_name);

    if (temp == NULL){
        printf("variable has not been initialized.\n");
        return;
    }
    else{
        printf("%s : %d\n",temp_var_name, temp->var_value);
        return;
    }
}
