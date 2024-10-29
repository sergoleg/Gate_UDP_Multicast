/*
 * Wxika.hpp - Импорт данных из разделяемой памяти симулятора и экспорт данных в ИВС ХИКА
 *
 *  Created on: 26.06.2019
 *      Author: Oleg Sergeev
 */

#ifndef _WXIKA_HPP_
#define _WXIKA_HPP_





// https://en.cppreference.com/w/cpp/header

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>

// Windows
#ifdef _WIN32
// Linux
#else
#include <arpa/inet.h>

#ifdef _DARWIN
#include <net/if.h>
#endif

#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif /* _WIN32 */

// Utilities library
#include <cstdlib>

// Strings library
#include <string>

// Containers library
#include <vector>		// std::vector container
#include <deque>		// std::deque container
#include <list>			// std::list container
#include <set>			// std::set and std::multiset associative containers
#include <map>			// std::map and std::multimap associative containers
#include <stack>		// std::stack container adaptor
#include <queue>		// std::queue and std::priority_queue container adaptors

// Input/output library
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdio>

// Algorithms library
#include <algorithm>

#include <LoggerCpp.h>

#include "Wconf.hpp"
#include "Wshm.hpp"
#include "Wmcast.hpp"

#ifndef EXIT_SUCCESS		// ANSI C SUCCESSFUL COMPLETION
#define EXIT_SUCCESS	0
#endif

#ifndef EXIT_FAILURE		// ANSI C FAILURE COMPLETION
#define EXIT_FAILURE	1
#endif

/**********************************************************************
 * Существует три вида трафика:
 *
 *   unicast   — одноадресный, один источник потока один получатель
 *   broadcast — широковещательный, один источник, получатели
 *               все клиенты в сети
 *   multicast — многоадресный, один отправитель, получатели
 *               некоторая группа клиентов
 **********************************************************************/

/********************************************************************
 * Конфигурации мультиадресной рассылки
 */
struct multicast {
	std::string ethIP;		// local IP address of interface
	std::string groupIP;	// IP multicast address of group
	int groupPORT;			// and port
};

#pragma pack(1)

/**********************************************************************
 *
 * Для организации обмена данными в сети ПМТ энергоблока 3 ДП ХИКА
 * предлагает:
 *   - для передачи аналоговой информации применять формат A
 *     (возможно использования формата F).
 *   - для передачи дискретной информации применять формат B.
 *
 **********************************************************************/

/********************************************************************
 * Структура элемента данных аналогового параметра (тип A).
 * Размер каждого элемента составляет 5 байт.
 *
 * Поле "Текущее значение параметра" соответствует типу float
 * стандарта языка C99.
 *
 * Поле "Код состояния" должен содержать качественную характеристику
 * параметра.
 *
 * Возможные значения кода отклонений:
 *   0 - норма            7 - отклонение не контролируется
 *   1 - превышение НРГ   4 - превышение ВРГ
 *   2 - превышение НАГ   5 - превышение ВАГ
 *   3 - превышение НВГ   6 - превышение ВВГ
 *
 * Возможные значения признака недостоверности:
 *   0 - значение параметра достоверно (определено);
 *   1 - значение недостоверно (не определено).
 */
struct type_A {
	float zn;					// (4 byte = 32 bit) Текущее значение параметра
	unsigned char kod_otkl :3;	// (1 byte =  8 bit) Код отклонения
	unsigned char nd :1;		//                   Признак недостоверности
	unsigned char reserv :4;	//                   Резерв
};

/********************************************************************
 * Структура элемента данных дискретных упакованных значений (тип B).
 * Элемент данных соответствует одному байту, в байте упаковывается
 * до восьми значений дискретных одиночных сигналов.
 *
 * Блок данных типа B, в общем случае, содержит два идентичных,
 * логически связанных по структуре массива элементов - массив
 * текущих состояний параметров и массив признаков недостоверности
 * этих параметров, - которые размещаются один за другим.
 *
 * Формат элемента дискретных упакованных значений
 *
 *   7   6   5   4   3   2   1   0
 * ---------------------------------
 * |n+7|   |   |   |   |   |n+1| n |
 * ---------------------------------
 *
 * ----------------------
 * | Сетевой заголовок  |
 * |--------------------|
 * | Заголовок данных   |
 * |--------------------|        ------------------------------------
 * | Блок данных типа B | <===== | Массив текущих состояний         |
 * | ( упакованные      |        ------------------------------------
 * |    дискретные      | <===== | Массив признаков недостоверности |
 * |     значения )     |        ------------------------------------
 * ----------------------
 *
 * Текущие состояния двухпозиционных дискретных параметров определяются
 * следующими значениями в массиве состояний:
 *   0 - нет, закрыто, отключено;
 *   1 - да, открыто, включено.
 *
 * Возможные значения в массиве признаков недостоверности параметров следующие:
 *   0 - значение параметра достоверно;
 *   1 - значение параметра недостоверно.
 */
