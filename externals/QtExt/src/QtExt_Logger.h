#ifndef QtExt_LoggerH
#define QtExt_LoggerH

#include <fstream>

#include "QtExt_global.h"

namespace QtExt {

// TODO : CodeReview: IBK::Path for filename argument, IBK::open_ofstream() and
//        non-trivial functions to cpp file and only #include <iosfwd>
//        + Documentation!
class QtExt_EXPORT Logger {
public:
	static Logger& instance() {
		static Logger log;
		return log;
	}

	inline void set(const std::string& filename) {
		if(filename.empty()) {
			if(m_opened)
				m_out.close();
			m_opened = false;
		}
		else {
			m_out.open(filename);
			m_opened = m_out.is_open();
		}
	}

	template<typename T>
	inline Logger& operator<<(const T& msg) {
		if(m_opened) {
			m_out << msg << std::endl;
		}
		return *this;
	}

private:
	Logger() = default;

	bool m_opened = false; // TODO remove: redundant, m_out has a stream state to check
	std::ofstream m_out;

};

} // end namespace

#endif // QtExt_LoggerH
