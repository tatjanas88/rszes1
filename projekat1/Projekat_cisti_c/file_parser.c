/*
 * U OVOM FAJLU SE NALAZI IMPLEMENTACIJA FILE PARSERA
 * */

#include "file_parser.h"
#include "command_draw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
/*#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>*/

/*
 * DEKLARACIJA GLOBALNIH PROMJENJIVIH
 * */
#ifndef DYNAMIC_ALLOCATION
command_buffer_t cmd_buffer;
#else
command_buffer_t* cmd_buffer;
#endif
int color_matrix[SIZE_OF_IMAGE];

// potpisi statickih funkcija
static int find_char(char* line,int size,char ch);
static int number_of_chars(char* line, int size,char ch);
static int int_pow_10(int num);
static int process_args(char* line, int size,int num_of_args,int* args);
static color_id_t decode_clr(char* clr);
static void process_command(char* line,int size);

/*
 * Funckija vraca pokazivac na globalnu promjenjivu command_buffer_t* cmd_buffer
 * */
command_buffer_t* get_cmd_buffer(){
#ifndef DYNAMIC_ALLOCATION
    return &cmd_buffer;
#else
    return cmd_buffer;
#endif
}
/*
 * Funkcija pronalazi indeks prvog karaktera u stringu (poziciju karaktera u stringu)
 * ARGUMENTI:
 *          line-> string u kome se traze karakteri
 *          size-> velicina linije (koristiti strlen())
 *          ch->   karakter kojeg je potrebno pronaci
 * POVRATNA VIJEDNOST
 *          funkcija vraca poziciju(indeks) karaktera ako je pronadjen
 *          ako karakter nije pronadjen, funkcija vraca -1 
 * */
static int find_char(char* line,int size,char ch){
    int break_location=-1;
    int i;
    for (i=0; i<size;i++)
        if(line[i]==ch){
            break_location=i;
            break; 
        }
    return break_location;
}
/*
 * Funkcija pronalazi broj karaktera u stringu. 
 * ARGUMENTI:
 *          line-> string u kome se traze karakteri
 *          size-> velicina linije (koristiti strlen())
 *          ch->   karakter kojeg je potrebno pronaci
 * POVRATNA VIJEDNOST
 *          funkcija vraca broj detektovanih kararktera
 * */
static int number_of_chars(char* line, int size,char ch){
    int num_of_chars=0;
    int i=0;
    for (i=0; i<size;i++)
        if(line[i]==ch)
            num_of_chars++;
    return num_of_chars;
}
/*
 * pomocna funkcija koja vraca stepen na 10 od argumenta num
 * */
static int int_pow_10(int num){
    int ret=1;
    int i=0; 
    for (i=0;i<num;i++)
        ret*=10;
    return ret;
}
/*
 * Funkcija prima :
 *      line-> liniju za procesiranje
 *      size-> velicinu linije za procesiranje
 *      num_of_args-> broj argumenata (samo integera) koje treba izvaditi iz linije 
 *      args-> pokazivac na niz integera u koje treba smjestiti argumente
 * Funkcija vraca:
 *      ret->broj karaktera koje je vec iskoristila za izdvajanje brojeva -> koristi se za dalje procesiranje linije
 * */
static int process_args(char* line, int size,int num_of_args,int* args){
    int cnt=0;
    int n=0;
    int ret=0;
    int i=0;
    int k=0;
    int position;
    for (i=0;i<num_of_args-2;i++){
        args[i]=0;
        position=find_char(line,strlen(line),',');
        k=position-1;
            for (;k>=0;k--){
                if(line[k]<'0'|| line[k]>'9')
                    return -1;
                ret++;
                n=n+int_pow_10(position-k-1)*(line[k]-((int)'0'));
            }
        k=0;
        args[i]=n;
        ret++;
        line=line+position+1;
        n=0;
    }
    //pronadji i zadnji karakter
    position=find_char(line,size,';');
    for (i=position-1;i>=0;i--){
        if(line[i]<'0'||line[i]>'9')
            return -1;
            ret++;
            n=n+int_pow_10(position-i-1)*(line[i]-((int)'0'));
    }
    args[num_of_args-2]=n;
    line=line+position+1;
    ret++;
    //svi brojevi procesirani, ostaje linija 
    return ret;
    
}
/*
 * Funkcija prima boju u formatu stringa, i vraca ekvivalent definisan u color_id_t
 * */