struct type_B {
	union {
		struct {
			unsigned char b0 :1, b1 :1, b2 :1, b3 :1, b4 :1, b5 :1, b6 :1,
					b7 :1;
		} bits;
		unsigned char byte;	// (1 byte =  8 bit)
	};
};

/********************************************************************
 * Структура элемента данных дискретного параметра (тип D) - однобайтный код.
 *
 * Бинарное состояние двухпозиционных параметров (одиночный дискретный сигнал)
 * определяется следующими значениями:
 *   **0 - закрыто, отключено, нет;
 *   **1 - открыто, включено, да.
 *
 * Бинарные состояния многопозиционных дискретных параметров приведены ниже:
 *   - технологические задвижки (парный дискретный сигнал типа "S")
 *     *10 - закрыта;   011 - промежуточное положение (ход на открытие);
 *     *01 - открыта;   111 - промежуточное положение (ход на закрытие);
 *     *00 - технологическая недостоверность (задвижка не контролируется,
 *     отсутствует или схема задвижки разобрана);
 *
 *   - технологические механизмы (парный дискретный сигнал типа "М")
 *     *10 - отключен;   *01 - включен;
 *     *00 или *11 - технологическая недостоверность (механизм не контролируется,
 *     отсутствует или схема контроля механизма разобрана).
 *
 * Примечание. Технологическая недостоверность, как правило, фиксируется на основании
 * логики возможных состояний многопозиционного контролируемого объекта или схемы
 * его привода: невозможные состояния считают недостоверными.
 *
 * Возможные значения признака недостоверности параметра определяются по состоянию
 * аппаратуры измерительного канала (аппаратная недостоверность):
 *   0 - значение параметра достоверно (определено);
 *   1 - значение недостоверно (не определено).
 */
struct type_D {
	unsigned char sost :3;	// Бинарное состояние дискретного параметра
	unsigned char nd :1;	// Признак недостоверности
};

/********************************************************************
 * Структура элемента данных вещественных значений (тип F).
 * Размер каждого элемента составляет 4 байта.
 *
 * Недостоверность параметра отмечается кодом 0xFFFFFFFF
 * (величина "nan" в стандарте языка C99) в поле
 * "Текущее значение параметра".
 */
struct type_F {
	union {
		char c[4];
		int i;
		unsigned int u;
		float zn;
	};
};

/********************************************************************
 * Структура элемента данных целых относительных значений (тип I).
 * Размер каждого элемента составляет 4 байта.
 *
 * Поле "Текущее значение параметра" в специально оговоренных случаях может иметь
 * укороченный формат целого числа (2 байта). Применение данного формата для
 * аналоговых и дискретных параметров должно быть согласовано на этапе проектирования.
 */
struct type_I {
	union {
		char c[4];
		int zn;
		unsigned int u;
		float f;
	};
};

/********************************************************************
 */
struct type_R {
	float zn;
	union {
		unsigned short u;
		struct {
			unsigned short b0 :1, b1 :1, b2 :1, b3 :1, b4 :1, b5 :1, b6 :1, rez :9;
		} pr;
	} u;
};

/********************************************************************
 */
struct type_K {
	unsigned char zn;
	union {
		unsigned char u;
		struct {
			unsigned char b0 :1, b1 :1, b2 :1, b3 :1, rez :4;
		} pr;
	} u;
};

/*********** Форматы на выдачу пакетов во внешние системы ***********/

/********************************************************************
 */
struct TYtype_Event {
	unsigned short idx;		// индекс параметра в блоке обмена
	unsigned short dt;		// относительное время фиксации события (мс)
	union {
		struct {
			type_D pred;
			type_D tek;
		} diskr;
		struct {
			struct {
				unsigned char kod_otkl :3,	// код отклонения
				nd :1;						// недоставерность
			} pred;
			struct {
				unsigned char kod_otkl :3,	// код отклонения
				nd :1;						// недоставерность
			} tek;
		} anal;
		unsigned char value[2];
	} sost_union;
};

/********************************************************************
 */
struct TYtype_Event_K {
	unsigned short num;
	unsigned short vrem;
	union {
		unsigned short u;
		type_K k;
	} sost_p;
	union {
		unsigned short u;
		type_K k;
	} sost_n;
};

/********************************************************************
 * Заголовок данных сетевого пакета
 *
 * ID блока данных:
 *   - глобальный идентификатор источника данных (код внешней системы,
 *     включая N энергоблока) в двоичной форме;
 *   - тип данных (ASCII код), напр., А - аналоговые параметры,
 *     D - дискретные параметры, F - вещественные значения,
 *     I - целые относительные значения, B - дискретные упакованные значения и др.;
 *   - номер (код) списка параметров, включенных в блок данных, в двоичной форме
 *   - номер (код) версии списка обмена (штатная версия списка обмена - нулевая)
 *
 * Имя списка (файла) передачи для описания передаваемого
 * сетевого потока следующей структуры:
 *
 *   <rs><n_vs><n_bl><type><n_spis>.<n_ver>
 *
 * Например: S143A034.099 – список «034» версии «.099» экспорта аналоговых
 * параметров от внешней системы «14» энергоблока «3».
 *
 */
