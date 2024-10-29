#include "RecvUdp.h"

int main(int argc, char *argv[]) {
	QApplication App(argc, argv);
	App.setApplicationName("QRecvUdp");
	App.setOrganizationName("SimSU3");

	QRecvUdp Window;
	Window.show();

	return App.exec();
}
