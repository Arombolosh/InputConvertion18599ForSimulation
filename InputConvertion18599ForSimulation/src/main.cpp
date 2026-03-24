#include <QApplication>

#include <IBK_messages.h>
#include <IBK_MessageHandler.h>
#include <IBK_MessageHandlerRegistry.h>

#include "IC18599MainWindow.h"

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);

	// Setup message handler
	IBK::MessageHandler msgHandler;
	IBK::MessageHandlerRegistry::instance().setMessageHandler(&msgHandler);

	IBK::IBK_Message("InputConvertion18599ForSimulation started.\n", IBK::MSG_PROGRESS, "[main]", IBK::VL_STANDARD);

	IC18599MainWindow w;
	w.show();

	return a.exec();
}
