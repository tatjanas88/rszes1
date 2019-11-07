#ifndef FILE_PARSER_H
#define FILE_PARSER_H

/*
 * FAJL SADRZI POTPISE FUNKCIJA KOJE SE KORISTE KAO INTERFEJS KA FAJL PARSERU
 * PRILIKOM KORISTENJA FAJL PARSERA NEOPHODNO JE UKLJUCITI SAMO OVAJ H FAJL
 * */
#include "file_parser_cfg.h"
#define MAX_NUM_OF_LINES 100

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
#ifndef DYNAMIC_ALLOCATION 
    command_t commands[MAX_NUM_OF_LINES];
#else
    command_t* commands;
#endif
}command_buffer_t;

//FUNKCIJA ISPISUJE SVE KOMANDE U KONZOLI, ISKLJUCIVO NAMJENJENA ZA DEBAGOVANJE
void print_commands();

/*
 * Funkcija prevodi fajl u niz komandi 
 * Ulazni parametar funkcije -> putanja do fajla kojeg treba prevesti
 * */
void process_file(const char* path);

/*Funkcija vraca pokazivac na buffer sa komandama 
 * */
command_buffer_t* get_cmd_buffer();

#endif