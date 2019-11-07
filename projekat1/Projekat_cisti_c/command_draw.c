#include "command_draw.h"
#include "file_parser.h"

//DEKLARACIJA GLOBALNE PROMJENJIVE
int color_matrix[SIZE_OF_IMAGE];

static int* get_color_matrix(){ 
    return &color_matrix;
}

static void draw_cmd(int x1, int y1, int x2, int y2,color_id_t clr_id){
    int i;
    int color_to_write=0;
    int* p_matrix=get_color_matrix(); 
    int k;
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
    }else if (x1=x2){
        for (i=y1*Y_MAX+x1;i<y2*Y_MAX+x2;i+=Y_MAX) 
            *(p_matrix+i)=0; // ovdje treba upisati boju
    }else {
        for (i=y1;i<y2;i++)
            for (k=i*Y_MAX+x1;k<i*Y_MAX+x1+x2;k++)
                *(p_matrix+k)=0;
    }
}
        
/*void copy_image_matrix(){
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
} */

void draw_all(command_buffer_t* p_cmd_buffer){
    int i=0;
    for (i=0;i<p_cmd_buffer->size;i++)
        draw_cmd(p_cmd_buffer->commands[i].x1,p_cmd_buffer->commands[i].y1,p_cmd_buffer->commands[i].x2,p_cmd_buffer->commands[i].y2,p_cmd_buffer->commands[i].clr_id);
    //copy_image_matrix();
}
