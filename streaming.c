#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>	//added by zjlu, for enum AVRounding
#include <libavutil/time.h>	//added by zjlu, may has nothing usage
//#define AV_CODEC_FLAG_GLOBAL_HEADER (1 << 22)	//added by zjlu, included in mathematics.h
#include <libavutil/log.h>	//for av_log_set_level(AV_LOG_QUIET);
#include <libavutil/dict.h>	//for AVDictionary struct
//struct AVDictionary {	//no helps
//	int count;
//	AVDictionaryEntry *elems;
//};

//added by zjlu
#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMAX3(a,b,c) FFMAX(FFMAX(a,b),c)
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))
#define FFMIN3(a,b,c) FFMIN(FFMIN(a,b),c)

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag)
{
	AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;
	printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n", tag, av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base), av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base), av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),	pkt->stream_index);
}

extern int streaming(char* input_file, char* output_url, int frames)
{
	AVOutputFormat *ofmt = NULL;
	AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
	AVPacket pkt;
	int ret, i;

	av_register_all();
	avformat_network_init();	//add by zjlu: because using network stream

	av_log_set_level(AV_LOG_QUIET);	//zjlu: do not show warning messages

	AVDictionary *opt = NULL;
	char option_key[20] = "service_provider";
	char option_value[20] = "Lu Zhengjun";
	av_dict_set(&opt, option_key, option_value, 0);
	memset(option_key, '\0', 20);
	memset(option_value, '\0', 20);
	strcpy(option_key, "service_name");
	strcpy(option_value, "Smart Stream Server");
	av_dict_set(&opt, option_key, option_value, 0);

	//if((ret = avformat_open_input(&ifmt_ctx, input_file, 0, &opt)) < 0){	//no helps
	if((ret = avformat_open_input(&ifmt_ctx, input_file, NULL, NULL)) < 0){
		fprintf(stderr, "Could not open input file '%s'", input_file);
		goto end;
	}
	if((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0){
		fprintf(stderr, "Failed to retrieve input stream information");
		goto end;
	}

	int video_index = -1;
	for(i=0; i<ifmt_ctx->nb_streams; i++){ 
		if(ifmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
			video_index = i;
			break;
		}
	}

	av_dump_format(ifmt_ctx, 0, input_file, 0);
	avformat_alloc_output_context2(&ofmt_ctx, NULL, "mpegts", output_url);
	if(!ofmt_ctx){
		fprintf(stderr, "Could not create output context\n");
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt = ofmt_ctx->oformat;
	for(i = 0; i < ifmt_ctx->nb_streams; i++){
		AVStream *in_stream = ifmt_ctx->streams[i];
		AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
		if(!out_stream){
			fprintf(stderr, "Failed allocating output stream\n");
			ret = AVERROR_UNKNOWN;
			goto end;
		}
		ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
		if(ret < 0){
			fprintf(stderr, "Failed to copy context from input to output stream codec context\n");
			goto end;
		}
		out_stream->codec->codec_tag = 0;
		if(ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER){
			out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
		}
	}
	av_dump_format(ofmt_ctx, 0, output_url, 1);
	if(!(ofmt->flags & AVFMT_NOFILE)){
		ret = avio_open(&ofmt_ctx->pb, output_url, AVIO_FLAG_WRITE);
		if(ret < 0){
			fprintf(stderr, "Could not open output file '%s'", output_url);
			goto end;
		}
	}

	av_dict_set(&ofmt_ctx->metadata, "service_name", "Enjoy Streaming Server", 0);
	av_dict_set(&ofmt_ctx->metadata, "service_provider", "Lu Zhengjun", 0);
	//ret = avformat_write_header(ofmt_ctx, &opt);	//no helps
	ret = avformat_write_header(ofmt_ctx, NULL);
	if(ret < 0){
		fprintf(stderr, "Error occurred when opening output file\n");
		goto end;
	}

	int frame_index = 0;
	int64_t start_time = av_gettime();	//int64_t av_gettime(void), Get the current time in microseconds.
	int64_t last_dts = 0;	//added by zjlu, for question "non monotonically increasing dts to muxer in stream"
	while(1){
		AVStream *in_stream, *out_stream;

		ret = av_read_frame(ifmt_ctx, &pkt);
		if(ret < 0){
			if(frame_index < frames){
				fprintf(stderr, "Clip's physical length is less than the length in pgmlist.\n");
				av_usleep((frames-frame_index)*1000000/25);
			}
			break;
		}

		//added by zjlu, ref from Internet, for delay between two frames
		if(pkt.stream_index == video_index){
			AVRational time_base = ifmt_ctx->streams[video_index]->time_base;	////AVRational AVStream::time_base, This is the fundamental unit of time (in seconds) in terms of which frame timestamps are represented.
			//int64_t av_rescale_q(int64_t a, AVRational bq, AVRational cq) const;(a * bq / cq)
			//pkt.dts * time_base(time in seconds), #define AV_TIME_BASE 1000000, so av_rescale_q(pkt.dts, time_base, AV_TIME_BASE_Q)(time in microseconds)
			//av_rescale_q(pkt.dts, time_base, AV_TIME_BASE_Q) is the time in microseconds after the begining, in this example, the first frame's dts is 1160000 microsecons
			int64_t dts_in_microseconds = av_rescale_q(pkt.dts, time_base, AV_TIME_BASE_Q);
			int64_t microseconds_after_the_beginning = av_gettime() - start_time;
			if (dts_in_microseconds > microseconds_after_the_beginning){
				av_usleep(dts_in_microseconds - microseconds_after_the_beginning);	//zjlu: sleeping time = time between two frames
			}
		}

		in_stream  = ifmt_ctx->streams[pkt.stream_index];
		out_stream = ofmt_ctx->streams[pkt.stream_index];
		//in_stream->metadata = opt;	//segment error
		//out_stream->metadata = opt;	//segment error
		//av_dict_set(&out_stream->metadata, "service_name", "zjlu", 0);	//no helps
		//av_dict_set(&out_stream->metadata, "service_provider", "zjlu", 0);	//no helps
		#if 0
		log_packet(ifmt_ctx, &pkt, "in");
		#endif
		//pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);	//modify by zjlu
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));	//int64_t pts,	Presentation timestamp in AVStream->time_base units; the time at which the decompressed packet will be presented to the user. 
		//pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);	//modify by zjlu
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));	//int64_t dts,	Decompression timestamp in AVStream->time_base units; the time at which the packet is decompressed.
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);	//int64_t duration, Duration of this packet in AVStream->time_base units, 0 if unknown.
		pkt.pos = -1;	//int64_t AVPacket::pos, byte position in stream, -1 if unknown
		
		if(pkt.stream_index == video_index){
			//printf("Send %8d video frames to output URL\n",frame_index);
			if(frame_index >= frames){	//zjlu: streaming according to the schedule duration
				break;
			}
			frame_index++;
		}
		//log_packet(ofmt_ctx, &pkt, "out");

		//added by zjlu, ref from ffmpeg.c write_frame(), in order to solve the problem "non monotonically increasing dts to muxer in stream"
		if(!(ofmt->flags & AVFMT_NOTIMESTAMPS)) {
			if (pkt.dts != AV_NOPTS_VALUE && pkt.pts != AV_NOPTS_VALUE && pkt.dts > pkt.pts) {
				//av_log(s, AV_LOG_WARNING, "Invalid DTS: %"PRId64" PTS: %"PRId64" in output stream %d:%d, replacing by guess\n", pkt->dts, pkt->pts, ost->file_index, ost->st->index);
				pkt.pts = pkt.dts = pkt.pts + pkt.dts + last_dts + 1 - FFMIN3(pkt.pts, pkt.dts, last_dts + 1) - FFMAX3(pkt.pts, pkt.dts, last_dts + 1);
			}
			if((out_stream->codec->codec_type == AVMEDIA_TYPE_AUDIO || out_stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) && pkt.dts != AV_NOPTS_VALUE && !(out_stream->codec->codec_id == AV_CODEC_ID_VP9) && last_dts != AV_NOPTS_VALUE){
      		int64_t max = last_dts + !(ofmt->flags & AVFMT_TS_NONSTRICT);
				if(pkt.dts < max) {
					int loglevel = max - pkt.dts > 2 || out_stream->codec->codec_type == AVMEDIA_TYPE_VIDEO ? AV_LOG_WARNING : AV_LOG_DEBUG;
					//av_log(s, loglevel, "Non-monotonous DTS in output stream ""%d:%d; previous: %"PRId64", current: %"PRId64"; ", ost->file_index, ost->st->index, ost->last_mux_dts, pkt->dts);
					//if (exit_on_error) {	//zjlu: maybe no use, because only "int exit_on_error = 0" in ffmpeg_opt.c
						//av_log(NULL, AV_LOG_FATAL, "aborting.\n");
						//exit_program(1);
					//}
					//av_log(s, loglevel, "changing to %"PRId64". This may result ""in incorrect timestamps in the output file.\n", max);
					if(pkt.pts >= pkt.dts){
						pkt.pts = FFMAX(pkt.pts, max);
					}
					pkt.dts = max;
				}
			}
		}
		last_dts = pkt.dts;	

		ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
		if(ret < 0){
			fprintf(stderr, "error while muxing packet\n");
			break;
		}
		av_packet_unref(&pkt);
		//av_free_packet(&pkt);	//zjlu: someone write this from Internet, but get a warning
	}

	av_write_trailer(ofmt_ctx);

end:
	avformat_close_input(&ifmt_ctx);
	if(ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE)){
		avio_closep(&ofmt_ctx->pb);
	}
	avformat_free_context(ofmt_ctx);
	if(ret < 0 && ret != AVERROR_EOF){
		fprintf(stderr, "error occurred: %s\n", av_err2str(ret));
		return 1;
	}
	return 0;
}
