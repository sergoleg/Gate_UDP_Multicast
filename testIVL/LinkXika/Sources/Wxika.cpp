/*
 * Wxika.cpp
 *
 *  Created on: 26.06.2019 - 30.01.2020
 *      Author: Oleg Sergeev
 */

#include "Wxika.hpp"
#include <LoggerCpp.h>

/********************************************************************
 *
 ********************************************************************/
static void puthex(long n, unsigned int digits, char *buf, unsigned int pos) {
	if (digits > 1)
		puthex(n / 16, digits - 1, buf, pos);
	buf[pos + digits - 1] = "0123456789abcdef"[n % 16];
} // puthex

/********************************************************************
 *
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

/********************************************************************
 * format_out(std::cout, 6, 9, ' ', -0.152454345) << '\n';
 ********************************************************************/
std::ostream& format_out(std::ostream &os, int precision, int width, char fill,
		double value) {
	return os << std::fixed << std::setprecision(precision) << std::setw(width)
			<< std::setfill(fill) << value;
} // format_out

/********************************************************************
 * Инициализировать списки параметров
 ********************************************************************/
int Wxika::Init(std::map<std::string, Package_entry> inSendPackages) {

	// Развернем Пакеты параметров для многоадрессной передачи симулятором в списки ИВС
	//
	// Конфигурационный файл удобнее писать в Пакетах, а передатчик удобнее писать в Списках
	//---------------------------------------------------------------------------------------
	std::map<std::string, Package_entry>::iterator iitt;
	for (iitt = inSendPackages.begin(); iitt != inSendPackages.end(); iitt++) {
		for (int ii = 0; ii < iitt->second.Lists.size(); ii++) {
			List_entry e = List_entry();

			// Конфигурации мультиадресной рассылки
			e.mcast.ethIP = iitt->second.MulticastIfIp;
			e.mcast.groupIP = iitt->second.MulticastAddress;
			e.mcast.groupPORT = iitt->second.MulticastPort;

			// Заголовок данных сетевого пакета
			if (ParseFileName(iitt->second.Lists.at(ii), &e.id) != EXIT_SUCCESS)
				return EXIT_FAILURE;

			// Разобрать список (файл) в перечень параметров с индексами
			if (ParseFile(iitt->second.ListHomeDir, iitt->second.Lists.at(ii),
					&e.CIS_PntName) != EXIT_SUCCESS)
				return EXIT_FAILURE;

			// Все в порядке - добавляем список к словарю
			Lists[iitt->second.Lists.at(ii)] = e;
		}
	}

	return (EXIT_SUCCESS);
} // Init

/********************************************************************
 * Разобрать имя списка (файла) в заголовок данных сетевого пакета
 ********************************************************************/
int Wxika::ParseFileName(std::string const &in, packid *out) {
	if (in.size() != 12) {
		logger.critic() << "Invalid file name [" << in << "] size = "
				<< in.size() << " != 12";
		return (EXIT_FAILURE);
	}

	/////////////////////////////////////////////////////////////////////
	//  0   123      4     567    8 901
	// <rs><n_vs_bl><type><n_spis>.<n_ver>
	/////////////////////////////////////////////////////////////////////

	int i;

	if (sscanf(in.substr(1, 3).c_str(), "%d", &i) != 1) {
		logger.critic() << "Unexpected 2-4 '" << in[1] << in[2] << in[3]
				<< "' characters in file name [" << in
				<< "], numeric characters expected";
		return (EXIT_FAILURE);
	} else
		out->n_vs_bl = i;	// номер внешней системы + номер энергоблока

	if (!((in[4] == 'A') or (in[4] == 'B'))) {
		logger.critic() << "Unexpected 5 character '" << in[4]
				<< "' in file name [" << in
				<< "], character 'A' or 'B' is expected";
		return (EXIT_FAILURE);
	} else
		out->type = in[4];	// код A B (типа данных)

	if (sscanf(in.substr(5, 3).c_str(), "%d", &i) != 1) {
		logger.critic() << "Unexpected 6-8 '" << in[5] << in[6] << in[7]
				<< "' characters in file name [" << in
				<< "], numeric characters expected";
		return (EXIT_FAILURE);
	} else
		out->n_spis = i;	// номер списка параметров

	if (!(in[8] == '.'))
		logger.warning() << "Unexpected 9 character '" << in[8]
				<< "' in file name [" << in << "], character '.' is expected";

	if (sscanf(in.substr(9, 3).c_str(), "%d", &i) != 1) {
		logger.critic() << "Unexpected 10-12 '" << in[9] << in[10] << in[11]
				<< "' characters in file name [" << in
				<< "], numeric characters expected";
		return (EXIT_FAILURE);
	} else
		out->n_ver = i;	// номер версии списка обмена

	return (EXIT_SUCCESS);
} // ParseFileName