static color_id_t decode_clr(char* clr){
        char* av_colors[6]={"RED","GREEN","BLUE","YELLOW","WHITE","BLACK"};
        int num_of_av_colors=6;
        int i=0;
        for(i=0; i<num_of_av_colors;i++) {
            if(!strcmp(clr,av_colors[i])){ //pronasli smo boju
                return i+1;
            }
        }
}
/*
 * FUNKCIJA PROCESIRA JEDNU KOMANDU
 *      ARGUMENTI :
 *          line -> linija koju treba procesirat
 *          size -> velicina linije za procesiranje , pri pozivu pozeljno koristiti funkciju strlen(). Alternativa je koristiti num_of_chars iz strukture line_t ; 
 *          cmd_id-> indeks trenutne komande (redni broj komande, indeks, u baferu tipa command_t) 
 * */
static void process_command(char* line,int size){
    static cmd_id=0;
#ifdef DYNAMIC_ALLOCATION
    printf("pucanj");
    //okej, imamo dinamicku alokaciju, cupamo pointer
    if (cmd_buffer==NULL)//alociramo prvi put i samo jednom !
        cmd_buffer=(command_buffer_t*) calloc(1,sizeof(command_buffer_t));
    //sada provjerimo da li je pointer na komande null, ako jeste, opet alociram
    printf("pucanj");
    if(cmd_buffer->commands==NULL){ //prvi put alociramo komandu
        cmd_buffer->commands=(command_t*)calloc(1,sizeof(command_t));
       printf("pucanj");
    }
    else  //ako pointer na komande nije null, realociramo na lokacija cmd_buffer za cmd_id 
        cmd_buffer->commands=(command_t*)realloc(cmd_buffer->commands,(1+cmd_id)*sizeof(command_t));
    printf("pucanj");
#else
    //u suprotnom je globalna promjenjica vec definisana 
#endif
    //nadji :
    command_t l_command;
    char* av_commands[4]={"BCKG","LINE_H","LINE_V","RECT"};
    int num_of_args[4]={1,4,4,5};
    int break_location=find_char(line,size,':');
    if (break_location==-1)
        return; //nema tacke zareza
    //ako nismo prekinuli, kopirajmo komandu
#ifndef DYNAMIC_ALLOCATION
    char command[10];
    memcpy(&command,line,break_location);
#else
    char* command=calloc(break_location+1,sizeof(char));
    memcpy(command,line,break_location);
#endif
    command[break_location]='\0';
    l_command.type=0;
    int command_idx=-1;
    int i;
    for(i=0; i<4;i++) {
        if(!strcmp(command,av_commands[i])){ //pronasli smo komandu
            l_command.type=i+1;
            command_idx=i;
        }
    }
    //komanda obradjena, prepodesy line 
    line = line+break_location+1;
    //pogledaj da li je pronadjen terminator ;
    int arg_arr[4];
    if (num_of_args[command_idx]-1>0){   //trazimo brojeve samo ako postoje dodatni argumenti osim boje za datu komandu! 
        if(find_char(line+1,strlen(line),';')<0)
            return;
        //ako je pronadjen terminator, pogledaj koliko imamo zareza
        if(number_of_chars(line,strlen(line),',')!=num_of_args[command_idx]-2)
            return; //nedovoljan broj argumenata!
        int ret=process_args(line,strlen(line),num_of_args[command_idx],arg_arr);
        line=line+ret;
    }
    
    //DEKODIRAJ BOJU 
    color_id_t clr_id=decode_clr(line);
    l_command.clr_id=clr_id;
    //popuni argumente
    switch (l_command.type){
            case BCKG_COMMAND:
                l_command.x1=0;
                l_command.x2=640;
                l_command.y1=0;
                l_command.y2=480;
            break;
            case LINE_H_COMMAND:
                l_command.x1=arg_arr[0];
                l_command.x2=arg_arr[1];
                l_command.y1=arg_arr[2];
                l_command.y2=l_command.y1;
            break;
            case LINE_V_COMMAND:
                l_command.x1=arg_arr[0];
                l_command.x2=arg_arr[0];
                l_command.y1=arg_arr[1];
                l_command.y2=arg_arr[2];
            break;
            case RECT_COMMAND:
                l_command.x1=arg_arr[0];
                l_command.x2=arg_arr[1];
                l_command.y1=arg_arr[2];
                l_command.y2=arg_arr[3];
            break;
            default:
                return;
            break;
    }
    //napokon, komanda je spremna, prepisujemo u buffer komandi
    command_buffer_t* p_command_buffer=get_cmd_buffer();
    command_t* p_command=&p_command_buffer->commands[cmd_id];
    p_command_buffer->commands[cmd_id].clr_id=l_command.clr_id;
    p_command_buffer->commands[cmd_id].type=l_command.type;
    p_command_buffer->commands[cmd_id].x1=l_command.x1;
    p_command_buffer->commands[cmd_id].x2=l_command.x2;
    p_command_buffer->commands[cmd_id].y1=l_command.y1;
    p_command_buffer->commands[cmd_id].y2=l_command.y2;
    cmd_id++;
    p_command_buffer->size=cmd_id;
}

