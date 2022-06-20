#ifndef _STATS_H_
#define _STATS_H_

#include <vector>
#include <ostream>
#include <string>

namespace stats {
	typedef std::vector<unsigned char>	VUCHAR;

	class s_base {
	protected:
		const int	_n_streams,
				_i_width,
				_i_height;
		std::ostream	&_ostr;
	public:
		s_base(const int& n_streams, const int& i_width, const int& i_height, std::ostream& ostr) : 
		_n_streams(n_streams), _i_width(i_width), _i_height(i_height), _ostr(ostr) {
		}

		virtual void set_parameter(const std::string& p_name, const std::string& p_value) = 0;

		virtual void process(const int& ref_frame, VUCHAR& ref, const std::vector<bool>& v_ok, std::vector<VUCHAR>& streams) = 0;

		virtual ~s_base() {
		}
	};

	extern s_base* get_analyzer(const char* id, const int& n_streams, const int& i_width, const int& i_height, std::ostream& ostr);
}

#endif /*_STATS_H_*/

