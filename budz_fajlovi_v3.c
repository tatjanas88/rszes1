#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#define MAX_PKT_SIZE (640*480*4)

//comment to send pixels as commands via regular write function of char driver
//leave uncommented to write directly to memory (faster)
#define MMAP

int main(void)

#define X_MAX 640
#define Y_MAX 480
#define SIZE_OF_IMAGE X_MAX*Y_MAX
/*
 * U SLUCAJU STATICKE ALOKACIJE NEOPHODNO JE DEFINISATI MAKSIMALNU VELICINU LINIJE IZ FAJLA 
 * KAO I MAKSIMALAN BROJ LINIJA KOJE PROGRAM MOZE PROCESIRATI U FAJLU
 * ZASTITA NIJE REALIZOVANA U SLUCAJU DA SE OVAJ BROJ PREMASI
 * U TOM SLUCAJU NASTUPA PISANJE PO NULL POINTERU ILI JOS GORE, PISANJE PO NAREDNOM ALOCIRANOM PROSTORU STO MOZE DOVESTI DO NEKONZISTENTNOSTI PODATAKA ! 
 * */
#define MAX_NUM_OF_LINES 100
#define MAX_SIZE_OF_LINES 100

#define RGB_RED     0x8000
#define RGB_BLACK   0xFFFF
#define RGB_WHITE   0x0000
#define RGB_YELLOW  0x000F
//u narednim defovima su deklarisane uin32 interpretacije RGB boja
/*typedef struct
{
    uint16_t red : 6;
    uint16_t green : 5;
    uint16_t blue: 5;
}   colour_t;
colour_t c;*/

c.green = 0x1000;
c.red = 0x10000;
/*
 * ENUMERACIJA ZA CUVANJE IDENTIFIKACIJE VRSTE KOMANDE 
 * */
typedef enum{
    BCKG_COMMAND=1,
    LINE_H_COMMAND,
    LINE_V_COMMAND,
    RECT_COMMAND
}command_id_t;

/*
 * ENUMERACIJA ZA CUVANJE IDENTIFIKACIJE BOJE
 * */
typedef enum{
    RED_CLR=1,
    GREEN_CLR,
    BLUE_CLR,
    YELLOW_CLR,
    WHITE_CLR,
    BLACK_CLR
}color_id_t;

/*
 * DEFINICIJA PODATAKA ZA CUVANJE PROCESIRANIH KOMANDI
 * */
typedef struct {
    command_id_t type;
    color_id_t clr_id;
    int x1;
    int x2;
    int y1; 
    int y2; 
}command_t;
typedef struct{
    int size; 
    command_t commands[MAX_NUM_OF_LINES];
}command_buffer_t;

/*
 * DEFINICIJA PODATAKA ZA STOROVANJE LINIJA OCITANIH IZ FAJLA
 * */
typedef struct{
    int num_of_chars;
    char content[MAX_SIZE_OF_LINES];
}line_t;
typedef struct{
    int num_of_lines;
    line_t lines[MAX_NUM_OF_LINES];
}buffer_t; 

/*
 * DEKLARACIJA GLOBALNIH PROMJENJIVIH
 * */
command_buffer_t cmd_buffer;
buffer_t buffer;
int color_matrix[SIZE_OF_IMAGE];
int* get_color_matrix(){
    return &color_matrix;
}
/*
 * Funckija vraca pokazivac na globalnu promjenjivu buffer_t buffer
 * */
buffer_t* get_buffer(){
    return &buffer;
}
/*
 * Funckija vraca pokazivac na globalnu promjenjivu command_buffer_t* cmd_buffer
 * */
command_buffer_t* get_cmd_buffer(){
    return &cmd_buffer;
}
/*
 * Funkcija procesira fajl 
 *      ARGUMETNI:
 *          path: putanja fajla u formatu stringa
 *          mode-> mod citanja, uvijek koristiti "r"
 *      REZULTAT:
 *          procesirane linije su smjestene u bafer tipa buffer_t
 * */