void print_commands(){
    int i;
    command_buffer_t* p_cmd_buffer=get_cmd_buffer();
    printf("===============================\n");
    printf("COMMANDS\n");
    printf("===============================\n");
    for(i=0;i<p_cmd_buffer->size;i++){
        printf("-------------------------------\n");
        printf("ID:%d\n",i);
        printf("type:%d\n",p_cmd_buffer->commands[i].type);
        printf("clr_id:%d\n",p_cmd_buffer->commands[i].clr_id);
        printf("x1:%d\n",p_cmd_buffer->commands[i].x1);
        printf("x2:%d\n",p_cmd_buffer->commands[i].x2);
        printf("y1:%d\n",p_cmd_buffer->commands[i].y1);
        printf("y2:%d\n",p_cmd_buffer->commands[i].y2);
        printf("-------------------------------\n");
    }
    printf("===============================\n");
}

void process_file(const char* path){
    FILE* fp=fopen (path,"r");
    if (fp==NULL)
        return; 
    char temp_char;
    int current_idx=0;
    int current_line_idx=0;
#ifndef DYNAMIC_ALLOCATION
    char line[MAX_SIZE_OF_LINES];
#else
    char* line = (char*) calloc(1,sizeof(char));
    int num_of_alloc=1;
    if (line==NULL)
        return;
#endif
    do{
        temp_char=fgetc(fp);
        if(temp_char!='\n' && temp_char!=EOF){
            if(temp_char!=' '){
#ifdef DYNAMIC_ALLOCATION
                if (num_of_alloc==current_idx)
                line = (char*) realloc(line,(1+num_of_alloc++)*sizeof(char));
                    if (line==NULL)
                        return;
#endif  
                line[current_idx++]=temp_char;
            }
        }
        else{
#ifndef DYNAMIC_ALLOCATION
            line[current_idx]='\0';
            (void)process_command(&line,current_idx);
#else
            /*line = (char*) realloc(1+num_of_alloc++,sizeof(char));
            if (line==NULL)
                return;*/
            line[current_idx]='\0';
            (void)process_command(line,current_idx);
#endif 
            line[current_idx]='\0';
            current_idx++;
            current_idx=0;
        }
    }while (temp_char!=EOF);
    fclose(fp);
}