/********************************************************************
 * Разобрать список (файл) в перечень параметров с индексами
 ********************************************************************/
int Wxika::ParseFile(std::string const &prefix, std::string const &input,
		std::map<int, std::string> *out) {
	std::string fname = prefix + input;
	std::ifstream in(fname.c_str());

	if (!in) {
		logger.critic() << "I/O error while reading file " << fname;
		return (EXIT_FAILURE);
	}

	std::string line;
	while (std::getline(in, line)) {
		removeSpaces(line);

		if (line.size() > 0) {

			if (line[0] != '#') {
				std::vector<std::string> tokens;
				tokens = split(line.c_str(), '\t');

				if (tokens.size() >= 2) {
					int number = std::atoi(tokens[0].c_str());// индекс параметра
					std::string pnt_name = tokens[1];			// имя параметра
					(*out)[number] = pnt_name;
				}
			}
		}
	}

	in.close();

	return (EXIT_SUCCESS);
} // ParseFile

/********************************************************************
 * Обновить динамические данные из SHM
 ********************************************************************/
void Wxika::UpdatePntValQ(Wshm& pshm) {

	// Пробежимся по спискам передачи
	std::map<std::string, List_entry>::iterator lst_it;
	for (lst_it = Lists.begin(); lst_it != Lists.end(); lst_it++) {

		// Пробежимся по параметрам из списка передачи
		std::map<int, std::string>::iterator pnt_it;

		// Первый (минимальный) порядковый номер в списке
		pnt_it = lst_it->second.CIS_PntName.begin();
		lst_it->second.smpl_F_index = pnt_it->first;

		// Последний (максимальный) порядковый номер в списке
		pnt_it = lst_it->second.CIS_PntName.end();
		pnt_it--;
		lst_it->second.smpl_E_index = pnt_it->first;

		// Количество параметров в сетевом пакете
		lst_it->second.smpl_C_index = lst_it->second.smpl_E_index - lst_it->second.smpl_F_index + 1;

		// Изменить длину Pnt_Sample - Списока блоков данных параметров
		lst_it->second.Pnt_Sample.resize(lst_it->second.smpl_E_index);

		// Инициализируем плохим качеством Pnt_Sample - Список блоков данных параметров
		for (int i = 0; i < lst_it->second.Pnt_Sample.size(); i++) {
			pt_sample smpl;
			smpl.quality = BAD;
			switch (lst_it->second.id.type) {
			case 'A':
				smpl.type = 1;
				smpl.value.as_1_float = 99999.0;
				break;
			case 'B':
				smpl.type = 3;
				smpl.value.as_3_boolean = 1;
				break;
			default:
				smpl.type = 2;
				smpl.value.as_2_int = 1;
				break;
			} // switch
			lst_it->second.Pnt_Sample[i] = smpl;
		}

		// Инициализируем Pnt_Sample из SHM
		for (pnt_it = lst_it->second.CIS_PntName.begin(); pnt_it != lst_it->second.CIS_PntName.end(); pnt_it++) {
			lst_it->second.SHM_PntValQ[pnt_it->first] = pshm.get_point_value(pnt_it->second);
			lst_it->second.Pnt_Sample[(pnt_it->first - 1)] = pshm.get_point_value(pnt_it->second);
		}

	}

} // UpdatePntValQ

/********************************************************************
 * Печать динамических данных
 ********************************************************************/
