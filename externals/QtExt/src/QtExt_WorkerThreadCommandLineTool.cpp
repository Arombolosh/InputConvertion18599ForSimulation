#include "QtExt_WorkerThreadCommandLineTool.h"

#include <QTimer>
#include <QDebug>

namespace QtExt {

WorkerThreadCommandLineTool::~WorkerThreadCommandLineTool() {
	delete m_p;
}


unsigned int WorkerThreadCommandLineTool::max() const {
	return 0;
}


void WorkerThreadCommandLineTool::run() {
	m_p  = new QProcess;
	connect(m_p, &QProcess::readyReadStandardOutput, this, &WorkerThreadCommandLineTool::onReadyRead);
	connect(m_p, &QProcess::readyReadStandardError, this, &WorkerThreadCommandLineTool::onReadyRead);
	connect(m_p, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
	qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
	connect(m_p, &QProcess::errorOccurred, this, &WorkerThreadCommandLineTool::onErrorOccurred);
	m_p->start(m_executable, m_commandlineArgs);
	m_exitCode = exec(); // start event loop
	m_processError = m_p->error();
}


void WorkerThreadCommandLineTool::onReadyRead() {
//	QString text = m_p->readAllStandardOutput();
//	emit progress(0,text);
	emit progress(0,m_task);
}


void WorkerThreadCommandLineTool::onFinished(int exitCode) {
	qDebug() << "Worker thread's process finished.";
	exit(exitCode);
}


void WorkerThreadCommandLineTool::onErrorOccurred(QProcess::ProcessError error) {
	qCritical() << "Worker thread's process had error:" << error;
	exit(1);
}

} // namespace QtExt
