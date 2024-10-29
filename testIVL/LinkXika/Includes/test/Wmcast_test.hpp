/*
 * Wmcast_test.hpp
 *
 *  Created on: 05.02.2020 г.
 *      Author: Oleg Sergeev
 */

#ifndef LINKXIKA_INCLUDES_TEST_WMCAST_TEST_HPP_
#define LINKXIKA_INCLUDES_TEST_WMCAST_TEST_HPP_





// https://en.cppreference.com/w/cpp/header

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
#include "Wmcast.hpp"
#include "Wxika.hpp"

#ifdef _WIN32
#include <../../../libconfig_win/libconfig.h++>
#else
#include <../../../libconfig_x32/libconfig.h++>
#endif /* _WIN32 */

#ifndef EXIT_SUCCESS		// ANSI C SUCCESSFUL COMPLETION
#define EXIT_SUCCESS	0
#endif

#ifndef EXIT_FAILURE		// ANSI C FAILURE COMPLETION
#define EXIT_FAILURE	1
#endif





#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

/************************************************************
 * Приостановка работы потока (в миллисекундах)
 ************************************************************/
void mySleep(int sleepMs) {
#ifdef WIN32
	// время задержки в миллисекундах - MilliSeconds
	Sleep(sleepMs);
#else
	// время задержки в микросекундах - MicroSeconds
    usleep(sleepMs * 1000);
#endif
}





// Размер данных сетевого пакета
///////////////////////////////////////////////////////
int myGetDatasize(int count, char TYtype) {
	switch (TYtype) {
	case 'A':
		return count * sizeof(type_A);
	case 'B':
		return ((count - 1) / 8 + 1) * 2;
	case 'D':
		return count * sizeof(type_D);
//	case 'F':
//		return count * sizeof(type_F);
//	case 'I':
//		return count * sizeof(type_I);
	}
	return -1;
}





/********************************************************************
 ********************************************************************/
static void puthex(long n, unsigned int digits, char *buf, unsigned int pos) {
	if (digits > 1)
		puthex(n / 16, digits - 1, buf, pos);
	buf[pos + digits - 1] = "0123456789abcdef"[n % 16];
} // puthex

/********************************************************************
 ********************************************************************/
void dumpmem(FILE *f, void *buf, unsigned int len, unsigned int offset) {
	unsigned int i;
	unsigned int address;
	unsigned char *p;
	static const unsigned int MASK_LOWER = 0x0f;

	char line[80];

	if (offset >= len)
		return;

	address = offset;
	p = ((unsigned char*) buf) + offset;

	std::fputs(
			"  Addr     0 1  2 3  4 5  6 7  8 9  A B  C D  E F 0 2 4 6 8 A C E \n",
			f);
	std::fputs(
			"--------  ---- ---- ---- ---- ---- ---- ---- ---- ----------------\n",
			f);

	while (p < ((unsigned char*) buf) + len) {
		for (i = 0; i < 50; i++)
			line[i] = ' ';
		for (; i < 80; i++)
			line[i] = 0;
		if ((address & ~MASK_LOWER) != address) { // address % 16 != 0
			puthex((address & ~MASK_LOWER), 8, line, 0);
			for (i = 0; i < (address & MASK_LOWER); i++) {
				line[10 + i * 2 + i / 2] = ' ';
				line[10 + i * 2 + i / 2 + 1] = ' ';
				line[50 + i] = ' ';
			}
			address = address & ~MASK_LOWER;
		} else {
			puthex(address, 8, line, 0);
			i = 0;
		}

		for (; i < 16; i++) {
			puthex(((long) *p) & 0x0ff, 2, line, 10 + i * 2 + i / 2);
			line[50 + i] = '.';
			if (isprint(*p))
				line[50 + i] = *p;
			if (++p >= (unsigned char*) buf + len)
				break;
		}
		std::fputs(line, f);
		std::fputs("\n", f);
		address += 16;
	}
} // dumpmem





//-----------------------------------
// Класс конфигурационных параметров
//-----------------------------------
class Wmcast_conf {
public:
	// список методов доступных другим функциям и объектам программы
	//--------------------------------------------------------------
	Log::Logger logger;	// A named logger to produce log

	Wmcast_conf() :
		logger("Wmcast_send") {
		McastGrPort = 0;
		FilterIndex = 0;
	}

	~Wmcast_conf() {
//		std::cout << "Destructing Wmcast_conf" << '\n';
	}

	std::string McastIfAddr;	// ip адрес интерфейса приема/передачи
	std::string McastGrAddr;	// групповой ip адрес
	int McastGrPort;			// групповой порт
	std::string PackNetHead;	// заголовок данных сетевого пакета
	int FilterIndex;			// фильтер по индексу параметра

	packid PackID;

