#include "RecvUdp.h"

#pragma pack(1)

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
 *   <rs><SrcID><DataType><LstNum>.<LstVersion>
 *
 * Например: S143A034.099 – список «034» версии «.099» экспорта аналоговых
 * параметров от внешней системы «14» энергоблока «3».
 *
 */
struct DataHeader {
    unsigned char SrcID;		//глобальный идентификатор источника данных
    char DataType;				//тип данных
    unsigned char LstNum;		//номер (код) списка пар-ров
	unsigned char LstVersion;	//номер (код) версии списка пар-ров
	short StartIndex;
	short IndexCnt;
	long DataTime;
};

struct AnalogIVS {
	float Value;
	unsigned char KO :3;
	unsigned char KH :1;
	unsigned char Reserv :4;
};

struct DiscretIVS {
	unsigned char Value :3;
	unsigned char KH :1;
	unsigned char Reserv :4;
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
struct PackedIVS {
	union {
		struct {
			unsigned char b0 :1, b1 :1, b2 :1, b3 :1, b4 :1, b5 :1, b6 :1,
					b7 :1;
		} bits;
		unsigned char byte;	// (1 byte =  8 bit)
	};
};

#pragma pack()

/********************************************************************
 *
 ********************************************************************/
QRecvUdp::QRecvUdp(QWidget *parent) :
		QWidget(parent) {
	ui.setupUi(this);

	QRecvUdp::pRecvSocket = NULL;

	foreach(QNetworkInterface If, QNetworkInterface::allInterfaces()) {
		if(If.flags() & QNetworkInterface::CanMulticast)
		ui.pIfBox->addItem(If.humanReadableName());
	}

	connect(ui.pProcessBtn, SIGNAL(clicked()), this, SLOT(Process()));
	connect(ui.pSelectPathBtn, SIGNAL(clicked()), this,
			SLOT(SelectWritePath()));

	setLayout(ui.pMainLayout);

	QSettings Settings;
	Settings.beginGroup("Window");
	restoreGeometry(Settings.value("GeometryState").toByteArray());
	Settings.endGroup();

	Settings.beginGroup("Settings");

	ui.pIfBox->setCurrentText(Settings.value("Interface").toString());

	QString Host = Settings.value("Host").toString();
	if (!Host.isEmpty())
		ui.pHostEdit->setText(Host);

	QString Port = Settings.value("Port").toString();
	if (!Port.isEmpty())
		ui.pPortEdit->setText(Port);

	ui.pWriteCheck->setChecked(Settings.value("Write").toBool());

	QString WritePath = Settings.value("WritePath").toString();
	if (!WritePath.isEmpty())
		ui.pPathEdit->setText(WritePath);

	Settings.endGroup();
}

/********************************************************************
 *
 ********************************************************************/
QRecvUdp::~QRecvUdp() {
	if (WriteFile.isOpen())
		WriteFile.close();
	if (pRecvSocket)
		delete pRecvSocket;

	QSettings Settings;
	Settings.beginGroup("Window");
	Settings.setValue("GeometryState", saveGeometry());
	Settings.endGroup();

	Settings.beginGroup("Settings");
	Settings.setValue("Interface", ui.pIfBox->currentText());
	Settings.setValue("Host", ui.pHostEdit->text());
	Settings.setValue("Port", ui.pPortEdit->text());
	Settings.setValue("Write", ui.pWriteCheck->isChecked());
	Settings.setValue("WritePath", ui.pPathEdit->text());
	Settings.endGroup();
}

/********************************************************************
 *
 ********************************************************************/
void QRecvUdp::SelectWritePath() {
	QString WritePath = QFileDialog::getExistingDirectory(this,
			tr("Select write path"), ui.pPathEdit->text(),
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (WritePath.length())
		ui.pPathEdit->setText(WritePath);
}

/********************************************************************
 *
 ********************************************************************/
bool QRecvUdp::Check() {
	bool bRetValue = true;
	return bRetValue;
}

/********************************************************************
 *
 ********************************************************************/
void QRecvUdp::Process() {
	if (pRecvSocket) {
		if (WriteFile.isOpen()) {
			ui.pOutList->addItem(QString("Close file ") + WriteFile.fileName());
			WriteFile.close();
		}
		delete pRecvSocket;
		pRecvSocket = NULL;
		ui.pProcessBtn->setText("Start");
	} else if (Check()) {
		ui.pOutList->clear();

		QString If = ui.pIfBox->currentText();
		QString Host = ui.pHostEdit->text();
		QString Port = ui.pPortEdit->text();

		bool bStart = false;
		pRecvSocket = new QUdpSocket;
		if (pRecvSocket->bind(QHostAddress::AnyIPv4, Port.toUInt(),
				QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
			QHostAddress GroupAddress(Host);
			if (If.isEmpty())
				bStart = pRecvSocket->joinMulticastGroup(GroupAddress);
			else
				bStart = pRecvSocket->joinMulticastGroup(GroupAddress,
						QNetworkInterface::interfaceFromName(If));
		}

		if (bStart) {
			connect(pRecvSocket, SIGNAL(readyRead()), this,
					SLOT(PrepareDatagram()));
			ui.pProcessBtn->setText("Stop");
		} else {
			ui.pOutList->addItem(
					QString("Error : ") + pRecvSocket->errorString());
			delete pRecvSocket;
			pRecvSocket = NULL;
		}
	}
}

/********************************************************************
 *
 ********************************************************************/
QBitArray QRecvUdp::bytesToBits(QByteArray bytes, int offst) {
	//QBitArray bits(bytes.count()*8);
	QBitArray bits(8);
	// Convert from QByteArray to QBitArray
	//for (int i = 0; i < bytes.count(); ++i)
	for (int i = 0; i < 1; ++i)
		for (int b = 0; b < 8; ++b)
			bits.setBit(i * 8 + b, bytes.at(offst) & (1 << b));
	return bits;
}

/********************************************************************
 *
 ********************************************************************/
void QRecvUdp::PrepareDatagram() {
	while (pRecvSocket->hasPendingDatagrams()) {
		QByteArray Datagram;
		Datagram.resize(pRecvSocket->pendingDatagramSize());
		QHostAddress sender;
		quint16 senderPort;
		int RecvSize = pRecvSocket->readDatagram(Datagram.data(),
				Datagram.size(), &sender, &senderPort);
		if (RecvSize != -1) {
			QDataStream in(&Datagram, QIODevice::ReadOnly);
			DataHeader Header;
            if (in.readRawData((char*) &Header, sizeof(Header)) == sizeof(Header)) {

//                QString ListName = QString("%1%2%3")
//                        .arg((int) Header.SrcID, 3, 10, QLatin1Char('0'))
//                        .arg(Header.DataType)
//                        .arg((int) Header.LstNum, 3, 10, QLatin1Char('0'));

                QDateTime _CurTime = QDateTime::currentDateTime();

                QString ListName = QString("%1_%2_%3..%4")
                        .arg((int) Header.SrcID, 3, 10, QLatin1Char('0'))
                        .arg(Header.DataType)
                        .arg((int) Header.LstNum, 3, 10, QLatin1Char('0'))
                        .arg((int) Header.LstVersion, 3, 10, QLatin1Char('0'));

                QString ItemText = QString("Sender: ") + sender.toString();
                ItemText += ", ListName: S_" + ListName;

                ItemText += QString("   %1").arg(_CurTime.toString("yyyy-MM-dd   HH:mm:ss.zzz"));

                if (ui.pShortCheck->checkState() != Qt::Checked) {
                    ItemText += ", nFirst = " + QString::number(Header.StartIndex + 1);
                    ItemText += ", nEnd = "	+ QString::number(Header.StartIndex + Header.IndexCnt);
                    ItemText += ", nCount = " + QString::number(Header.IndexCnt);
                }
//                ItemText += ", type = ";

				//in.skipRawData(sizeof(Header));
				if (Header.DataType == 'A') {
//                    ItemText += "A";
                    ui.pOutList->addItem(ItemText);
                    ItemText = "";

					for (int ValuePos = 0; ValuePos < Header.IndexCnt;
							ValuePos++) {
						AnalogIVS aValue;
						if (in.readRawData((char*) &aValue, sizeof(aValue))
								== sizeof(aValue)) {

//							ItemText += (!ValuePos ? " : (" : "")
//									+ QString::number(aValue.Value) + "[кн="
//									+ QString::number(aValue.KH) + ".ко="
//									+ QString::number(aValue.KO) + "]"
//									+ (ValuePos == (Header.IndexCnt - 1) ?
//											")" : ", ");

							if (ui.pShortCheck->checkState() != Qt::Checked) {
								QString LineText = "\t"
									+ QString::number(Header.StartIndex + 1 + ValuePos)
									+ "\t" + "________\t"
									+ QString::number(aValue.Value) + "("
									+ QString::number(aValue.KH) + ")";
								ui.pOutList->addItem(LineText);
							}

						} else
							break;
					}
//					ui.pOutList->addItem(ItemText);

					//------------------------------------------------------------
				} else if (Header.DataType == 'D') {
//                    ItemText += "D";
                    ui.pOutList->addItem(ItemText);
                    ItemText = "";

					for (int ValuePos = 0; ValuePos < Header.IndexCnt;
							ValuePos++) {
						DiscretIVS dValue;
						if (in.readRawData((char*) &dValue, sizeof(dValue))
								== sizeof(dValue)) {
							ItemText += (!ValuePos ? " : (" : "")
									+ QString::number(dValue.Value & 0x1)
									+ "[кн=" + QString::number(dValue.KH) + "]"
									+ (ValuePos == (Header.IndexCnt - 1) ?
											")" : ", ");
						} else
							break;
					}
					ui.pOutList->addItem(ItemText);

					//------------------------------------------------------------
				} else if (Header.DataType == 'B') {
//                    ItemText += "B";
                    ui.pOutList->addItem(ItemText);
                    ItemText = "";

					unsigned short datasize = (unsigned short) ((Header.IndexCnt - 1) / 8 + 1) * 2;
//					ui.pOutList->addItem("Размер данных B пакета " + QString::number(datasize) + " байт");
					QByteArray dataB;
					dataB.resize(datasize);
//					ui.pOutList->addItem("Размер данных B пакета " + QString::number(sizeof(dataB)) + " байт");

					// Поток использовать не можем - нужен прямой доступ к байтам
					//===========================================================
					if (in.readRawData((char*) dataB.data(), sizeof(dataB)) == sizeof(dataB)) {
//						ui.pOutList->addItem("Блок данных: " + QString::number(sizeof(dataB)) + " байт [ " + dataB.toHex() + " ]");
					} else {
//						ui.pOutList->addItem("Не скопировано " + QString::number(sizeof(dataB)) + " байт");
						break;
					}

					for (int i = 0; i < Header.IndexCnt; i++) {

						// расчет смещения до значения
						unsigned short dataByteOffset = (unsigned short) (i / 8);
						// расчет смещения до недостоверности
						unsigned short qualByteOffset =
								(unsigned short) (Header.IndexCnt / 8) + 1
										+ dataByteOffset;
						// расчет № бита в байте
						unsigned short bitNumber = (unsigned short) (i % 8);

						// Текущее значение параметра
						QBitArray data_block = bytesToBits(dataB, dataByteOffset);
						// Признак недостоверности
						QBitArray qual_block = bytesToBits(dataB, qualByteOffset);

						if (ui.pShortCheck->checkState() != Qt::Checked) {
							QString LineText = "\t"
									+ QString::number(Header.StartIndex + 1 + i)
									+ "\t" + "________\t"
									+ QString::number(data_block[bitNumber]) + "("
									+ QString::number(qual_block[bitNumber]) + ")\t"
									+ "vOFST= " + QString::number(dataByteOffset)
									+ ", " + "qOFST= "
									+ QString::number(qualByteOffset) + ", "
									+ "bit= " + QString::number(bitNumber);
							ui.pOutList->addItem(LineText);
			            }
					}
				}
			}

			if (ui.pWriteCheck->checkState() == Qt::Checked) {
				bool bFileOpen = WriteFile.isOpen();
				if (!bFileOpen) {
					QDateTime CurTime = QDateTime::currentDateTime();
                    QString FileName = QString("GI_%1.DAT").arg(CurTime.toString("yyyyMMddHHmmsszzz"));
					QString WritePath = ui.pPathEdit->text();
					if (WritePath.isEmpty())
						WritePath = QDir::currentPath();
                    WriteFile.setFileName(WritePath + QDir::separator() + FileName);
					if (WriteFile.open(QIODevice::WriteOnly)) {
                        ui.pOutList->addItem(QString("Open file for write ") + WriteFile.fileName());
						bFileOpen = true;
					} else
						ui.pOutList->addItem(WriteFile.errorString());
				}
				if (bFileOpen) {
                    if (WriteFile.write((const char*) &RecvSize, sizeof(RecvSize)) != sizeof(RecvSize)) {
                        ui.pOutList->addItem(QString("Error write : ") + WriteFile.errorString());
                        ui.pOutList->addItem(QString("Close file ") + WriteFile.fileName());
						WriteFile.close();
					}
					if (WriteFile.write(Datagram) != Datagram.size()) {
                        ui.pOutList->addItem(QString("Error write : ") + WriteFile.errorString());
                        ui.pOutList->addItem(QString("Close file ") + WriteFile.fileName());
						WriteFile.close();
					}
				}
			}
		}
	}
}
