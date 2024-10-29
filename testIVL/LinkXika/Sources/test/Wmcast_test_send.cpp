/*
 * Wmcast_test_send.cpp - аля программа rex_light.cpp (ХИКА)
 *
 *  Created on: 05.02.2020
 *      Author: Oleg Sergeev
 */

/*
 * Программа rex_light является служебным инструментальным средствам отладки обмена информацией
 * в соответствии с "Технической спецификацией сопряжения ИВС "Комплекс АЭС" с локальными системами энергоблока".
 *
 * rex_light - программа выдачи в сетевых пакетов данных, составленных в соответствии со
 *             спецификацией сопряжения с локальными системами
 *
 * Программа имеет настроечный файл rex_light.cfg, содержащий следующие настроечные параметры:
 *
 *	#ID             - идентификатор блока данных
 *	                  Напр., #ID 061B001.000
 *                    Здесь 061-идентификатор источника данных, B-тип данных,
 *                    001 - номер списка обмена,  000-номер версии списка обмена)
 *                    Таблицу зарезервированных при проекировании идентификаторов можно
 *                    увидеть в приложении к спецификации сопряжения с локальными системами
 *
 *	#COUNT          - количество передаваемых параметров. Напр., #COUNT 300
 *
 *	#SEND_TO_ADDR   - групповой(multicast) IP адрес (224.0.0.х) на который происходит выдача данных.
 *	                  Этот адрес является уникальным для каждой внешней системы и согласовывается
 *	                  на этапе проектирования. Внешняя система, являющаяся приемником данных
 *	                  сетевых пакетов, должна прослушивать именно этот групповой адрес.
 *	                  Таблицу зарезервированных при проекировании групповых адресов можно
 *	                  увидеть в приложении к спецификации сопряжения с локальными системами
 *	                  Напр., #SEND_TO_ADDR 224.0.0.114
 *
 *	#SEND_TO_PORT   - порт UDP протокола по которому происходит передача (по спецификации - 9090)
 *	                  #SEND_TO_PORT 9090
 *	#SEND_ETHER     - имена сетевых интерфейсов(через запятую), в которые выдавать пакеты.
 *                    Напр., #SEND_ETHER eth2, eth3
 *
 *	#TIME           - интервал выдачи пакетов в секундах
 *	                  Напр., #TIME 1
 *
 *    Программу необходимо запускать от пользователя root. При этом останов ПО станции не требуется.
 * Программа запускается без параметров, считывая конфигурационный файл из текущего каталога.
 * После запуска программа начинает выдавать в фоновом режиме сетевые пакеты данных, структура которых
 * соответствует технической спецификации обмена с внешними системами. Выдача происходит в сетевые интерфейсы
 * перечисленные ключем #SEND_ETHER. В заголовок пакетов вписывается идентификатор (#ID), текущее время,
 * количество выдаваемых параметров (#COUNT) из настроечного файла. Поля данных отправляемых пакетов
 * заполняются нулями.
 *
 *    Паралельно с выдачей пакетов пользователю предоставляется возможность интерактивно (с клавиатуры)
 * менять значения по отдельным параметрам. Для этого после запуска программы в нижней части экрана
 * предлагается ввести порядковый номер изменяемого параметра и устанавливаемые по этому параметру
 * значения аттрибутов. Данные требуется вводить через пробел. Окончание ввода необходимо завершить
 * переводом строки (кл. ENTER).
 *
 *    При этом независимо от того изменил пользователь значение передаваемых параметров, изменяет их,
 * или вообще не использует эту возможность, в фоновом режиме происходит выдача сетевых пакетов.
 */

#include "../../Includes/test/Wmcast_test.hpp"

