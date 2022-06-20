#include "qav.h"
#include "settings.h"
#include <stdexcept>
#include <sstream>


qav::qvideo::qvideo(const char* file, int _out_width, int _out_height) : frnum(0), videoStream(-1), out_width(_out_width),
out_height(_out_height), pFormatCtx(NULL), pCodecCtx(NULL), pCodec(NULL), pFrame(NULL), img_convert_ctx(NULL) 
{
	int ret;
	const char* pslash = strrchr(file, '/');
	if (pslash)
        	fname = pslash+1;
    	else
        	fname = file;

	if (avformat_open_input(&pFormatCtx, file, NULL, NULL) < 0) {
        	free_resources();
        	throw std::runtime_error("Can't open file");
    	}
    	if (avformat_find_stream_info(pFormatCtx, NULL)<0) {
        	free_resources();
        	throw std::runtime_error("Multimedia type not supported");
    	}
    	LOG_INFO << "File info for (" << file << ")" << std::endl;
    	av_dump_format(pFormatCtx, 0, file, false);

    	// find video stream (first)
    	for (unsigned int i=0; i<pFormatCtx->nb_streams; i++) {
        	if (AVMEDIA_TYPE_VIDEO == pFormatCtx->streams[i]->codecpar->codec_type) {
            		videoStream=i;
            		break;
        	}
    	}
    	if (-1==videoStream) {
        	free_resources();
        	throw std::runtime_error("Can't find video stream");
    	}
    	// Get a pointer to the codec context for the video stream
		const AVCodec* dec = avcodec_find_decoder(pFormatCtx->streams[videoStream]->codecpar->codec_id);
		if (!dec) {
			av_log(NULL, AV_LOG_ERROR, "Failed to find decoder for stream #%u\n", videoStream);
			free_resources();
			throw std::runtime_error("Failed to find decoder for stream");
		}

		pCodecCtx = avcodec_alloc_context3(dec);
		if (!pCodecCtx) {
			av_log(NULL, AV_LOG_ERROR, "Failed to allocate the decoder context for stream #%u\n", videoStream);
			free_resources();
			throw std::runtime_error("Failed to allocate the decoder context for stream ");
		}


		ret = avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStream]->codecpar);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Failed to copy decoder parameters to input decoder context "
				"for stream #%u\n", videoStream);
			free_resources();
			throw std::runtime_error("Failed to copy decoder parameters");
		}

		if (pCodecCtx->codec_type == AVMEDIA_TYPE_VIDEO)
			pCodecCtx->framerate = av_guess_frame_rate(pFormatCtx, pFormatCtx->streams[videoStream], NULL);
		/* Open decoder */
		ret = avcodec_open2(pCodecCtx, dec, NULL);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", videoStream);
			free_resources();
			throw std::runtime_error("Can't open codec for video stream");
		}

     	// alloacate data to extract frames
    	pFrame = av_frame_alloc(); // avcodec_alloc_frame();
    	if (!pFrame) {
        	free_resources();
        	throw std::runtime_error("Can't allocated frame for video stream");
    	}
    	// populate the out_width/out_height members
    	if (out_width > 0 && out_height > 0) {
        	LOG_INFO << "Output frame size for (" << file << ") is: " << out_width << 'x' << out_height << std::endl;
    	} else if (-1 == out_width && -1 == out_height) {
        	out_width = pCodecCtx->width;
        	out_height = pCodecCtx->height;
        	LOG_INFO << "Output frame size for (" << file << ") (default) is: " << out_width << 'x' << out_height << std::endl;
    	} else {
        	free_resources();
        	throw std::runtime_error("Invalid output frame size for video stream");
    	}
    	// just report if we're using a different video size
    	if (out_width!=pCodecCtx->width || out_height!=pCodecCtx->height)
        	LOG_WARNING << "Video (" << file <<") will get scaled: " << pCodecCtx->width << 'x' << pCodecCtx->height << " (in), " << out_width << 'x' << out_height << " (out)" << std::endl;

    	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, out_width, out_height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
    	if (!img_convert_ctx) {
        	free_resources();
        	throw std::runtime_error("Can't allocated sw_scale context");
    	}
}

qav::scr_size qav::qvideo::get_size(void) const {
	return scr_size(out_width, out_height);
}

int qav::qvideo::get_fps_k(void) const {
	if (pFormatCtx->streams[videoStream]->r_frame_rate.den)
		return 1000*pFormatCtx->streams[videoStream]->r_frame_rate.num/pFormatCtx->streams[videoStream]->r_frame_rate.den;
	return 0;
}

