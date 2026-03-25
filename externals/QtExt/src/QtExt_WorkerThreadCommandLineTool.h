#ifndef QtExtWorkerThreadCommandLineToolH
#define QtExtWorkerThreadCommandLineToolH

#include <QObject>
#include <QProcess>

#include "QtExt_WorkerThread.h"
#include "QtExt_global.h"

namespace QtExt {

/*! A worker thread that encapsulates the execution of an external program while the user interface
	waits for its completion.

	\code
	// construct progress dialog for worker thread
	QtExt::WorkerThreadProgressDialog dlg(this, tr("VICUS simulation"));
	// construct worker thread object
	QtExt::WorkerThreadCommandLineTool * worker = new WorkerThreadCommandLineTool(solverExecutable, commandLineArgs);
	// run the job in worker thread, showing a blocking progress bar in the UI
	dlg.run(worker);
	// retrieve result
	bool success = (worker->m_exitCode == 0);
	\endcode
*/
class QtExt_EXPORT WorkerThreadCommandLineTool : public QtExt::WorkerThread {
	Q_OBJECT
public:
	explicit WorkerThreadCommandLineTool(QString executable, QStringList commandlineArgs) :
		QtExt::WorkerThread(), m_executable(executable), m_commandlineArgs(commandlineArgs)
	{}

	~WorkerThreadCommandLineTool();

	// WorkerThread interface
	unsigned int max() const override;

	QString		m_executable;
	QStringList m_commandlineArgs;
	QString		m_task;

	/*! Will hold exit code of thread. */
	int			m_exitCode = 0;
	/*! Process error flag of type QProcess::ProcessError. */
	int			m_processError;

protected:
	// QThread interface
	void run() override;

private slots:
	void onReadyRead();
	void onFinished(int exitCode);
	void onErrorOccurred(QProcess::ProcessError error);

private:
	QProcess *m_p = nullptr;

};

} // namespace QtExt

#endif // QtExtWorkerThreadCommandLineToolH