struct packid {
	unsigned char n_vs_bl;  // номер внешней системы + номер энергоблока
	unsigned char type;     // код типа данных
	unsigned char n_spis;   // номер списка параметров
	unsigned char n_ver;    // номер версии списка обмена
};

/********************************************************************
 * Данные сетевого пакета для передачи
 */
struct PACKET {
	// Заголовок данных сетевого пакета
	packid id;    // n_vs_bl - номер внешней системы + номер энергоблока
				  // type    - код типа данных
				  // n_spis  - номер списка параметров
				  // n_ver   - номер версии списка обмена
	unsigned short index;   // Индекс первого параметра в блоке или 0
	unsigned short num;     // Количество параметров в блоке
	time_t time;            // Время формирования блока данных или 0	!!!
	char *data;             // Собственно данные
};

#pragma pack()

/********************************************************************
 * Список
 */
struct List_entry {

	// Конфигурации мультиадресной рассылки
	multicast mcast;	// ethIP     - local IP address of interface
						// groupIP   - IP multicast address of group
						// groupPORT - and port

						// Заголовок данных сетевого пакета
	packid id;			// n_vs_bl - номер внешней системы + номер энергоблока
						// type    - код типа данных
						// n_spis  - номер списка параметров
						// n_ver   - номер версии списка обмена

						// Список имен параметров
	std::map<int, std::string> CIS_PntName;

	// Список блоков данных параметров
	std::map<int, pt_sample> SHM_PntValQ;	// Нумерация с 1

	int smpl_F_index;	// Первый (минимальный) порядковый номер в списке
	int smpl_E_index;	// Последний (максимальный) порядковый номер в списке
	int smpl_C_index;	// Количество параметров в сетевом пакете

	std::vector<pt_sample> Pnt_Sample;	// Нумерация с 0
};

#define MAX_D_POINTS_PER_GROUP 16380

/**********************************************************************
 **********************************************************************/
class Wxika {

public:
	// список методов доступных другим функциям и объектам программы
	//--------------------------------------------------------------
	Log::Logger logger;	// A named logger to produce log

	std::map<std::string, List_entry> Lists;// Списки параметров для передачи

	Wxika() :
			logger("Wxika") {
	}

	~Wxika() {
	}

	// Инициализировать
	///////////////////////////////////////////////////////
	int Init(std::map<std::string, Package_entry> inSendPackages);

	// Обновить динамические данные из SHM
	///////////////////////////////////////////////////////
	void UpdatePntValQ(Wshm& pshm);

	// Передача динамических динамических данных
	///////////////////////////////////////////////////////
	int SendPntValQ(Wmcast& pmcast);

private:
	// список свойств и методов для использования внутри класса
	//---------------------------------------------------------

	// Разобрать имя списка (файла) в заголовок данных сетевого пакета
	//-----------------------------------------------------------------
	int ParseFileName(std::string const &in, packid *out);

	// Разобрать список (файл) в перечень параметров с индексами
	//-----------------------------------------------------------
	int ParseFile(std::string const &prefix, std::string const &input, std::map<int, std::string> *out);

	// Печать динамических данных
	///////////////////////////////////////////////////////
	int Print_SHM_PntValQ();
	int Print_Pnt_Sample(std::map<std::string, List_entry>::iterator lst_it);

	// Размер данных сетевого пакета
	///////////////////////////////////////////////////////
	int get_datasize(int count, char TYtype) {
		switch (TYtype) {
		case 'A':
			return count * sizeof(type_A);
		case 'B':
			return ((count - 1) / 8 + 1) * 2;
		case 'D':
			return count * sizeof(type_D);
//		case 'F':
//			return count * sizeof(type_F);
//		case 'I':
//			return count * sizeof(type_I);
		}
		return -1;
	}

	// Function to in-place trim all spaces in the string such that
	// all words should contain only a single space between them
	//---------------------------------------------------------------
	void removeSpaces(std::string &str) {
		int n = str.length();
		int i = 0, j = -1;
		bool spaceFound = false;
		while (++j < n && str[j] == ' ')
			;
		while (j < n) {
			if (str[j] != ' ') {
				if ((str[j] == '.' || str[j] == ',' || str[j] == '?')
						&& i - 1 >= 0 && str[i - 1] == ' ')
					str[i - 1] = str[j++];
				else
					str[i++] = str[j++];
				spaceFound = false;
			} else if (str[j++] == ' ') {
				if (!spaceFound) {
					str[i++] = ' ';
					spaceFound = true;
				}
			}
		}
		if (i <= 1)
			str.erase(str.begin() + i, str.end());
		else
			str.erase(str.begin() + i - 1, str.end());
	} // removeSpaces

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
	} // split

protected:
	// список средств, доступных при наследовании
	//-------------------------------------------

};





#endif /* _WXIKA_HPP_ */