bool qav::qvideo::get_frame(std::vector<unsigned char>& out, int *_frnum, const bool skip) {
	out.resize(av_image_get_buffer_size(AV_PIX_FMT_RGB24, out_width, out_height, 1));
	AVPacket	packet;
	bool		is_read = false;
	int used;
	av_init_packet(&packet);
	while (av_read_frame(pFormatCtx, &packet)>=0) {
		if (packet.stream_index==videoStream) {
			int frameFinished = 0;
			// Decode video frame
#if 0	
			if(0 > avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet))
				return false;

				// FIXME: this function can crash with bad packets
				used = avcodec_decode_video2(video_ctx, frame, &got_frame, pkt);
#else
			if (pCodecCtx->codec_type == AVMEDIA_TYPE_VIDEO ||
				pCodecCtx->codec_type == AVMEDIA_TYPE_AUDIO) {
				used = avcodec_send_packet(pCodecCtx, &packet);
				if (used < 0 && used != AVERROR(EAGAIN) && used != AVERROR_EOF) {
				}
				else {
					if (used >= 0)
						packet.size = 0;
					    used = avcodec_receive_frame(pCodecCtx, pFrame);
					    if (used >= 0)
							frameFinished = 1;
					//             if (used == AVERROR(EAGAIN) || used == AVERROR_EOF)
				  //                 used = 0;
				}
			}
#endif
			if(frameFinished) {
				++frnum;
				if (_frnum) *_frnum = frnum;
				is_read=true;
				if (!skip) {
#if 0
					AVPicture picRGB;
					// Assign appropriate parts of buffer to image planes in pFrameRGB
					avpicture_fill((AVPicture*)&picRGB, (unsigned char*)&out[0], AV_PIX_FMT_RGB24, out_width, out_height);
#else /* 0 avpicture_fill*/
					uint8_t * dst_data[8];
					int dst_linesize[8];
					int ret;
					ret = av_image_fill_arrays(dst_data, dst_linesize, (unsigned char*)&out[0], AV_PIX_FMT_RGB24, out_width, out_height, 1);
					if (ret < 0) {
						// av_buffer_unref(&frame->buf[0]); //http://ffmpeg.org/doxygen/trunk/bitpacked__dec_8c_source.html#l00041> 
						return false;
					}
#endif /* 0 avpicture_fill */
               			// Convert the image from its native format to RGB
       					sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, dst_data, dst_linesize);
					if (settings::SAVE_IMAGES)
						save_frame(&out[0]);
				}
			}
		}
		av_packet_unref(&packet);
		if (is_read) return true;
	}	
	return false;
}

/*bool SaveTGA(char *name, const unsigned char *data, int sizeX, int sizeY) {
	BYTE	TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};
	BYTE	header[6];
	header[1] = (BYTE)(sizeX/256);
	header[0] = (BYTE) sizeX%256;
	header[3] = (BYTE)(sizeY/256);
	header[2] = (BYTE) sizeY%256;
	header[4] = (BYTE) 24;
	header[5] = (BYTE) 0x00;
	int fh = open(name, _O_CREAT|_O_TRUNC|_O_WRONLY);
	if (-1 == fh) return false;
	if (12*sizeof(BYTE) != write(fh, TGAheader, 12*sizeof(BYTE))) return false;
	if (6*sizeof(BYTE) != write(fh, header, 6*sizeof(BYTE))) return false;
	for (int i = 0; i < sizeY; i++) {
		if (3*sizeX != write(fh, data + sizeX*i*3, sizeX*3)) {
			close(fh);
			return false;
		}
	}
	close(fh);
	return true;
}*/

void qav::qvideo::save_frame(const unsigned char *buf, const char* __fname) {
	FILE 		*pFile;
	std::string	s_fname;
	char		num_buf[32];

	//
	sprintf(num_buf, ".%08d.ppm", frnum);
	num_buf[31] = '\0';
	std::ostringstream oss;
	oss << ((__fname) ? __fname : fname.c_str()) << num_buf;
	// Open file
	pFile=fopen(oss.str().c_str(), "wb");
	if(pFile==NULL)
		return;

    	// Write header
    	fprintf(pFile, "P6\n%d %d\n255\n", out_width, out_height);

    	// Write pixel data
    	for(int y=0; y<out_height; y++)
        	fwrite(buf+y*out_width*3, 1, out_width*3, pFile);

    	// Close file
    	fclose(pFile);
}

qav::qvideo::~qvideo() {
    free_resources();
}

void qav::qvideo::free_resources(void) {
    	if (img_convert_ctx) {
		sws_freeContext(img_convert_ctx);
		img_convert_ctx = 0;
	}
	if (pFrame) {
		av_free(pFrame);
		pFrame = 0;
	}
	if (pCodecCtx) {
		avcodec_close(pCodecCtx);
		pCodecCtx = 0;
	}
	if (pFormatCtx) {
		avformat_close_input(&pFormatCtx);
		pFormatCtx = 0;
	}
}

