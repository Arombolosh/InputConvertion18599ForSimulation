#ifndef QTEXT_WorkerThreadProgressDialogH
#define QTEXT_WorkerThreadProgressDialogH

#include <QDialog>
#include <QWidget>

#include <memory>

#include "QtExt_global.h"

class QTimer;
class QProgressBar;
class QLabel;

namespace QtExt {

class WorkerThread;

/*! This is a progress dialog that manages a worker thread.
	Use it like:
	\code

	// create instance of own worker thread, a class derived from QtExt::WorkerThread
	MyWorkerThread * worker = new MyWorkerThread;

	// create modal dialog
	WorkerThreadProgressDialog dlg(this, tr("Running long simulation");

	// executes the thread and shows progress dialog with progress messages
	int result = dlg.run(worker);
	// result may be QDialog::Rejected when user has canceled the dialog
	\endcode

	Dialog is shown after 500 ms delay time, so on very fast processes, the dialog won't bother the user.
	When the dialog is configured to stay open at end (setAutoClose(false)), the the dialog will always be shown.
	When user tries to close the dialog, a message box is popped up, asking user to "cancel the process", which
	is - when confirmed - the same as hitting "cancel".
*/
class QtExt_EXPORT WorkerThreadProgressDialog : public QDialog {
	Q_OBJECT
public:
	WorkerThreadProgressDialog(QWidget * parent, QString title = QString());

	/*! Release worker on destruction. */
	~WorkerThreadProgressDialog();

	/*! Releases worker thread to caller and transfers ownership to caller. */
	WorkerThread * releaseWorker();

	/*! Starts worker thread and shows dialog after 500 ms.
		\param worker Pointer to worker thread. If this is not the same worker as in a previous call to run,
			the old worker is deleted and replaced by the new worker.
	*/
	int run(WorkerThread * worker);

	/*! Set to true (the default), if dialog should be closed when execution of worker is finished. */
	bool			m_autoClose = true;

	/*! Seconds to wait for worker thread to finish/stop normally, before killing the thread. */
	unsigned int	m_killTimout = 10;

protected:
	void closeEvent(QCloseEvent * event) override;

private slots:
	/*! Called from worker thread when there is a new task or step. */
	void onProgress(int stepNr, QString currentTask);

	/*! Triggered when the initial 500 ms have expired and the dialog shall be shown. */
	void onStartupTimerExpired();

	/*! Triggered when user clicks cancel.
		Sets stop-requested flag in worker and starts kill timer.
	*/
	void onCancelClicked();

	/*! Triggered when work has finished its work.
		Closes the dialog.
	*/
	void onWorkerFinished();

private:

	void stopWorkerAndCancel();

	/*! Our worker thread, must be set before running the dialog.
		Dialog owns worker and releases worker on destruction.
	*/
	WorkerThread	*m_worker = nullptr;

//	QTimer			*m_startupTimer = nullptr;
	QProgressBar	*m_progressBar = nullptr;
	QLabel			*m_currentTaskLabel = nullptr;

};

} // namespace QtExt

#endif // QTEXT_WorkerThreadProgressDialogH
