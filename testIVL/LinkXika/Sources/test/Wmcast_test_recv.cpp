/*
 * Wmcast_test_recv.cpp - аля программа rim_light.cpp (ХИКА)
 *
 *  Created on: 07.02.2020
 *      Author: Oleg Sergeev
 */

/*
 * Программа rim_light является служебным инструментальным средствам отладки процесса
 * обмена информацией в соответствии с "Технической спецификацией сопряжения ИВС "Комплекс АЭС"
 * с локальными системами энергоблока".
 *
 * rim_light - программа приема сетевых пакетов данных, составленных в соответствии со спецификацией
 *
 * Программа имеет настроечный файл rim_light.cfg, содержащий следующие настроечные параметры:
 *
 *    #RECV_PORT     - порт UDP протокола по которому происходит прием (по спецификации - 9090)
 *                     #RECV_PORT 9090
 *
 *    #RECV_MGROUP   - групповой(multicast) IP адрес по которому происходит прием.
 *                     Таблицу зарезервированных при проектировании групповых адресов можно
 *		     увидеть в приложении к спецификации сопряжения с локальными системами
 *		     Напр., #RECV_MGROUP 224.0.0.224
 *
 *    #MCAST_ETHER   - имена сетевых интерфейсов(через запятую), которые прослушиваются
 *                     Напр., #MCAST_ETHER eth2, eth3
 *			если #MCAST_ETHER eth2
 *    			то осуществляется привязка к одному интерфейсу eth2 (SO_BINDTODEVICE)
 *!!! для FC21, пока модуль не исправлен, в настройках
 *    нужно указывать реальные имена интерфейсов "en*" (см. ifconfig или /etc/aliases.net)
 *
 *
 * Фильтры пакетов:
 *
 *      #FILTER_IP     192.168.101.79 - фильтр источника пакетов
 *
 *      #FILTER_ID     072A001.000    - фильтр потока пакетов
 *
 *      #FILTER_INDEX  0              - фильтр по индексу данных (0 - пакеты событий, иначе - заданные фрагменты)
 *
 *  !!!! Строки, не содержащие "#" в первой позиции, рассматриваются как комментарий
 *
 *  Конфигурационный файл ищется в текущем каталоге запуска.
 *
 *  Программу можно запускать от имени любого пользователя.
 *  Значения параметра запуска программы (по умолчанию =1):
 *    число > 0 - начальный номер смежных пяти отображаемых элементов данных;
 *    число = 0 - значения данных не отображаются, только заголовок пакета;
 *    число < 0 - отображаются значения всех элементов данных, начиная с указанного.
 *
 *  После запуска программа переходит в режим ожидания сетевых пакетов от внешних систем.
 *  При получении пакетов от внешних систем на экран выводится
 *  - IP адрес источника пакета,
 *  - размер пакета в байтах,
 *  - содержание полей заголовка полученного пакета,
 *  - в [ ] - разность между локальным временем приемника и временем в пакете,
 *  - само время пакета.
 *
 *  Формат отображения данных зависит от их типа.
 *  Пакеты событий отображаются с меткой EVEN и номером пакета в ().
 *  В процессе инициализации программа может вывести сообщения об ошибках,
 *  свидетельствующих о некорректном задании IP адреса приема (#RECV_MGROUP)
 *  либо имени сетевого интерфейса (#MCAST_ETHER)
 *
 *  !!! Внимание !
 *
 *  Запуск программы rim_light может оказывать влияние на  параллельно работающую программу
 *  rim (программа обмена информацией с внешними системами).
 *  Если порт (#RECV_PORT) прослушивания программы rim_light совпадает с портом прослушивания программы rim,
 *  то подключеный к сокету групповой(multicast) IP адрес, по которому происходит прием программой rim_light,
 *  будет прослушиваться и программой rim. Это приведет к тому, что программа rim будет принимать пакеты по
 *  дополнительному, подключенному в программе rim_light, групповому(multicast) IP адресу, что в свою очередь
 *  повлияет на диагностические сообщения программы rim.
 *
 *  !!!
 */

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#include "../../Includes/test/Wmcast_test.hpp"

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

#define N_COUNT   10
#define NANN      0xFFFFFFFF

#define RBUF_SIZE   65000		// размер приемного буфера сокета
#define RECVmax     RBUF_SIZE
#define viewItem    5

struct tmp_type_A {
	type_F zn;
	unsigned char kod_otkl :3, nd :1;
};

