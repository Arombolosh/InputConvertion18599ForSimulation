#include <QApplication>

#include <IBK_messages.h>
#include <IBK_MessageHandler.h>
#include <IBK_MessageHandlerRegistry.h>

#include <QtExt_Directories.h>
#include <QtExt_LanguageHandler.h>

#include "IC18599MainWindow.h"

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	a.setOrganizationName("IBK");
	a.setApplicationName("IC18599");

	// Setup message handler
	IBK::MessageHandler msgHandler;
	IBK::MessageHandlerRegistry::instance().setMessageHandler(&msgHandler);

	IBK::IBK_Message("InputConvertion18599ForSimulation started.\n", IBK::MSG_PROGRESS, "[main]", IBK::VL_STANDARD);

	// Setup directories and language handler
	QtExt::Directories::appname = "IC18599";
	QtExt::Directories::devdir = "InputConvertion18599ForSimulation";

	QtExt::LanguageHandler::setup("IBK", "IC18599", "IC18599");
	QString langId = QtExt::LanguageHandler::langId();
	QtExt::LanguageHandler::instance().installTranslator(langId);

	IC18599MainWindow w;
	w.show();

	return a.exec();
}