	// Инициализировать - Прочитать конфигурационный файл
	///////////////////////////////////////////////////////
	int Init(const std::string& in) {
		// Прочитать файл. Или выйти с ошибкой
		// Класс в С++ не возвращает ошибку, а кидает исключение
		try {
	#ifdef _WIN32
			cfg.readFile(in.c_str());
	#else
			cfg.readFile(in.c_str());
	#endif /* _WIN32 */

		} catch (const libconfig::FileIOException &fioex) {
			logger.critic() << "I/O error while reading file " << in;
			return (EXIT_FAILURE);
		} catch (const libconfig::ParseException &pex) {
			logger.critic() << "Parse file error at " << pex.getFile() << ":"
					<< pex.getLine() << " - " << pex.getError();
			return (EXIT_FAILURE);
		}

		/// McastIfAddr ////////////////////////////////////////////////////
		if (cfg.exists("McastIfAddr")) {
			McastIfAddr = cfg.lookup("McastIfAddr").c_str();
		} else {
			logger.critic() << "No 'McastIfAddr' setting in configuration file";
			return (EXIT_FAILURE);
		}
		logger.debug() << "McastIfAddr: " << McastIfAddr;

		/// McastGrAddr ////////////////////////////////////////////////////
		if (cfg.exists("McastGrAddr")) {
			McastGrAddr = cfg.lookup("McastGrAddr").c_str();
		} else {
			logger.critic() << "No 'McastGrAddr' setting in configuration file";
			return (EXIT_FAILURE);
		}
		logger.debug() << "McastGrAddr: " << McastGrAddr;

		/// McastGrPort ////////////////////////////////////////////////////
		if (cfg.exists("McastGrPort")) {
			cfg.lookupValue("McastGrPort", McastGrPort);
		} else {
			logger.critic() << "No 'McastGrPort' setting in configuration file";
			return (EXIT_FAILURE);
		}
		logger.debug() << "McastGrPort: " << McastGrPort;

		/// PackNetHead ////////////////////////////////////////////////////
		if (cfg.exists("PackNetHead")) {
			PackNetHead = cfg.lookup("PackNetHead").c_str();
		} else {
			logger.critic() << "No 'PackNetHead' setting in configuration file";
			return (EXIT_FAILURE);
		}
		logger.debug() << "PackNetHead: " << PackNetHead;

		/// FilterIndex ////////////////////////////////////////////////////
		if (cfg.exists("FilterIndex")) {
			cfg.lookupValue("FilterIndex", FilterIndex);
		} else {
			logger.critic() << "No 'FilterIndex' setting in configuration file";
			return (EXIT_FAILURE);
		}
		logger.debug() << "FilterIndex: " << FilterIndex;

		ParsePackNetHead(PackNetHead, &PackID);

		return (EXIT_SUCCESS);
	}

	/********************************************************************
	 * Разобрать заголовок данных сетевого пакета
	 ********************************************************************/
	int ParsePackNetHead(std::string const &in, packid *out) {
		if (in.size() != 11) {
			logger.critic() << "Invalid PackNetHead [" << in << "] size = " << in.size() << " != 11";
			return (EXIT_FAILURE);
		}

		/////////////////////////////////////////////////////////////////////
		//  023      3     456    7 890
		// <n_vs_bl><type><n_spis>.<n_ver>
		/////////////////////////////////////////////////////////////////////

		int i;

		if (sscanf(in.substr(0, 3).c_str(), "%d", &i) != 1) {
			logger.critic() << "Unexpected 1-3 '" << in[0] << in[1] << in[2] << "' characters in PackNetHead [" << in << "], numeric characters expected";
			return (EXIT_FAILURE);
		} else
			out->n_vs_bl = i;	// номер внешней системы + номер энергоблока

		if (!((in[3] == 'A') or (in[3] == 'B') or (in[3] == 'D'))) {
			logger.critic() << "Unexpected 4 character '" << in[3] << "' in PackNetHead [" << in << "], character 'A' or 'B' is expected";
			return (EXIT_FAILURE);
		} else
			out->type = in[3];	// код A B (типа данных)

		if (sscanf(in.substr(4, 3).c_str(), "%d", &i) != 1) {
			logger.critic() << "Unexpected 5-7 '" << in[4] << in[5] << in[6] << "' characters PackNetHead [" << in << "], numeric characters expected";
			return (EXIT_FAILURE);
		} else
			out->n_spis = i;	// номер списка параметров

		if (!(in[7] == '.'))
			logger.warning() << "Unexpected 8 character '" << in[7] << "' in PackNetHead [" << in << "], character '.' is expected";

		if (sscanf(in.substr(8, 3).c_str(), "%d", &i) != 1) {
			logger.critic() << "Unexpected 9-11 '" << in[8] << in[9] << in[10] << "' characters PackNetHead [" << in << "], numeric characters expected";
			return (EXIT_FAILURE);
		} else
			out->n_ver = i;	// номер версии списка обмена

//		logger.debug() << "n_vs_bl: " << (int)out->n_vs_bl;
//		logger.debug() << "type:    " << out->type;
//		logger.debug() << "n_spis:  " << (int)out->n_spis;
//		logger.debug() << ".";
//		logger.debug() << "n_ver:   " << (int)out->n_ver;

		return (EXIT_SUCCESS);
	} // ParseFileName

private:
	// список свойств и методов для использования внутри класса
	//---------------------------------------------------------

	libconfig::Config cfg;

protected:
	// список средств, доступных при наследовании
	//-------------------------------------------

};





#endif /* LINKXIKA_INCLUDES_TEST_WMCAST_TEST_HPP_ */
