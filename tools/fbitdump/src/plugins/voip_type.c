/*
 * File:    voip.c
 * Author:  Jan Remes (xremes00@stud.fit.vutbr.cz)
 * Project: ipfixcol
 *
 * Description: This file contains fbitdump plugin for displaying VoIP
 *              enumeration fields in text format
 *
 * (C) CESNET 2014
 */


/**** INCLUDES and DEFINES *****/
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>

#include "plugin_header.h"

#define NAME_SIZE 16


//#define DBG(str) do { fprintf(stderr, "DEBUG: %s() in %s: %s\n", __func__, __FILE__, str); } while(0)
#define DBG(str)


/***** TYPEDEFs and DATA *****/

typedef struct {
    unsigned code;
    char name[NAME_SIZE];
} item_t;

static const item_t values[] = {
    { 0, "NO VOIP" },
    { 1, "SERV REQUEST" },
    { 2, "SERV REPLY" },
    { 3, "CALL REQUEST" },
    { 4, "CALL REPLY" },
    { 8, "VOICE DATA" },
    { 16, "RTCP" }
};

static const int NAMES_SIZE = sizeof(values) / sizeof(item_t);


/***** FUNCTIONS *****/

char *info()
{
	return "";
}

/**
 * Fill the buffer with text representation of field's content
 */
void format( const union plugin_arg * arg,
                int plain_numbers,
                char buffer[PLUGIN_BUFFER_SIZE] )
{
    DBG("called");
    // get rid of the warning
    if(plain_numbers) {;}

    int i;

    for(i = 0; i < NAMES_SIZE; i++)
    {
        if(arg->uint8 == values[i].code)
        {
            snprintf(buffer, PLUGIN_BUFFER_SIZE, "%s", values[i].name);
            return;
        }
    }

    snprintf(buffer, PLUGIN_BUFFER_SIZE, "%u", arg->uint8);
}

/**
 * Parse text data and return inner format
 */
void parse(char *input, char out[PLUGIN_BUFFER_SIZE])
{
    int i;

    for(i = 0; i < NAMES_SIZE; i++)
    {
        if(!strcasecmp(input, values[i].name))
        {
            snprintf(out, PLUGIN_BUFFER_SIZE, "%u", values[i].code);
            return;
        }
    }

    snprintf(out, PLUGIN_BUFFER_SIZE, "");
}