int Wxika::Print_SHM_PntValQ() {

	// Пробежимся по спискам передачи
	std::map<std::string, List_entry>::iterator lst_it;
	for (lst_it = Lists.begin(); lst_it != Lists.end(); lst_it++) {

		// Пробежимся по параметрам из списка передачи
		std::map<int, std::string>::iterator pnt_it;
		for (pnt_it = lst_it->second.CIS_PntName.begin();
				pnt_it != lst_it->second.CIS_PntName.end(); pnt_it++) {

			pt_sample smpl = lst_it->second.SHM_PntValQ[pnt_it->first];
			switch (smpl.type) {
			case 1:	// Floating Point Value From Shared Memory
				std::cout << lst_it->first << '\t' << pnt_it->first << '\t'
						<< pnt_it->second << '\t' << smpl.value.as_1_float
						<< "(" << smpl.quality << ")" << std::endl;
				break;
			case 2: // Short Integer Value From Shared Memory
				std::cout << lst_it->first << '\t' << pnt_it->first << '\t'
						<< pnt_it->second << '\t' << +smpl.value.as_2_int << "("
						<< smpl.quality << ")" << std::endl;
				break;
			case 3: // Byte Integer Value From Shared Memory
				std::cout << lst_it->first << '\t' << pnt_it->first << '\t'
						<< pnt_it->second << '\t' << +smpl.value.as_3_boolean
						<< "(" << smpl.quality << ")" << std::endl;
				break;
			default:
				break;
			} // switch

		}
	}

	return (EXIT_SUCCESS);
} // PrintPntValQ

/********************************************************************
 * Печать динамических данных
 ********************************************************************/
int Wxika::Print_Pnt_Sample(
		std::map<std::string, List_entry>::iterator lst_it) {

	std::cout << "\n" << lst_it->first << "    if= "
			<< lst_it->second.mcast.ethIP << ", addr= "
			<< lst_it->second.mcast.groupIP << ", port= "
			<< lst_it->second.mcast.groupPORT

			<< ", nFirst = " << lst_it->second.smpl_F_index << ", nEnd = "
			<< lst_it->second.smpl_E_index << ", nCount = "
			<< lst_it->second.smpl_C_index << ", type = "
			<< lst_it->second.id.type << std::endl;

	for (int i = (lst_it->second.smpl_F_index - 1);
			i < lst_it->second.Pnt_Sample.size(); i++) {

		switch (lst_it->second.Pnt_Sample[i].type) {

		case 1:	// Floating Point Value From Shared Memory
			std::cout << (i + 1) << '\t' << std::setw(20) << std::setfill('_')
					<< lst_it->second.CIS_PntName[i + 1] << '\t'
					<< lst_it->second.Pnt_Sample[i].value.as_1_float << "("
					<< lst_it->second.Pnt_Sample[i].quality << ")" << std::endl;
			break;

		case 2: // Short Integer Value From Shared Memory
			std::cout << (i + 1) << '\t' << std::setw(20) << std::setfill('_')
					<< lst_it->second.CIS_PntName[i + 1] << '\t'
					<< +lst_it->second.Pnt_Sample[i].value.as_2_int << "("
					<< lst_it->second.Pnt_Sample[i].quality << ")" << std::endl;
			break;

		case 3: // Byte Integer Value From Shared Memory
		{
			// расчет смещения до значения
			int dataByteOffset = (int) ((i
					- ((int) lst_it->second.smpl_F_index - 1)) / 8);
			// расчет смещения до недостоверности
			int qualByteOffset = (int) ((int) lst_it->second.Pnt_Sample.size()
					/ 8) + 1 + dataByteOffset;
			// расчет № бита в байте
			int bitNumber = (int) ((i - ((int) lst_it->second.smpl_F_index - 1))
					% 8);

			std::cout << (i + 1) << '\t' << std::setw(20) << std::setfill('_')
					<< lst_it->second.CIS_PntName[i + 1] << '\t'
					<< +lst_it->second.Pnt_Sample[i].value.as_3_boolean << "("
					<< lst_it->second.Pnt_Sample[i].quality << ")" << '\t'
					<< "vOFST= " << dataByteOffset << ", " << "qOFST= "
					<< qualByteOffset << ", " << "bit= " << bitNumber
					<< std::endl;
		}
			break;

		default:
			break;
		} // switch

	}

	return (EXIT_SUCCESS);
} // Print_Pnt_Sample