struct {
	// Заголовок данных сетевого пакета
//	packid id;              // n_vs_bl - номер внешней системы + номер энергоблока
	unsigned char id[4];	// n_block;     //Номер внешней системы
							// type;        //Тип массива
							// n_vs;        //номер списка
							// version;     //Номер версии списка обмена
	unsigned short index;	// Индекс первого параметра
	unsigned short num;		// Количество параметров
	time_t time;			// Время

//	char *data;             // Собственно данные
	unsigned char data[RECVmax];
} inpPACKET;




/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int main(int argc, char** argv) {

	int viewfrom = 1, vfrom;
	int bytes, all = 0;





	if (argc > 1) {
		if (atoi(argv[1]) >= 0) {
			viewfrom = atoi(argv[1]);
		} else if (atoi(argv[1]) < 0) {
			all = 1;
			viewfrom = abs(atoi(argv[1]));
		}
		if (*argv[1] == 'a' || *argv[1] == 'A') {
			all = 1;
			viewfrom = 1;
		}
	}





	// Configure the default severity Level of new Channel objects
#ifndef NDEBUG
	Log::Manager::setDefaultLevel(Log::Log::eNotice);
#else
	Log::Manager::setDefaultLevel(Log::Log::eDebug);
#endif
	Log::Manager::setDefaultLevel(Log::Log::eNotice);



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
	Log::Logger logger("Wmcast_recv");

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

	Wmcast mcRecv;
//	mcRecv.logger.setLevel(Log::Log::eDebug);
	mcRecv.logger.setLevel(Log::Log::eNotice);

	// Принять сообщение для многоадресной группы
	//===========================================
	logger.info() << "Receive";

	while (1) {

		tmp_type_A packA;
		type_D packD;
//		type_F packF;
//		type_I packI;
//		type_K packK;
//		type_R packR;

		bzero(&inpPACKET, sizeof(inpPACKET));

		mcRecv.Receive_From_Ether(
				sizeof(inpPACKET),
				(unsigned char*)&inpPACKET,
				cfg.McastIfAddr,
				cfg.McastGrAddr,
				cfg.McastGrPort);

		int nByte = mcRecv.GetBytesReceived();

//		logger.info() << "Received " << nByte << " bytes";

/*
		// применяем фильтр FILTER_IP
		if (strlen(ARG.FIP)) {
			char fip[20] = "";
			strcpy(fip, inet_ntoa(*(struct in_addr*) &sda.sin_addr));
			if (strcmp(fip, ARG.FIP) != 0)
				continue;
		}
*/

		// применяем фильтр PackNetHead (FILTER_ID)
		//==========================================
		if (cfg.PackNetHead.length() != 0) {
			char got[20] = "";
			sprintf(got, "%03d%c%03d.%03d", inpPACKET.id[0], inpPACKET.id[1], inpPACKET.id[2], inpPACKET.id[3]);
			std::string sgot = got;
			int x = sgot.compare(cfg.PackNetHead);

			if (x != 0) {
				//std::cout << sgot << " is not equal to " << cfg.PackNetHead << std::endl;
				continue;
			}

			//if (x > 0)
			//	std::cout << sgot << " is greater than " << cfg.PackNetHead << std::endl;
			//else
			//	std::cout << cfg.PackNetHead << " is greater than " << cfg.PackNetHead << std::endl;
		}

		// применяем фильтр FilterIndex (FILTER_FIRST)
		//=============================================
		if (cfg.FilterIndex != -1) {
			if (inpPACKET.index != cfg.FilterIndex)
				continue;
		}

		unsigned int datasize = 0;

		switch (inpPACKET.id[1]) {
		case 'A': { datasize = inpPACKET.num * sizeof(type_A);    break; }
		case 'B': { datasize = ((inpPACKET.num - 1) / 8 + 1) * 2; break; }
		case 'D': { datasize = inpPACKET.num * sizeof(type_D);    break; }
//		case 'F': { datasize = inpPACKET.num * sizeof(type_F);    break; }
//		case 'I': { datasize = inpPACKET.num * sizeof(type_I);    break; }
//		case 'K': { datasize = inpPACKET.num * sizeof(type_K);    break; }
//		case 'R': { datasize = inpPACKET.num * sizeof(type_R);    break; }
		}

		printf("\n{%03d%c%03d.%03d|index=%u n=%3u%s",
				inpPACKET.id[0], inpPACKET.id[1], inpPACKET.id[2], inpPACKET.id[3],
				inpPACKET.index, inpPACKET.num,
				(inpPACKET.index == 0) ? " EVEN}" : "} ");

		if (inpPACKET.index == 0) {
			char *ch = (char*) &inpPACKET;
			unsigned short num = *(unsigned short*) (&ch[nByte - 2]);
			printf("(num=%u) ", num);
		}

		if (nByte == datasize + 12)
			printf("bytes=%d ", nByte);
		else
			printf("bytes=%d ", nByte);

		if (inpPACKET.time == 0)
			printf("time[*]\n");
		else
			printf("time=%s", asctime(localtime(&inpPACKET.time)));

		int print_count = viewItem, p_count;

		if (inpPACKET.num < print_count)
			print_count = inpPACKET.num;

		print_count = viewfrom + print_count;
		vfrom = viewfrom;

		if (all == 1)
			print_count = inpPACKET.num;
		if (inpPACKET.index == 0) {
			print_count = inpPACKET.num;
			vfrom = 1;
		}

		p_count = print_count;

		if (inpPACKET.id[1] == 'B' && inpPACKET.index != 0) {
			if (viewfrom / 8 * 8 == viewfrom)
				vfrom = viewfrom / 8;
			else
				vfrom = viewfrom / 8 + 1;

			if (p_count > inpPACKET.num) {
				p_count = inpPACKET.num;
			}

			if (all == 1) {
				p_count = inpPACKET.num;
			}

			if (p_count / 8 * 8 == p_count)
				p_count = p_count / 8;
			else
				p_count = p_count / 8 + 1;
		}

		for (int i = vfrom; (i <= p_count) & (vfrom > 0); i++) {
			printf("%3s", " ");

			//=============================================================================
			//=============================================================================
			if (inpPACKET.id[1] == 'A') {
				packA = ((tmp_type_A*) &inpPACKET.data[0])[i - 1];

				if (packA.zn.i != 0)
					printf("[%u]%f=0x%x=%d(%u)", i, packA.zn.zn, packA.zn.i, packA.zn.i, packA.zn.u);
				else
					printf("[%u]%f", i, packA.zn.zn);

				printf(" nd=%d kod_otkl=%d\n", packA.nd, packA.kod_otkl);
				continue;
			}

			//=============================================================================
			//=============================================================================
			if (inpPACKET.id[1] == 'B') {
				type_B dataBzn = *(type_B*) &(inpPACKET.data[i - 1]);

				type_B dataBnd = *(type_B*) &(inpPACKET.data[(inpPACKET.num - 1) / 8 + i]);
				//***** zn ********************
				printf("zn[%d-%d]=%u%u%u%u%u%u%u%u=0x%02x=%u ", i * 8,
						(i - 1) * 8 + 1, dataBzn.bits.b7, dataBzn.bits.b6,
						dataBzn.bits.b5, dataBzn.bits.b4, dataBzn.bits.b3,
						dataBzn.bits.b2, dataBzn.bits.b1, dataBzn.bits.b0,
						dataBzn.byte, inpPACKET.data[i - 1]);
				//***** nd ********************
				printf("nd[%d-%d]=%u%u%u%u%u%u%u%u=0x%02x=%d\n", i * 8,
						(i - 1) * 8 + 1, dataBnd.bits.b7, dataBnd.bits.b6,
						dataBnd.bits.b5, dataBnd.bits.b4, dataBnd.bits.b3,
						dataBnd.bits.b2, dataBnd.bits.b1, dataBnd.bits.b0,
						dataBnd.byte,
						inpPACKET.data[(inpPACKET.num - 1) / 8 + i]);
				continue;
			}

			//=============================================================================
			//=============================================================================
			if (inpPACKET.id[1] == 'D') {
				type_B tmpB;
				tmpB = *(type_B*) &(inpPACKET.data[i - 1]);
				packD = *(type_D*) &(inpPACKET.data[i - 1]);
				printf("[%u]byte=%3u   ", i, *(unsigned char*) &packD);
				printf("bits=%u%u%u%u%u%u%u%u (nd=%u sost=%u%u%u)\n",
						tmpB.bits.b7, tmpB.bits.b6, tmpB.bits.b5,
						tmpB.bits.b4, tmpB.bits.b3, tmpB.bits.b2,
						tmpB.bits.b1, tmpB.bits.b0, tmpB.bits.b3,
						tmpB.bits.b2, tmpB.bits.b1, tmpB.bits.b0);
				continue;
			}

		} // for

	} // while

	logger.info() << "OK.";

	return EXIT_SUCCESS;
}