void process_file(const char* path,const char* mode){
    FILE* fp=fopen (path,mode);
    if (fp==NULL)
        return; 
    buffer_t* p_buffer=get_buffer();
    char temp_char;
    int current_idx=0;
    int current_line_idx=0;
    do{
        temp_char=fgetc(fp);
        char line[MAX_SIZE_OF_LINES];
        if(temp_char!='\n' || temp_char==EOF){
            if(temp_char!=' ')
                line[current_idx++]=temp_char;
        }
        else{
            if (temp_char) 
            line[current_idx]='\0';
            memcpy(&(p_buffer->lines[current_line_idx].content),&line,sizeof(line));
            p_buffer->lines[current_line_idx++].num_of_chars=current_idx;
            current_idx++;
            current_idx=0;
            //OVDJE IMAS GOTOVU LINIJU, PROCESIRAJ ODAVDE KOMANDU, NEPOTREBNO CUVATI LINIJU AKO JE ODMAH PREVEDEMO ! 
        }
        //=fgetc(fp);
    }while (temp_char!=EOF);
    p_buffer->num_of_lines=current_line_idx;
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
int find_char(char* line,int size,char ch){
    int break_location=-1;
    for (int i=0; i<size;i++)
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
int number_of_chars(char* line, int size,char ch){
    int num_of_chars=0;
    for (int i=0; i<size;i++)
        if(line[i]==ch)
            num_of_chars++;
    return num_of_chars;
}
/*
 * pomocna funkcija koja vraca stepen na 10 od argumenta num
 * */
int int_pow_10(int num){
    int ret=1;
    for (int i=0;i<num;i++)
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
int process_args(char* line, int size,int num_of_args,int* args){
    int cnt=0;
    int n=0;
    int ret=0;
    for (int i=0;i<num_of_args-2;i++){
        args[i]=0;
        int position=find_char(line,strlen(line),',');
            for (int i=position-1;i>=0;i--){
                if(line[i]<'0'|| line[i]>'9')
                    return -1;
                ret++;
                n=n+int_pow_10(position-i-1)*(line[i]-((int)'0'));
            }
        args[i]=n;
        ret++;
        line=line+position+1;
        n=0;
    }
    //pronadji i zadnji karakter
    int position=find_char(line,size,';');
    for (int i=position-1;i>=0;i--){
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
color_id_t decode_clr(char* clr){
        char* av_colors[6]={"RED","GREEN","BLUE","YELLOW","WHITE","BLACK"};
        int num_of_av_colors=6;
        for(int i=0; i<num_of_av_colors;i++) {
            if(!strcmp(clr,av_colors[i])){ //pronasli smo boju
                return i+1;
        }
    }
}
/*
 * FUNKCIJA PROCESIRA JEDNU KOMANDU
 *      ARGUMENTI :
 *          line -> jedna linija iz bafera buffer_t
 *          size -> velicina linije za procesiranje , pri pozivu pozeljno koristiti funkciju strlen(). Alternativa je koristiti num_of_chars iz strukture line_t ; 
 *          cmd_id-> indeks trenutne komande (redni broj komande, indeks, u baferu tipa command_t) 
 * */
void process_command(char* line,int size,int cmd_id){
    //nadji :
    command_t l_command;
    char* av_commands[4]={"BCKG","LINE_H","LINE_V","RECT"};
    int num_of_args[4]={1,4,4,5};
    int break_location=find_char(line,size,':');
    if (break_location==-1)
        return; //nema tacke zareza 
    //ako nismo prekinuli, kopirajmo komandu
    char command[10];
    memcpy(&command,line,break_location);
    command[break_location]='\0';
    l_command.type=0;
    int command_idx=-1;
    for(int i=0; i<4;i++) {
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
                //printf();
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
    p_command_buffer->size++;
    p_command_buffer->commands[cmd_id].clr_id=l_command.clr_id;
    p_command_buffer->commands[cmd_id].type=l_command.type;
    p_command_buffer->commands[cmd_id].x1=l_command.x1;
    p_command_buffer->commands[cmd_id].x2=l_command.x2;
    p_command_buffer->commands[cmd_id].y1=l_command.y1;
    p_command_buffer->commands[cmd_id].y2=l_command.y2;
}
void draw_cmd(int x1, int y1, int x2, int y2,color_id_t clr_id){
    int i;
    int color_to_write=0;
    switch (clr_id){
        case RED_CLR:
            color_to_write=RGB_RED;
        break;
        case GREEN_CLR:
            color_to_write=RGB_GREEN;
        break;
        case BLUE_CLR:
            color_to_write=RGB_BLUE;
        break;
        case YELLOW_CLR:
            color_to_write=RGB_YELLOW;
        break; 
        case WHITE_CLR:
            color_to_write=RGB_WHITE;
        break;
        case BLACK_CLR:
            color_to_write=RGB_BLACK;
        break;
        default:
        break;
    }
    if (y1=y2){//linija horizontalna
        for (i=Y_MAX*y1+x1; i<Y_MAX*y1+x2;i++)
            *(p_matrix+i)=0; //ovdje treba upisati boju
        break;
    }else if (x1=x2){
        for (i=y1*Y_MAX+x1;i<y2*Y_MAX+x2;i+=Y_MAX) 
            *(p_matrix+i)=0; // ovdje treba upisati boju
    }else {
        for (i=y1;i<y2;i++)
            for (k=i*Y_MAX+x1;k<i*Y_MAX+x1+x2;k++)
                *(p_matrix+k)=0;
    }
}
/*void draw_command(command_t* command){
    int* p_matrix=get_color_matrix();
    switch (command->type){
        case
    }
}*/

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
void print_lines(){
    buffer_t* p_buffer=get_buffer();
    int i;
    printf("***********************************\n");
    printf("LINES");
    printf("***********************************\n");
    printf("BUFFER SIZE:%d\n",p_buffer->num_of_lines);
    printf("***********************************\n");
    for(i=0;i<p_buffer->num_of_lines+1;i++)
        printf("%s\n",p_buffer->lines[i].content);
    printf("***********************************\n");
}
void copy_image_matrix(){
    int x,y;
	// If memory map is defined send image directly via mmap
	int fd;
	int *p;
	fd = open("/dev/vga_dma", O_RDWR|O_NDELAY);
	if (fd < 0)
	{
		printf("Cannot open /dev/vga for write\n");
		return -1;
	}
	p=(int*)mmap(0,640*480*4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	memcpy(p, color_matrix, MAX_PKT_SIZE);
	munmap(p, MAX_PKT_SIZE);
	close(fd);
	if (fd < 0)
	{
		printf("Cannot close /dev/vga for write\n");
		return -1;
	}
} 
//pozivom ove funkcije u main u vrsi se obrada fajla na zadatoj lokaciji u modu citanja 
//nakon toga je potrebno procesirati sve storovane podatke u baferu 
//obrada se izvrsava tako sto se za svaku liniju poziva funkicija process_command()
//pozeljno bi bilo napisati samo jednu funkciju koja ce sve podatke ocitane iz fajla procesirati
//funkcija treba obezbediti pokazivac na postojece komande (svejedno da li ce vracati pokazivac, ili ce joj pokazivac biti prosledjen). 
//u slucaju da se vrsi prosledjvanje pokazivaca, a da su prethodno komande(buffer komandi) deklarisane lokalno u funkciji, obratiti paznju na allokaciju memorije  
//razmisliti o mogucnosti da se postojanost svih promjenjivih (osim eventualno one bitne za dalji rad, a to je lista komandi u formatu command_t) ogranici u lokalnom skoupu funkcije ispod
void command_interpreter_main(char* file_path){
    process_file(file_path,"r");
    //prodji kroz sve komande
    buffer_t* p_buffer=get_buffer();
    command_buffer_t* p_cmd_buffer=get_cmd_buffer(); 
    int i=0;
    for (i=0;i<p_buffer->num_of_lines+1;i++)
        process_command(p_buffer->lines[i].content,strlen(p_buffer->lines[i].content),p_cmd_buffer->size);
    for (i=0;i<p_cmd_buffer->size;i++)
        draw_cmd(p_cmd_buffer->comands[i].x1,p_cmd_buffer->comands[i].y1,p_cmd_buffer->comands[i].x2,p_cmd_buffer->comands[i].y2,p_cmd_buffer->comands[i].clr_id);
    copy_image_matrix();
}           
int main(int argc, char **argv)
{
    
    /*process_file("C:\\Users\\HP\\test.txt","r");
    printf("%d\n",buffer.lines[0].num_of_chars);
    printf("%s\n",buffer.lines[0].content);
	printf("hello world\n");
    //process_command("BCKG:122,,,, ,",14,0);
    char* test_str="LINE_H:1,2,3;YELLOW";
    process_command(test_str,strlen(test_str),0);
    printf("TYPE:\n%d\n",cmd_buffer.commands[0].type);
    printf("CLR_ID:%d\n",cmd_buffer.commands[0].clr_id);
    printf("X1:%d\n",cmd_buffer.commands[0].x1);
    printf("X2:%d\n",cmd_buffer.commands[0].x2);
    printf("Y1:%d\n",cmd_buffer.commands[0].y1);
    printf("Y2:%d\n",cmd_buffer.commands[0].y2);
    int num_of_args=3;
    int* arguments=calloc(3,sizeof(int));*/
    //int proba=process_args(test_str,sizeof(test_str),3,arguments);
    //printf("PROBA:%d\n",proba);
    
    //printf("%s\n",test_str+proba);
    /*remove_spaces();
    printf("%d\n",buffer.lines[0].num_of_chars);
    printf("%s\n",buffer.lines[0].content);*/
    command_interpreter_main("C:\\Users\\HP\\Documents\\test.txt");
    //print_lines();
    print_commands();
	return 0;
}
