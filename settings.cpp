#include "settings.h"

namespace settings {
	char		LOG = 0x07;
	std::string	REF_VIDEO = "";
	int		MAX_FRAMES = -1;
	int		SKIP_FRAMES = -1;
	bool		SAVE_IMAGES = false;
	std::string	ANALYZER = "psnr";
	bool		IGNORE_FPS = false;
	int		VIDEO_SIZE_W = -1;
	int		VIDEO_SIZE_H = -1;
}
