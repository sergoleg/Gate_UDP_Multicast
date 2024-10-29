/*
 * Wshm.hpp - Доступ к разделяемой памяти симулятора
 *
 *  Created on: 24.06.2019
 *      Author: Oleg Sergeev
 */

#ifndef _WSHM_HPP_
#define _WSHM_HPP_

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#ifdef _WIN32
#else
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif /* _WIN32 */

// Utilities library
#include <cstdlib>

// Strings library
#include <string>

// Input/output library
#include <iostream>
#include <iomanip>
#include <fstream>

// Algorithms library
#include <algorithm>

#include <LoggerCpp.h>

#ifndef key_t
#define key_t		long
#endif

#ifndef caddr_t
#define caddr_t		char*
#endif

#ifndef IPC_STAT
#define IPC_STAT	2
#endif

#ifndef NULL
#define NULL		0
#endif

#ifndef EXIT_SUCCESS		// ANSI C SUCCESSFUL COMPLETION
#define EXIT_SUCCESS	0
#endif

#ifndef EXIT_FAILURE		// ANSI C FAILURE COMPLETION
#define EXIT_FAILURE	1
#endif

#ifndef FALSE
#define FALSE			0
#endif

#ifndef TRUE
#define TRUE			1
#endif

/*****************************************************************************
 * Internal Macros For The SHM Susbystem Functions
 *****************************************************************************/

/************************** Segment Address Macros **************************/
#define  GET_SHM_BASE_ADDRESS(shmname,base,shmsize,err) { \
            if (SHM_attach_table.find(shmname) == SHM_attach_table.end()) \
               (quality) = BAD_SEGNAME; \
            else { \
               (quality) = GOOD; \
               (base) = SHM_attach_table[shmname].shmaddr; \
               (shmsize) = SHM_attach_table[shmname].size; \
            }}

/************************* Read Shared Memory Macros *************************/
#define  READ_SHM_VAR_BYTES(start,store,size) { \
            (void) memcpy((unsigned char *)(store), (unsigned char *)(start),\
                  (int)(size));}

#define  READ_SHM_BYTES(start,store) { \
            *(unsigned char *)(store) = *(unsigned char *)(start);}

#define  READ_SHM_2_BYTES(start,store) { \
            *(unsigned short *)(store) = *(unsigned short *)(start);}

#define  READ_SHM_4_BYTES(start,store) { \
            *(unsigned short *)(store) = *(unsigned short *)(start); \
            *((unsigned short *)(store) + 1) = *((unsigned short *)(start) + 1);}

#define  READ_SHM_ALL_4_BYTES(start,store) { \
            *(unsigned long *)(store) = *(unsigned long *)(start);}

#define  READ_SHM_8_BYTES(start,store) { \
            *(unsigned long *)(store) = *(unsigned long *)(start); \
            *((unsigned long *)(store) + 1) = *((unsigned long *)(start) + 1);}

#define  READ_SHM_ALL_8_BYTES(start,store) { \
            *(double *)(store) = *(double *)(start);}

/********************************************************
 * Return Point Quality Codes For SHM Get Value Functions
 ********************************************************/
typedef enum {		// GET VALUE ROUTINE COMPLETION QUALITY CODES
	GOOD = 0,		//     Normal Quality
	BAD = 1,		// 200 General Not Normal Quality
	BAD_NOTOPEN,	//     Segment not open or does not exist
	BAD_SEGLEN,		//     Field not within segment size
	BAD_SEGNAME		//     Invalid segment name specified
} Quality;

/*******************
 * Class Data Types
 *******************/
typedef enum {			// SHARED MEMORY TYPE
	SHM_TYPE_NONE,		// 0 - None
	SHM_TYPE_SEGMENT,	// 1 - Shared Memory Segment
	SHM_TYPE_FILE		// 2 - Memory Mapped File
} SHM_type;

/*******************
 *******************/
struct SHM_attach_entry {	// SEGMENT MEMORY INFO
	int size;				// Shared Memory Segment Size
	SHM_type type;			// Shared Memory Segment Type
	char *shmaddr;			// Shared Memory Segment Mapped Address
};

/*******************
 *******************/
struct CIS_S3_point_entry {
	// Поля из файлов analogs.config + discrets.config
//	std::string CISvar;		// Имя параметра в ИВС
	std::string FSSvar;		// Имя параметра в S3
	int OffsetVar;			// Смещение в сегменте разделяемой памяти
	std::string GlobalVar;// Текстовый идентификатор сегмента разделяемой памяти
	int TypeVar;			// Тип данных в сегменте разделяемой памяти
};

/********************************
 * Формат блока данных параметра
 ********************************/
struct pt_sample {
	unsigned short type;// Тип параметра 1-float, 2-int, 3-boolean или 0-неопределенный
	Quality quality;				// Качество параметра
	union {							// Значение параметра
		float as_1_float;
		unsigned short as_2_int;
		unsigned char as_3_boolean;
	} value;
};

/*****************************************************************************
 * Доступ к разделяемой памяти симулятора
 *****************************************************************************/
class Wshm {
public:
	// список методов доступных другим функциям и объектам программы
	//--------------------------------------------------------------
	Log::Logger logger;	// A named logger to produce log

	//	Wshm();
	Wshm() :
		logger("Wshm") {
		shm_open_flag = FALSE;
	}

	~Wshm() {
		close_segments();
	}

