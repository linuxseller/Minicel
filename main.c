#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#define SVFMT "%.*s"
#define SVVARG(sv) sv.size, sv.data

typedef enum CellType {TYPE_INT, TYPE_STRING, TYPE_FORMULA} CellType;

char *TYPE_TO_STR[] = {
    [TYPE_INT]="INT",
    [TYPE_STRING]="STRING",
    [TYPE_FORMULA]="FORMULA"
};

typedef struct Cell {
    CellType type;
    void *data;
    bool row_end;
} Cell;

typedef struct Lexer {
    char *data;
    int position;
} Lexer;

typedef struct Sv {
    char *data;
    int size;
} Sv;

void print_cell(Cell cell){
    printf("Cell {\n\ttype: %s\n\tdata:", TYPE_TO_STR[cell.type]);
    switch(cell.type){
        case TYPE_INT:printf("%ld\n}\n", *(long*)cell.data);break;
        case TYPE_STRING:
        case TYPE_FORMULA:printf("%s\n}\n", (char*)cell.data);break;
    }
}

void print_usage(const char* prog_name){
    fprintf(stderr, "Usage: %s <file.mcell>", prog_name);
}
Sv next_cell_sv(Lexer *lxr, bool *endl){
    Sv sv={0};
    sv.data=&lxr->data[lxr->position];
    while(lxr->data[lxr->position]!='|' && lxr->data[lxr->position]!='\n'){
        if(lxr->data[lxr->position]=='\0'){
            return (Sv){0};
        }
        sv.size++;
        lxr->position++;
    }
    *endl = lxr->data[lxr->position]=='\n';
    lxr->position++;
    return sv;
}

Cell parse_cell_from_sv(Sv sv, bool endl){
    Cell cell = {0};
    cell.row_end = endl;
    // cell: formula, int, string
    if(sv.data[0]=='='){
        cell.type = TYPE_FORMULA;
        cell.data = malloc(sv.size+1);
        strncat(cell.data, sv.data, sv.size);
        return cell;
    }
    // cell: int, string
    bool is_num = true;
    for (int i = 0; i < sv.size; i++) {
        if(!isdigit(sv.data[i])){
            is_num = false;
        }
    }
    if(is_num){
        cell.data = (long*)malloc(sizeof(long));
        *(long*)cell.data = strtol(sv.data, NULL, 10);
        cell.type = TYPE_INT;
        return cell;
    }
    // cell string
    cell.type = TYPE_STRING;
    cell.data = malloc(sv.size+1);
    strncat(cell.data, sv.data, sv.size);
    return cell;
}

int main(int argc, char **argv){
    char *prog_name = argv[0];
    if(argc!=2){
        print_usage(prog_name);
        exit(1);
    }
    char *file_name = argv[1];
    FILE *file = fopen(file_name,"r");
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);
    Lexer lexer = {.data=NULL, .position=0};
    lexer.data = malloc(fsize+1);
    fread(lexer.data, fsize, 1, file);
    bool endl;
    Cell *cells = malloc(sizeof(Cell));
    int is=0;
    for(Sv sv = next_cell_sv(&lexer, &endl); sv.data!=NULL; sv = next_cell_sv(&lexer, &endl), is++){
        cells[is] = parse_cell_from_sv(sv,endl);
    }
    for(int i=0; i<is; i++){
        print_cell(cells[i]);
    }
    return 0;
}
