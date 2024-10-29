#ifndef RECVUDP_H
#define RECVUDP_H

#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include <QApplication>
#include <QtNetwork>
#include <QFileDialog>
#include "ui_RecvUdp.h"
#include "string.h"

class QRecvUdp: public QWidget {
	Q_OBJECT

public:
	QUdpSocket *pRecvSocket;
	QFile WriteFile;

public:
	QRecvUdp(QWidget *parent = 0);
	~QRecvUdp();

	bool Check();

public slots:
	void Process();
	void PrepareDatagram();
	void SelectWritePath();
	QBitArray bytesToBits(QByteArray bytes, int offst);

private:
	Ui::QRecvUdpClass ui;
};

#endif // RECVUDP_H
