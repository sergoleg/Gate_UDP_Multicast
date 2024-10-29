/*
 * Wconf.hpp - Обработчик инициализирующих или конфигурационных данных приложения
 *
 *  Created on: 29.06.2019
 *      Author: Oleg Sergeev
 */

#ifndef _WCONF_HPP_
#define _WCONF_HPP_

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

#ifdef _WIN32
#include <../../libconfig_win/libconfig.h++>
#else
#include <../../libconfig_x32/libconfig.h++>
#endif /* _WIN32 */

#ifndef EXIT_SUCCESS		// ANSI C SUCCESSFUL COMPLETION
#define EXIT_SUCCESS	0
#endif

#ifndef EXIT_FAILURE		// ANSI C FAILURE COMPLETION
#define EXIT_FAILURE	1
#endif

// Пакеты параметров для многоадрессной передачи симулятором в ИВС,
// содержащие списки обмена параметров для передачи симулятором в ИВС
///////////////////////////////////////////////////////////////////////
struct Package_entry {
	std::string	ListHomeDir;

	std::string MulticastIf;
	std::string MulticastIfIp;

	std::string MulticastAddress;
	int MulticastPort;

	std::vector<std::string> Lists;
};

//-----------------------------------
// Класс конфигурационных параметров
//-----------------------------------
class Wconf {
public:
	// список методов доступных другим функциям и объектам программы
	//--------------------------------------------------------------
	Log::Logger logger;	// A named logger to produce log

	Wconf() :
		logger("Wconf") {
	}

	~Wconf() {
//		std::cout << "Destructing Wconf" << '\n';
	}

	// Инициализировать - Прочитать конфигурационный файл
	///////////////////////////////////////////////////////
	int Init(const std::string& in);

	// Получить элемент - Имя файла конфигурации общей памяти симулятора
	//////////////////////////////////////////////////////////////////////
	std::string get_SimShmCfgFileName();

	// Получить Массив - Имена файлов привязки параметров ИВС к общей памяти симулятора
	/////////////////////////////////////////////////////////////////////////////////////
	std::vector<std::string> get_CisSimCfgFileNames();

	// Получить Список - Пакеты параметров для многоадрессной передачи симулятором в ИВС
	//////////////////////////////////////////////////////////////////////////////////////
	std::map<std::string, Package_entry> get_SendPackages();

private:
	// список свойств и методов для использования внутри класса
	//---------------------------------------------------------

	libconfig::Config cfg;

	// Имя файла конфигурации общей памяти симулятора
	std::string SimShmCfgFileName;

	// Имена файлов привязки параметров ИВС к общей памяти симулятора
	std::vector<std::string> CisSimCfgFileNames;

	// Доступные для многоадресной передачи интерфейсы симулятора
	std::map<std::string, std::string> AccessMcastIfs;

	// Заголовки пакетов, содержащих списки параметров для передачи симулятором в ИВС
	std::vector<std::string> TitleSendPackages;

	// Пакеты параметров для многоадрессной передачи симулятором в ИВС
	std::map<std::string, Package_entry> Packages;

	// Прочитать файл
	///////////////////
	int readFile(std::string const &in);

	// Получить элемент - Имя файла конфигурации общей памяти симулятора
	//////////////////////////////////////////////////////////////////////
	int readSimShmCfgFileName();

	// Получить Массив - Имена файлов привязки параметров ИВС к общей памяти симулятора
	/////////////////////////////////////////////////////////////////////////////////////
	int readCisSimCfgFileNames();

	// Получить Список - Доступные для многоадресной передачи интерфейсы симулятор
	////////////////////////////////////////////////////////////////////////////////
	int readAccessMcastIfs();

	// Получить Массив - Заголовки групп, содержащих списки параметров для передачи симулятором в ИВС
	///////////////////////////////////////////////////////////////////////////////////////////////////
	int readTitleSendPackages();

	// Получить Список - Пакеты параметров для многоадрессной передачи симулятором в ИВС
	//////////////////////////////////////////////////////////////////////////////////////
	int readPackages();

protected:
	// список средств, доступных при наследовании
	//-------------------------------------------

};

#endif /* _WCONF_HPP_ */
