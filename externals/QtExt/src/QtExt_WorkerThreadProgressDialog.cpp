#include "QtExt_WorkerThreadProgressDialog.h"

#include <QTimer>
#include <QDebug>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QCloseEvent>
#include <QMessageBox>

#include "QtExt_WorkerThread.h"

namespace QtExt {

WorkerThreadProgressDialog::WorkerThreadProgressDialog(QWidget * parent, QString title) :
	QDialog(parent)
{
	setWindowTitle(title);
	QVBoxLayout * lay = new QVBoxLayout;
	m_currentTaskLabel = new QLabel(this);
	m_progressBar = new QProgressBar(this);
	lay->addWidget(m_currentTaskLabel);
	lay->addWidget(m_progressBar);

	QDialogButtonBox * btnBox = new QDialogButtonBox(this);
	btnBox->setStandardButtons(QDialogButtonBox::Cancel);
	lay->addWidget(btnBox);
	setLayout(lay);

	connect(btnBox, &QDialogButtonBox::rejected, this, &WorkerThreadProgressDialog::onCancelClicked);
}


WorkerThreadProgressDialog::~WorkerThreadProgressDialog() {
	if (m_worker != nullptr && m_worker->isRunning()) {
		m_worker->terminate();
		m_worker->wait(1000);
	}
	delete m_worker;
}


WorkerThread * WorkerThreadProgressDialog::releaseWorker() {
	WorkerThread * w = m_worker;
	m_worker = nullptr;
	return w;
}


int WorkerThreadProgressDialog::run(WorkerThread * worker) {
	if (m_worker != worker) {
		delete m_worker;
		m_worker = worker;
		connect(worker, &WorkerThread::progress, this, &WorkerThreadProgressDialog::onProgress);
		connect(worker, &WorkerThread::finished, this, &WorkerThreadProgressDialog::onWorkerFinished);
	}


	// configure our progress bar
	m_progressBar->setMaximum( worker->max() );
	m_progressBar->setValue(0);

	// now start the worker thread
	m_worker->start();

	// and start the dialog's event loop
	int res = exec(); // show as modal dialog in own event loop

	return res;
}

// *** protected functions ***

void WorkerThreadProgressDialog::closeEvent(QCloseEvent * event) {
	if (m_worker != nullptr && m_worker->isRunning()) {
		if (QMessageBox::question(this, QString(), tr("Abort process?")) == QMessageBox::No) {
			event->ignore();
			return;
		}
		stopWorkerAndCancel();
	}
	event->accept();
}


// *** private slots ***

void WorkerThreadProgressDialog::onProgress(int stepNr, QString currentTask) {
	m_currentTaskLabel->setText(currentTask);
	m_progressBar->setValue(stepNr);
}


void WorkerThreadProgressDialog::onStartupTimerExpired() {
	qDebug() << "Startup timer expired.";
	setVisible(false);
}


void WorkerThreadProgressDialog::onCancelClicked() {
	qDebug() << "Cancel pressed.";
	stopWorkerAndCancel();
}


void WorkerThreadProgressDialog::onWorkerFinished() {
	qDebug() << "Worker has finished";
	if (m_autoClose)
		close();
}


void WorkerThreadProgressDialog::stopWorkerAndCancel() {
	m_worker->stop(); // signal thread to finish, this also records the "aborted"
	m_worker->quit(); // encode signal to thread's event queue to finish it
	// in case that the thread doesn't have an event loop, we wait for it to finish
	m_worker->wait(m_killTimout*1000);
	reject();
}

} // namespace QtExt


