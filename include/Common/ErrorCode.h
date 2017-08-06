#ifndef __QPLAYER_ERRORCODE_H__
#define __QPLAYER_ERRORCODE_H__

#define QPLAYER_SUCCESS 0
#define QPLAYER_FAILED -1

typedef enum LY_ERROR_CODE
{
	EOK = QPLAYER_SUCCESS,
	EINVALID_PARAM,										//非法参数
	EERROR_PARAM,										//参数错误
	EERROR_PARAM_CID,									//解析url得到cid为0
	EERROR_PARAM_URL_LENGTH_INVALID,					//url长度错误，最大接收长度为256字节
	EERROR_CREATE_QUE_FAILED,							//创建队列失败，内存不够？
	EERROR_CREATE_INPUT_FAILED,							//创建数据输入源失败。检查类型是否支持
	EERROR_CONNECT_FAILED,								//建立连接失败
	EERROR_WORKING,										//该操作正在进行

	EPLAYER_NO_MEDIASOURCE = 100,						//没有设置mediaSource
	EPLAYER_CREATE_FAILD,								//创建播放器失败，检查url格式是否正确
	EPLAYER_START_VIDEO_FAILED,							//初始化视频解码播放相关失败
	EPLAYER_START_AUDIO_FAILED,							//初始化音频解码播放相关失败
	EPLAYER_WORKING,									//播放器正在工作，需要先停止播放器
	EPLAYER_NOT_WORKING,								//播放器没有工作，无法进行该操作
	EPLAYER_STOPING,									//播放器正在停止
	EPLAYER_RECORDING,									//正在录像中
	EPLAYER_NOT_RECORDING,								//没有开启录像
	EPLAYER_NOT_RECORD_FILE,							//不是播放云录像或者本地文件
	EPLAYER_NOT_PAUSE,									//不需要恢复播放，没有暂停
	EPLAYER_FIND_VIDEO_CODEC_FAILED,					//寻找视频解码器失败
	EPLAYER_FIND_AUDIO_CODEC_FAILED,					//寻找音频解码器失败
	EPLAYER_OPEN_VIDEO_CODEC_FAILED,					//打开视频解码器失败
	EPLAYER_OPEN_AUDIO_CODEC_FAILED,					//打开音频解码器失败

	ECREATE_DIR_FAILED,									//创建目录失败
	EREOCRD_CREATE_OBJ_FAILED = 0x00001000,				//创建录像对象失败
	EREOCRD_CREATE_FILE_FAILED,							//创建录像文件失败
	EREOCRD_SET_TIMEVAL_FAILED,							//设置单次录像时间间隔大于4个小时,默认设置为4个小时
	EREOCRD_FILE_NAME_TOO_LONG,							//文件名太长，最大259(包含目录)个字节
	ERECORD_DOWN_FINISHED,								//录像下载完成

	ELOCAL_OPEN_FILE_FAILED = 0x00002000,				//打开文件失败
	ELOCAL_GET_MEIDA_FAILED,							//没有在文件中发现有音视频数据
	ELOCAL_READ_FAILED,									//读取文件失败或者完成


	EOUTPUT_DONE = 0x00003000,

	EINPUT_TIMEOUT = 0x0000f000,

	EGET_MEDIA_FAILED = 0x00010000,
	EGET_MEDIA_DONE,
	ENOT_WORKING,
	EWORKING_DONE,
}EErrorCode;

typedef enum LY_EVENT_CODE
{
	//优先级从高到低,0最高
	//以下是高优先级事件ID，一些中断连接或者读取数据错误的事件
	EEID_RECVDATA_FAILED = 0,				//接收数据失败
	EEID_GET_RECORD_DATA_DONE,				//录像播放或者下载完成
	EEID_LOCAL_READ_FAILED,					//读取本地文件失败
	EEID_LOCAL_READ_DONE,					//读取本地文件完成
	EEID_DISCONNECT_SUCCESS,				//断开连接完成
	EEID_CONNECT_SERVER_FAILED,				//连接服务器失败
	EEID_DISCONNECTED,						//连接断开
	EEID_SAVE_PIC_FAILED,					//保存截图失败
	EEID_SAVE_PIC_SUCCESS,					//保存截图成功			
	EEID_COMMIT_CLIP_FAILD,					//新建剪辑任务失败
	EEID_COMMIT_CLIP_SUCCESS,				//新建剪辑任务成功
	EEID_CLIP_SUCCESS,						//剪辑成功
	EEID_GET_CLIP_ID_FAILED,				//获取剪辑ID失败，json解析失败
	EEID_GET_CLIP_STATUS_FAILED,			//获取剪辑状态出现错误

	//以下是中等优先级事件ID//
	EEID_RECORD_WRITE_V_FAILED,				//录像写视频到文件失败
	EEID_RECORD_WRITE_A_FAILED,				//录像写音频到文件失败
	EEID_RECORD_STREAM_PARSE_FAILED,		//解析录像失败
	EEID_GET_RECORD_LIST_FAILED,			//获取录像列表失败
	EEID_CLIP_FAILED,						//剪辑失败
	EEID_CREATE_RECORD_FAILED,				//打开录像文件失败
	EEID_START_GET_RECORD_LIST,				//开始获取录像列表
	EEID_GET_RECORD_LIST_SUCCESS,			//获取录像列表成功
	EEID_START_CONNECT_SERVER,				//开始连接服务器
	EEID_CONNECT_SERVER_SUCCESS,			//连接服务器成功
	EEID_START_DISCONNECT,					//开始断开连接
	EEID_NOT_SUPPORT_FRAME_TYPE,			//不支持的帧类型
	EEID_REOCRD_RENAME_FAILED,				//重命名录像文件失败
	EEID_LOCAL_READ_SUCCESS,				//读取本地文件成功

	//以下是最低优先级
	EEID_VIDEOBUF_FULL,						//视频缓存队列满了，接收比较快播放太慢了或者缓冲区太小了
	EEID_AUDIOBUF_FULL,						//音频缓存队列满了，接收比较快播放太慢了或者缓冲区太小了
	EEID_REOCRD_DOWN_PROCESS,				//下载录像进度更新
	EEID_DOWNRATE_UPDATE,					//更新下载速率，帧率等信息


	EEID_SHOW_SOMETHING,					//记录eid的数量，需要前面连续定义

}EEventCode;


#endif