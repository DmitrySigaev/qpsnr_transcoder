#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <string>
#include <iostream>

#define	LEVEL_LOG_ERROR		(0x01)
#define	LEVEL_LOG_WARNING	(0x02)
#define	LEVEL_LOG_INFO		(0x04)
#define	LEVEL_LOG_DEBUG		(0x08)

#define	__LOG(x)		if ( x & settings::LOG) std::cerr

#define LOG_ERROR 	__LOG(LEVEL_LOG_ERROR) << "[ERROR] "
#define LOG_WARNING	__LOG(LEVEL_LOG_WARNING) << "[WARNING] "
#define LOG_INFO 	__LOG(LEVEL_LOG_INFO) << "[INFO] "
#define LOG_DEBUG 	__LOG(LEVEL_LOG_DEBUG) << "[DEBUG] "

namespace settings {
	extern char		LOG;
	extern std::string	REF_VIDEO;
	extern int		MAX_FRAMES;
	extern int		SKIP_FRAMES;
	extern bool		SAVE_IMAGES;
	extern std::string	ANALYZER;
	extern bool		IGNORE_FPS;
	extern int		VIDEO_SIZE_W;
	extern int		VIDEO_SIZE_H;
}


#endif /*_SETTINGS_H_*/