#define N_COUNT   10
#define NANN      0xFFFFFFFF

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int main(int argc, char** argv) {

	// Configure the default severity Level of new Channel objects
#ifndef NDEBUG
	Log::Manager::setDefaultLevel(Log::Log::eNotice);
#else
	Log::Manager::setDefaultLevel(Log::Log::eDebug);
#endif

	// Configure the Output objects
	Log::Config::Vector configList;
	Log::Config::addOutput(configList, "OutputConsole");
//	Log::Config::addOutput(configList, "OutputFile");
//	Log::Config::setOption(configList, "filename", "log.txt");
//	Log::Config::setOption(configList, "filename_old", "log.old.txt");
	Log::Config::setOption(configList, "max_startup_size", "0");
	Log::Config::setOption(configList, "max_size", "10000");
#ifdef WIN32
	Log::Config::addOutput(configList, "OutputDebug");
#endif

	// Create a Logger object, using a "Wmcast_send" Channel
	Log::Logger logger("Wmcast_send");

	try {
		// Configure the Log Manager (create the Output objects)
		Log::Manager::configure(configList);
	} catch (std::exception& e) {
		std::cerr << e.what();
	}

	logger.info() << "Test Start ...";

	Wmcast_conf cfg;

	// Инициализировать - Прочитать конфигурационный файл
	///////////////////////////////////////////////////////
	if (cfg.Init("Wmcast_test.cfg") == EXIT_FAILURE)
		return EXIT_FAILURE;

	// Вычислить размера сетевого пакета для передачи
	//================================================
	int memsize = myGetDatasize(N_COUNT, (char)cfg.PackID.type) + sizeof(PACKET);

	logger.debug() << "memsize: " << memsize << " bytes for " << N_COUNT << " points, type " << (char)cfg.PackID.type;

	PACKET *pack;

	pack = (PACKET*) calloc(memsize, 1);

	if (!pack) {
		logger.critic() << "Allocation failure";
		return EXIT_SUCCESS;
	}

	// Заголовок данных сетевого пакета
	//==================================
	pack->id.n_vs_bl = cfg.PackID.n_vs_bl;	// номер внешней системы + номер энергоблока
	pack->id.type    = cfg.PackID.type;		// код типа данных
	pack->id.n_spis  = cfg.PackID.n_spis;	// номер списка параметров
	pack->id.n_ver   = cfg.PackID.n_ver;	// номер версии списка обмена
	pack->index      = 1;					// индекс первого параметра в блоке или 0
	pack->num        = N_COUNT;				// количество параметров в блоке
	pack->time       = time(NULL);

	// Реализация плавание единицы в значении
	//========================================
	int cur_item_num = -1;
	int zn = 0;
	int prev_item_int = 0;
	float prev_item_float = 0;
	float f = 0.0;

	Wmcast mcSend;
	mcSend.logger.setLevel(Log::Log::eDebug);

	// Заполнить буфер и передать по сети
	//====================================
	while (1) {

		switch (pack->id.type) {

		case 'A':
			((type_A*) (&pack->data))[0].zn = f = f + 0.1;
			if (cur_item_num > -1) ((type_A*) (&pack->data))[cur_item_num].nd = (char) prev_item_float;
			cur_item_num++;
			cur_item_num = (cur_item_num) % N_COUNT;
			prev_item_float = ((type_A*) (&pack->data))[cur_item_num].nd;
			((type_A*) (&pack->data))[cur_item_num].nd = 1;
			break;

		case 'B':
		{
			cur_item_num++;
			cur_item_num = (cur_item_num) % N_COUNT;

			if (cur_item_num == 0) {
				if (zn == 0)
					zn = 1;
				else
					zn = 0;
			}

			unsigned short dataByteOffset = (int) ((cur_item_num - (pack->index-1)) / 8);	// расчет смещения до значения
			unsigned short qualByteOffset = (int) (pack->num / 8) + 1 + dataByteOffset;		// расчет смещения до недостоверности
			unsigned short bitNumber = (int) ((cur_item_num - (pack->index-1)) % 8);		// расчет № бита в байте

			logger.debug()
					<< " cur_item_num = " << cur_item_num
					<< ", dataByteOffset = " << dataByteOffset
					<< ", qualByteOffset = " << qualByteOffset
					<< ", bitNumber = " << bitNumber;

			// Текущее значение параметра
			type_B data_block;
			data_block = ((type_B*) (&pack->data))[dataByteOffset];	// Считать из памяти

			if (zn == 1) data_block.byte |= (unsigned char) (1<<bitNumber);	// Записать единицу в бит bitNumber
			if (zn == 0) data_block.byte &= (unsigned char)~(1<<bitNumber);	// Записать ноль в бит bitNumber

			((type_B*) (&pack->data))[dataByteOffset] = data_block;	// Записать в память

			// Признак недостоверности
			type_B qual_block;
			qual_block = ((type_B*) (&pack->data))[qualByteOffset];	// Считать из памяти

			if (zn == 1) qual_block.byte |= (unsigned char) (1<<bitNumber);	// Записать единицу в бит bitNumber
			if (zn == 0) qual_block.byte &= (unsigned char)~(1<<bitNumber);	// Записать ноль в бит bitNumber

			((type_B*) (&pack->data))[qualByteOffset] = qual_block;	// Записать в память
		}
			break;

		case 'D':
			if (cur_item_num > -1) ((type_D*) (&pack->data))[cur_item_num].sost = prev_item_int;
			cur_item_num++;
			cur_item_num = (cur_item_num) % N_COUNT;
			prev_item_int = ((type_D*) (&pack->data))[cur_item_num].sost;
			((type_D*) (&pack->data))[cur_item_num].sost = 1;
			break;
/*
		case 'F':
			((type_F*) (&pack->data))[0].zn = f = f + 0.1;
			if (cur_item_num > -1) ((type_F*) (&pack->data))[cur_item_num].zn = prev_item_float;
			cur_item_num++;
			cur_item_num = (cur_item_num) % N_COUNT;
			prev_item_float = ((type_F*) (&pack->data))[cur_item_num].zn;
			((type_F*) (&pack->data))[cur_item_num].zn = NANN;
			break;

		case 'R':
			if (cur_item_num > -1) ((type_R*) (&pack->data))[cur_item_num].zn = (char) prev_item_float;
			cur_item_num++;
			cur_item_num = (cur_item_num) % N_COUNT;
			break;
*/
		}

		if (logger.getLevel() == Log::Log::eDebug)
			dumpmem((FILE *) stdout, (void*) pack, (unsigned int)memsize, (unsigned int)0);

		pack->time       = time(NULL);

		// Отправить сообщение в многоадресную группу
		//==================================================
		bool retval = mcSend.Send_To_Ether(
				(unsigned char*) pack,
				(unsigned int) memsize,
				cfg.McastIfAddr,
				cfg.McastGrAddr,
				cfg.McastGrPort);

		if (!retval)
			break;

		mySleep(4000);
	} // while

	logger.info() << "Test Stop ...";
	return EXIT_SUCCESS;
}
