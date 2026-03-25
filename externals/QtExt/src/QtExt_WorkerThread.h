#ifndef QTEXT_WorkerThreadH
#define QTEXT_WorkerThreadH

#include <QThread>
#include <QObject>

#include "QtExt_global.h"

namespace QtExt {

/*! A specialized QThread with feedback to a model progress dialog in the GUI thread (see WorkerThreadProgressDialog).

	You need to re-implement the functions max() and run() and also emit the signal progress() when your work progresses.
	Inside your run() function, you should monitor the variable m_stopRequested and stop your work internally, when requested externally (i.e. when
	user cancels the progress dialog).
*/
class QtExt_EXPORT WorkerThread : public QThread {
	Q_OBJECT
public:
	explicit WorkerThread(QObject *parent = nullptr);

	/*! Re-implemented to return the number of steps in the progress.
		Return 0 to signal a process with unknown number of steps.
		Progress dialog will then show a knight-rider type of progress bar.

		You may implement a worker loop without event loop, like:
		\code
			for (int i=0; i<200000; ++i) {
				// do long work

				// check if work was canceled
				if (m_stopRequested)
					return;
			}
		\endcode

		Or you can use an event loop:
		\code
			QTimer::singleShot(0, this, MyWorker::doWork); // in doWork() emit new worker signals
			exec();
		\endcode

	*/
	virtual unsigned int max() const = 0;

	/*! Returns true if user pressed cancel in dialog. */
	bool wasAborted() const { return m_stopRequested; }

signals:
	void progress(int stepNr, QString message);

public slots:

	/*! Connected to signal from GUI thread when worker thread should stop work gracefully.
		The GUI thread may force-stop the thread, but in this case loose any success information on the
		outcome of done work.
	*/
	virtual void stop() { m_stopRequested = true; }

protected:

	/*! This variable will be set to true in slot stop(). */
	bool m_stopRequested = false;
};

} // namespace QtExt

#endif // QTEXT_WorkerThreadH
