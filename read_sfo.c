/* 
  Project: Simple SF0 reader v2
  Author: Nicola Dalle Ave <rancido@ps3ita.it>
  Copyright: 2012 Nicola Dalle Ave
  License: GPL-2+
  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.
																			*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define FILE_VERSION 0x4
#define NAME_TABLE 0x8
#define DATA_TABLE 0xc
#define ENTRY_NUM 0x10
#define START_DEF_TABLE 0x14
#define DATA_INT 0x404
#define DATA_UTF_SPEC 0x4
#define PARAMS_SIZE 1024
#define SAVEDATA_FILE_LIST_SIZE 3168
#define SAVEDATA_PARAMS_SIZE 128

#if defined(__BYTE_ORDER__)&&(__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define READ_SFO_ES16(_val) \
	((uint16_t)(((((uint16_t)_val) & 0xff00) >> 8) | \
		   ((((uint16_t)_val) & 0x00ff) << 8)))

#define READ_SFO_ES32(_val) \
	((uint32_t)(((((uint32_t)_val) & 0xff000000) >> 24) | \
		   ((((uint32_t)_val) & 0x00ff0000) >> 8 ) | \
		   ((((uint32_t)_val) & 0x0000ff00) << 8 ) | \
		   ((((uint32_t)_val) & 0x000000ff) << 24)))

#else
#define READ_SFO_ES16(_val) (uint16_t)_val
#define READ_SFO_ES32(_val) (uint32_t)_val

#endif


int find_table( int xxxx_start, int xxxx_TABLE, FILE *file ) 
{
	fseek( file, xxxx_TABLE, SEEK_SET ); 
	fread( &xxxx_start, sizeof(uint32_t), 1, file );
	xxxx_start = READ_SFO_ES32(xxxx_start);
	return xxxx_start;
}                               

void print_params_value( char message[ ], char *data_utf8[ ], int counter, int size)
{
	int i;

	//printf( "%s", message );
	for( i = 0; i < size; i++ ) 
	{
		//printf( "%.2x ", *data_utf8[ counter ] );
		++data_utf8[ counter ];
	}
}

char * get_title_id_from_param(char * param_sfo_file_name)
{
	FILE *sfo;

	sfo = fopen( param_sfo_file_name, "r+" ); 
	if( sfo == NULL ) 
	{
		return 0;
	}
	
	uint32_t data_start = 0;
	uint32_t name_start = 0;
	uint32_t entries_num = 0;
	uint32_t file_version = 0;
	
	name_start = find_table( name_start, NAME_TABLE, sfo );
	data_start = find_table( data_start, DATA_TABLE, sfo );
	entries_num = find_table( entries_num, ENTRY_NUM, sfo );
	file_version = find_table( file_version, FILE_VERSION, sfo );		
	
	uint16_t offset_name[ entries_num ];
	uint16_t data_type[ entries_num ];
	uint32_t data_used[ entries_num ];
	uint32_t data_total[ entries_num ];
	uint32_t offset_data[ entries_num ];
	int i;
	
	fseek( sfo, START_DEF_TABLE, SEEK_SET );
	
	for( i = 0; i < entries_num; i++ ) 
	{		
		fread( & offset_name[ i ], sizeof(uint16_t), 1, sfo ); 
		offset_name[ i ] = READ_SFO_ES16(offset_name[ i ]);
		
		fread( & data_type[ i ], sizeof(uint16_t), 1, sfo ); 
		data_type[ i ] = READ_SFO_ES16(data_type[ i ]);
		
		fread( & data_used[ i ], sizeof(uint32_t), 1, sfo );
		data_used[ i ] = READ_SFO_ES32(data_used[ i ]);
		
	 	fread( & data_total[ i ], sizeof(uint32_t), 1, sfo );
		data_total[ i ] = READ_SFO_ES32(data_total[ i ]);
		
		fread( & offset_data[ i ], sizeof(uint32_t), 1, sfo );
		offset_data[ i ] = READ_SFO_ES32(offset_data[ i ]);
		
	}  
  
	char *list_param[ entries_num ];
    
	fseek( sfo, name_start, SEEK_SET );
		
	for( i = 0; i < entries_num; i++ ) 
	{
		list_param[ i ] = ( char *)malloc( offset_name[ i + 1 ] - offset_name[ i ] );
		fread( list_param[ i ], offset_name[ i + 1 ] - offset_name[ i ], 1, sfo );
	}	
	
	uint32_t data_int[ entries_num ];
	char *data_utf[ entries_num ];	
	
	for( i = 0; i < entries_num; i++ ) 
	{
		if( data_type[ i ] == DATA_INT ) 
		{
			fseek( sfo, data_start + offset_data[ i ], SEEK_SET );	
			fread( &data_int[ i ], data_used[ i ], 1, sfo );
			data_int[ i ] = READ_SFO_ES32(data_int[ i ]);
		}	
		else 
		{
			fseek( sfo, data_start + offset_data[ i ], SEEK_SET );
			data_utf[ i ] = (  char *)malloc( data_used[ i ] );
			fread( data_utf[ i ], data_used[ i ], 1, sfo );
		}
	}
	fclose( sfo );

	// printf( "\n****************\n|    HEADER    |\n****************\n\n" );
	// printf( "FILE_VERSION: %#x\n\nN°_PARAMETERS: %d\n\n", file_version, entries_num );
	// printf( "****************\n|  PARAMETERS  |\n****************\n\n" );

	int y;
	
	for( y = 0; y < entries_num; y++) 
	{	
		if (strcmp(list_param[y],"TITLE") != 0) {
			continue;
		}
		
		// printf( "%s: ", list_param[ y ] );
		if( data_type[ y ] == DATA_INT ) 
		{
			// printf( "0x%.2x\n\n", data_int[ y ] );
		}	
		else if( data_type[ y ] == DATA_UTF_SPEC ) 
		{
			switch( data_total[ y ] ) 
			{
				case PARAMS_SIZE:
					print_params_value( "unknown   | ", data_utf, y, 0xC );
					print_params_value( "\n\tunknown   | ", data_utf, y, 0x4 );
					print_params_value( "\n\tunknown   | ", data_utf, y, 0x4 );
					print_params_value( "\n\tunknown   | ", data_utf, y, 0x4 );
					print_params_value( "\n\tuserid    | ", data_utf, y, 0x4 );
					print_params_value( "\n\tpsid      | ", data_utf, y, 0x10 );
					print_params_value( "\n\tuserid    | ", data_utf, y, 0x4 );
					print_params_value( "\n\taccountid | ", data_utf, y, 0x10 );
					print_params_value( "\n\tunknown   | ", data_utf, y, 0x4 );
					// printf( ".. .. .. (end at offset 0x%.2x)\n\n", data_start + offset_data[ y + 1 ] );
					break;
				case SAVEDATA_FILE_LIST_SIZE:
					// printf( "file name  | %s\n\t\t    file hash? | ", data_utf[ y ] );
					for( i = 0; i < 0x10; i++ )
					{
						// printf( "%.2x ", *(data_utf[ y ] + 0xD) );
						++data_utf[ y ];
					}
					// printf( "\t\t    continue.. | " );
					for( i = 0; i < 0x4; i++ )
					{
						// printf( "%.2x ", *(data_utf[ y ] + 0xD) );
						++data_utf[ y ];
					}
					// printf( ".. .. .. (end at offset 0x%.2x)\n\n", data_start + offset_data[ y + 1 ] );
					break;
				case SAVEDATA_PARAMS_SIZE:
					print_params_value( "not change | ", data_utf, y, 0x10 );
					print_params_value( "\n\t\t change     | ", data_utf, y, 0x10 );
					print_params_value( "\n\t\t change     | ", data_utf, y, 0x10 );
					print_params_value( "\n\t\t not change | ", data_utf, y, 0x10 );
					print_params_value( "\n\t\t not change | ", data_utf, y, 0x10 );
					print_params_value( "\n\t\t not change | ", data_utf, y, 0x10 );
					print_params_value( "\n\t\t not change | ", data_utf, y, 0x10 );
					print_params_value( "\n\t\t change     | ", data_utf, y, 0x10 );
					// printf( "\n\n" );
					break;
				default:
					for( i = 0; i < data_used[ y ]; i++ ) 
					{	
						// printf( "%.2x ", *data_utf[ y ] );
						++data_utf[ y ];
					}
					// printf( "\n\n" );
			}
		}
		else {/* DATA UTF8 */
			char * new_text;
			new_text = malloc(strlen(data_utf[ y ]));
			strcpy(new_text,data_utf[ y ]);
			for( i = 0; i < entries_num; i++ ) 
			{
				free(list_param[ i ]);
			}
			for( i = 0; i < entries_num; i++ ) 
			{
				if( data_type[ i ] != DATA_INT ) {
					free(data_utf[ i ]);
				}
			}
			return new_text;
			// printf( "%s\n\n", data_utf[ y ] );
		}
	}

	for( i = 0; i < entries_num; i++ ) 
	{
		free(list_param[ i ]);
	}
	for( i = 0; i < entries_num; i++ ) 
	{
		if( data_type[ i ] != DATA_INT ) {
			free(data_utf[ i ]);
		}
	}
	// printf( "\n** Thanks www.ps3devwiki.com :D **\n\n" );
	return 0;
}