	// Инициализировать - Прочитать конфигурационные файлы и
	//                    Открыть доступ к сегментам разделяемой памяти
	//-----------------------------------------------------------------
	int Init(const std::string& in_segment,
			const std::vector<std::string> in_point);

	// Open Shared Memory Segments For Access
	//-----------------------------------------
	int open_segments();

	// Close Shared Memory Segments
	//-------------------------------
	int close_segments();

	// Get Point Value/Quality From Shared Memory
	//-------------------------------------
	pt_sample get_point_value(const std::string& pnt_name);

private:
	// список свойств и методов для использования внутри класса
	//-----------------------------------------------------------

	int shm_open_flag;

	std::map<std::string, key_t> SHM_segment_table;

	std::map<std::string, SHM_attach_entry> SHM_attach_table;

	std::map<std::string, CIS_S3_point_entry> CIS_S3_point_table;

	// Список идентификаторов сегментов разделяемой памяти, используемых в файлах analogs.config + discrets.config
	std::map<std::string, int> GlobalVar_segment_table;

	// Read Shared Memory File Configuration - ${S3_HOME}/s3_globals file
	//---------------------------------------------------------------------
	int read_shm_config_file(const std::string& file_name);

	// Read Points File Configuration
	//---------------------------------
	int read_pnt_config_file(const std::string& file_name);

	// trim from left
	//---------------------------------------------------------------------
	inline std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v") {
		s.erase(0, s.find_first_not_of(t));
		return s;
	}

	// trim from right
	//---------------------------------------------------------------------
	inline std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v") {
		s.erase(s.find_last_not_of(t) + 1);
		return s;
	}

	// trim from left & right
	//---------------------------------------------------------------------
	inline std::string& trim(std::string& s, const char* t = " \t\n\r\f\v") {
		return ltrim(rtrim(s, t), t);
	}

	// Right way to split an std::string into a vector<string>
	//----------------------------------------------------------
	std::vector<std::string> split(const char *str, char c = ' ') {
		std::vector<std::string> result;

		do {
			const char *begin = str;

			while (*str != c && *str)
				str++;

			result.push_back(std::string(begin, str)); // @suppress("Ambiguous problem")
		} while (0 != *str++);

		return result;
	}

	// Function to in-place trim all spaces in the string such that
	// all words should contain only a single space between them
	//---------------------------------------------------------------
	void removeSpaces(std::string &str) {
		// n is length of the original string
		int n = str.length();

		// i points to next postion to be filled in
		// output string/ j points to next character
		// in the original string
		int i = 0, j = -1;

		// flag that sets to true is space is found
		bool spaceFound = false;

		// Handles leading spaces
		while (++j < n && str[j] == ' ')
			;

		// read all characters of original string
		while (j < n) {
			// if current characters is non-space
			if (str[j] != ' ') {
				// remove preceding spaces before dot,
				// comma & question mark
				if ((str[j] == '.' || str[j] == ',' || str[j] == '?')
						&& i - 1 >= 0 && str[i - 1] == ' ')
					str[i - 1] = str[j++];

				else
					// copy current character at index i
					// and increment both i and j
					str[i++] = str[j++];

				// set space flag to false when any
				// non-space character is found
				spaceFound = false;
			}
			// if current character is a space
			else if (str[j++] == ' ') {
				// If space is encountered for the first
				// time after a word, put one space in the
				// output and set space flag to true
				if (!spaceFound) {
					str[i++] = ' ';
					spaceFound = true;
				}
			}
		}

		// Remove trailing spaces
		if (i <= 1)
			str.erase(str.begin() + i, str.end());
		else
			str.erase(str.begin() + i - 1, str.end());
	}

	// Get Floating Point Value From Shared Memory
	// Используется для S3 для type = 1 - float
	//----------------------------------------------
	int
	get_FP_value(const std::string& seg_name,	// Shared Memory Segment Name
			off_t * seg_offset,				// Shared Memory Segment Offset
			float *shm_float_val);			// Floating Point Value

	// Get Short Integer Value From Shared Memory
	// Используется для S3 для type = 2 - int
	//---------------------------------------------
	int
	get_short_value(const std::string& seg_name,// Shared Memory Segment Name
			off_t * seg_offset,				// Shared Memory Segment Offset
			unsigned short *shm_short_val);	// Short Integer Value

	// Get Byte Integer Value From Shared Memory
	// Используется для S3 для type = 3 - boolean
	//---------------------------------------------
	int
	get_byte_value(const std::string& seg_name,	// Shared Memory Segment Name
			off_t * seg_offset,				// Shared Memory Segment Offset
			unsigned char *shm_byte_val);	// Byte Integer Value

	// Get Double Precision Floating Point Value From Shared Memory
	//---------------------------------------------------------------
	int
	get_double_value(const std::string& seg_name,// Shared Memory Segment Name
			off_t * seg_offset,				// Shared Memory Segment Offset
			double *shm_double_val);		// Double Precision Float Value

	// Get Long Integer Value From Shared Memory
	//--------------------------------------------
	int
	get_long_value(const std::string& seg_name,	// Shared Memory Segment Name
			off_t * seg_offset,				// Shared Memory Segment Offset
			long *shm_long_val);			// Long Integer Value

protected:
	// список средств, доступных при наследовании
	//---------------------------------------------
};

#endif /* _WSHM_HPP_ */