/********************************************************************
 * Передача динамических динамических данных
 ********************************************************************/
int Wxika::SendPntValQ(Wmcast& pmcast) {

	bool sendFlag = true;

	// Пробежимся по спискам передачи
	//================================
	std::map<std::string, List_entry>::iterator lst_it;
	for (lst_it = Lists.begin(); lst_it != Lists.end(); lst_it++) {

		// Печать для отладки
		//======================================
		if (logger.getLevel() == Log::Log::eDebug)
			Print_Pnt_Sample(lst_it);

		// Конфигурации мультиадресной рассылки
		//======================================
		std::string ethIP   = lst_it->second.mcast.ethIP;		// - local IP address of interface
		std::string groupIP = lst_it->second.mcast.groupIP;		// - IP multicast address of group
		int groupPORT       = lst_it->second.mcast.groupPORT;	// - and port

		// до 29.01.2020
		//============================================================
		int nFirst = lst_it->second.smpl_F_index;	// Первый (минимальный) порядковый номер в списке (Индекс первого параметра в блоке)
		int nEnd   = lst_it->second.smpl_E_index;	// Последний (максимальный) порядковый номер в списке (Индекс последнего параметра в блоке)
		int nCount = lst_it->second.smpl_C_index;	// Количество параметров в сетевом пакете
		char type  = lst_it->second.id.type;		// Код типа данных в сетевом пакете

		// после 29.01.2020 - по просьбе ХИКА
		//============================================================
		switch (type) {
		case 'B':
			nFirst = 1;								// Индекс первого параметра в блоке
			nCount = nEnd - nFirst + 1;				// Количество параметров в блоке
			break;
		case 'A':
			break;
		}

		// Вычислить размера сетевого пакета для передачи
		//================================================
		int memsize = get_datasize(nCount, type) + sizeof(PACKET);

		PACKET *pack;
		pack = (PACKET*) std::calloc(memsize, 1);

		if (!pack) {
			logger.critic() << "PACKET Allocation failure " << memsize << " bytes";
			return (EXIT_FAILURE);
		}

		// Заголовок данных сетевого пакета
		//==================================
		pack->id = lst_it->second.id;	// n_vs_bl - номер внешней системы + номер энергоблока
										// type    - код типа данных
										// n_spis  - номер списка параметров
										// n_ver   - номер версии списка обмена
		pack->index = nFirst;			// Индекс первого параметра в блоке или 0
		pack->num   = nCount;			// Количество параметров в блоке
		pack->time  = time(NULL);

		// Перепакуем структуру pt_sample в type_A или type_B
		//====================================================
		switch (pack->id.type) {

		case 'A':
		{
			for (int i = (pack->index-1); i < nEnd; i++) {
				pt_sample smplAnalog = lst_it->second.Pnt_Sample[i];	// Нумерация с 0
				type_A pntAnalog;
				pntAnalog.zn = smplAnalog.value.as_1_float;	// Текущее значение параметра
				pntAnalog.kod_otkl = 0;						// Код отклонения

				// Ожидаем только элементы данных аналогового параметра (тип A или SHM float = 1)
				// Тип параметра 1 - float, 2 - int, 3 - boolean или 0 - неопределенный
				//===============================================================================
				if ((smplAnalog.quality == GOOD) && (smplAnalog.type == 1)) {
					pntAnalog.nd = GOOD;	// Значение достоверно - Признак недостоверности
				} else {
					pntAnalog.nd = BAD;		// Значение недостоверно - Признак недостоверности
				}

				pntAnalog.reserv = 0;				// Резерв
				((type_A*) (&pack->data))[i - (pack->index-1)] = pntAnalog;
			} // for
			if (logger.getLevel() == Log::Log::eDebug) dumpmem((FILE *) stdout, (void*) pack, (unsigned int)memsize, (unsigned int)0);
		}
			break;

		case 'B':
		{
			for (int i = (pack->index-1); i < nEnd; i++) {
				unsigned short dataByteOffset = (int) ((i - (pack->index-1)) / 8);// расчет смещения до значения
				unsigned short qualByteOffset = (int) (pack->num / 8) + 1
						+ dataByteOffset;// расчет смещения до недостоверности
				unsigned short bitNumber = (int) ((i - (pack->index-1)) % 8);// расчет № бита в байте

				pt_sample smplDigital = lst_it->second.Pnt_Sample[i];// Нумерация с 0

				// Текущее значение параметра
				type_B data_block;
				data_block = ((type_B*) (&pack->data))[dataByteOffset];	// Считать из памяти
				if (bitNumber == 0) data_block.bits.b0 = smplDigital.value.as_3_boolean;
				if (bitNumber == 1) data_block.bits.b1 = smplDigital.value.as_3_boolean;
				if (bitNumber == 2) data_block.bits.b2 = smplDigital.value.as_3_boolean;
				if (bitNumber == 3) data_block.bits.b3 = smplDigital.value.as_3_boolean;
				if (bitNumber == 4) data_block.bits.b4 = smplDigital.value.as_3_boolean;
				if (bitNumber == 5) data_block.bits.b5 = smplDigital.value.as_3_boolean;
				if (bitNumber == 6) data_block.bits.b6 = smplDigital.value.as_3_boolean;
				if (bitNumber == 7) data_block.bits.b7 = smplDigital.value.as_3_boolean;
				((type_B*) (&pack->data))[dataByteOffset] = data_block;	// Записать в память

				// Признак недостоверности
				type_B qual_block;
				qual_block = ((type_B*) (&pack->data))[qualByteOffset];	// Считать из памяти

				// Ожидаем только элементы данных дискретных упакованных значений (тип B или SHM boolean = 3)
				// Тип параметра 1 - float, 2 - int, 3 - boolean или 0 - неопределенный
				//===========================================================================================
				if ((smplDigital.quality == GOOD) && (smplDigital.type == 3)) {
					// Значение достоверно - Признак недостоверности
					if (bitNumber == 0) qual_block.bits.b0 = GOOD;
					if (bitNumber == 1) qual_block.bits.b1 = GOOD;
					if (bitNumber == 2) qual_block.bits.b2 = GOOD;
					if (bitNumber == 3) qual_block.bits.b3 = GOOD;
					if (bitNumber == 4) qual_block.bits.b4 = GOOD;
					if (bitNumber == 5) qual_block.bits.b5 = GOOD;
					if (bitNumber == 6) qual_block.bits.b6 = GOOD;
					if (bitNumber == 7) qual_block.bits.b7 = GOOD;
				} else {
					// Значение недостоверно - Признак недостоверности - после 29.01.2020
					// GOOD = 0 — значение параметра достоверно
					// BAD  = 1 — значение параметра недостоверно
					if (bitNumber == 0) qual_block.bits.b0 = GOOD;//= BAD;
					if (bitNumber == 1) qual_block.bits.b1 = GOOD;//= BAD;
					if (bitNumber == 2) qual_block.bits.b2 = GOOD;//= BAD;
					if (bitNumber == 3) qual_block.bits.b3 = GOOD;//= BAD;
					if (bitNumber == 4) qual_block.bits.b4 = GOOD;//= BAD;
					if (bitNumber == 5) qual_block.bits.b5 = GOOD;//= BAD;
					if (bitNumber == 6) qual_block.bits.b6 = GOOD;//= BAD;
					if (bitNumber == 7) qual_block.bits.b7 = GOOD;//= BAD;
				}
				((type_B*) (&pack->data))[qualByteOffset] = qual_block;	// Записать в память

			} // for
			if (logger.getLevel() == Log::Log::eDebug)
				dumpmem((FILE *) stdout, (void*) pack, (unsigned int)memsize, (unsigned int)0);
		}
			break;

		default:
		{
			logger.critic() << "Неподдерживаемый тип сетевого пакета: " << pack->id.type;
			sendFlag = false;
		}
			break;

		} // switch

		bool retval = true;

		// Отправка пакета
		//================
		if (sendFlag)
			retval = pmcast.Send_To_Ether((unsigned char*) pack, (unsigned int) memsize, ethIP, groupIP, groupPORT);

		std::free(pack);

		if (sendFlag && !retval)
			return (EXIT_FAILURE);

	} // for

	return (EXIT_SUCCESS);

} // SendPntValQ
