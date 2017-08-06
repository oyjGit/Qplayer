
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4390 )
#endif



#include <string.h>
#include <stdio.h>

#define NOCACHE 1
#include "h264Parser.h"
#include "BitStream.h"

#ifdef __cplusplus
extern "C"{
#endif


int h264_parse_sps(const uint8_t *buf, int len, h264_sps_data_t *sps)
{
	memset(sps, 0, sizeof(h264_sps_data_t));

	br_state br = BR_INIT(buf, len);
	int profile_idc, pic_order_cnt_type;
	int frame_mbs_only;
	int i, j;

	profile_idc = br_get_u8(&br);
	sps->profile = profile_idc;
	//printf("H.264 SPS: profile_idc %d", profile_idc);
	/* constraint_set0_flag = br_get_bit(br);    */
	/* constraint_set1_flag = br_get_bit(br);    */
	/* constraint_set2_flag = br_get_bit(br);    */
	/* constraint_set3_flag = br_get_bit(br);    */
	/* reserved             = br_get_bits(br,4); */
	br_skip_bits(&br, 8);
	sps->level = br_get_u8(&br);
	br_skip_ue_golomb(&br);   /* seq_parameter_set_id */
	if (profile_idc >= 100)
	{
		if (br_get_ue_golomb(&br) == 3) /* chroma_format_idc      */
		{
			br_skip_bit(&br);     /* residual_colour_transform_flag */
		}
		br_skip_ue_golomb(&br); /* bit_depth_luma - 8             */
		br_skip_ue_golomb(&br); /* bit_depth_chroma - 8           */
		br_skip_bit(&br);       /* transform_bypass               */
		if (br_get_bit(&br))    /* seq_scaling_matrix_present     */
		{
			for (i = 0; i < 8; i++)
				if (br_get_bit(&br))
				{ /* seq_scaling_list_present    */
				int last = 8, next = 8, size = (i < 6) ? 16 : 64;
				for (j = 0; j < size; j++)
				{
					if (next)
					{
						next = (last + br_get_se_golomb(&br)) & 0xff;
					}
					last = next ? next : last;
				}
				}
		}
	}

	br_skip_ue_golomb(&br);      /* log2_max_frame_num - 4 */
	pic_order_cnt_type = br_get_ue_golomb(&br);
	if (pic_order_cnt_type == 0)
	{
		br_skip_ue_golomb(&br);    /* log2_max_poc_lsb - 4 */
	}
	else if (pic_order_cnt_type == 1)
	{
		br_skip_bit(&br);          /* delta_pic_order_always_zero     */
		br_skip_se_golomb(&br);    /* offset_for_non_ref_pic          */
		br_skip_se_golomb(&br);    /* offset_for_top_to_bottom_field  */
		j = br_get_ue_golomb(&br); /* num_ref_frames_in_pic_order_cnt_cycle */
		for (i = 0; i < j; i++)
			br_skip_se_golomb(&br);  /* offset_for_ref_frame[i]         */
	}
	br_skip_ue_golomb(&br);      /* ref_frames                      */
	br_skip_bit(&br);            /* gaps_in_frame_num_allowed       */
	sps->width  /* mbs */ = br_get_ue_golomb(&br) + 1;
	sps->height /* mbs */ = br_get_ue_golomb(&br) + 1;
	frame_mbs_only = br_get_bit(&br);
	//printf("H.264 SPS: pic_width:  %u mbs", (unsigned) sps->width);
	// printf("H.264 SPS: pic_height: %u mbs", (unsigned) sps->height);
	// printf("H.264 SPS: frame only flag: %d", frame_mbs_only);

	sps->width *= 16;
	sps->height *= 16 * (2 - frame_mbs_only);

	if (!frame_mbs_only)
		if (br_get_bit(&br));

	br_skip_bit(&br);      /* direct_8x8_inference_flag    */
	if (br_get_bit(&br))
	{ /* frame_cropping_flag */
		uint32_t crop_left = br_get_ue_golomb(&br);
		uint32_t crop_right = br_get_ue_golomb(&br);
		uint32_t crop_top = br_get_ue_golomb(&br);
		uint32_t crop_bottom = br_get_ue_golomb(&br);
		//  printf("H.264 SPS: cropping %d %d %d %d",
		//  crop_left, crop_top, crop_right, crop_bottom);

		sps->width -= 2 * (crop_left + crop_right);
		if (frame_mbs_only)
			sps->height -= 2 * (crop_top + crop_bottom);
		else
			sps->height -= 4 * (crop_top + crop_bottom);
	}

	/* VUI parameters */
	sps->pixel_aspect.num = 0;
	if (br_get_bit(&br))
	{   /* vui_parameters_present flag */
		if (br_get_bit(&br))
		{ /* aspect_ratio_info_present */
			uint32_t aspect_ratio_idc = br_get_u8(&br);
			//printf("H.264 SPS: aspect_ratio_idc %d", aspect_ratio_idc);

			if (aspect_ratio_idc == 255 /* Extended_SAR */)
			{
				sps->pixel_aspect.num = br_get_u16(&br); /* sar_width */
				sps->pixel_aspect.den = br_get_u16(&br); /* sar_height */
				//printf("H.264 SPS: -> sar %dx%d", sps->pixel_aspect.num, sps->pixel_aspect.den);
			}
			else
			{
				static const mpeg_rational_t aspect_ratios[] =
				{ /* page 213: */
					/* 0: unknown */
					{ 0, 1 },
					/* 1...16: */
					{ 1, 1 }, { 12, 11 }, { 10, 11 }, { 16, 11 }, { 40, 33 }, { 24, 11 }, { 20, 11 }, { 32, 11 },
					{ 80, 33 }, { 18, 11 }, { 15, 11 }, { 64, 33 }, { 160, 99 }, { 4, 3 }, { 3, 2 }, { 2, 1 }
				};

				if (aspect_ratio_idc < sizeof(aspect_ratios) / sizeof(aspect_ratios[0]))
				{
					memcpy(&sps->pixel_aspect, &aspect_ratios[aspect_ratio_idc], sizeof(mpeg_rational_t));
					// printf("H.264 SPS: -> aspect ratio %d / %d", sps->pixel_aspect.num, sps->pixel_aspect.den);
				}
				else
				{
					//printf("H.264 SPS: aspect_ratio_idc out of range !");
				}
			}
		}
	}

	unsigned overscan_info_present_flag = br_get_bit(&br);
	if (overscan_info_present_flag)
	{
		br_skip_bit(&br);
	}
	unsigned video_signal_type_present_flag = br_get_bit(&br);

	if (video_signal_type_present_flag)
	{
		br_skip_bits(&br, 4); // video_format; video_full_range_flag
		unsigned colour_description_present_flag = br_get_bit(&br);
		if (colour_description_present_flag)
		{
			br_skip_bits(&br, 24); // colour_primaries; transfer_characteristics; matrix_coefficients
		}
	}
	unsigned chroma_loc_info_present_flag = br_get_bit(&br);

	if (chroma_loc_info_present_flag)
	{
		br_skip_golomb(&br); // chroma_sample_loc_type_top_field
		br_skip_golomb(&br); // chroma_sample_loc_type_bottom_field
	}
	unsigned timing_info_present_flag = br_get_bit(&br);
	if (timing_info_present_flag)
	{
		unsigned num_units_in_tick = br_get_ue_golomb(&br);

		unsigned time_scale = br_get_ue_golomb(&br);

		unsigned fixed_frame_rate_flag = br_get_bit(&br);


		if (num_units_in_tick > 0 && time_scale > 0)
		{
			sps->fps = time_scale / (2 * num_units_in_tick);
		}
	}
	return 1;
}

int findStartCode(char* buffer, int bufferLen, int *startCodeLen, int* frameType)
{
	int i = 0;
	if (!buffer && bufferLen < 3)
	{
		return -1;
	}
	for (i = 0; i + 1 < bufferLen; i += 2)
	{
		if (buffer[i])
			continue;
		if (i > 0 && buffer[i - 1] == 0)
			i--;
		if (i + 2 < bufferLen && buffer[i] == 0 && buffer[i + 1] == 0 && buffer[i + 2] == 1)
		{
			if (startCodeLen)
			{
				*startCodeLen = 3;
			}
			if (frameType && i + 3 <= bufferLen)
			{
				*frameType = buffer[i + *startCodeLen] & 0x1f;
			}
			return i;
		}
		if (i + 3 < bufferLen && buffer[i] == 0 && buffer[i + 1] == 0 && buffer[i + 2] == 0 && buffer[i + 3] == 1)
		{
			if (startCodeLen)
			{
				*startCodeLen = 4;
			}
			if (frameType && i + 4 <= bufferLen)
			{
				*frameType = buffer[i + *startCodeLen] & 0x1f;
			}
			return i;
		}
	}
	return -1;
}

int getH264FrameType(char* frame,int* startCodeLen)
{
	char type = 0;
	char* pTmp = frame;
	if (!frame)
		return -1;
	if ((*pTmp == 0) && (*(pTmp + 1) == 0) && (*(pTmp + 2) == 0) && (*(pTmp + 3) == 1))//
	{
		type = (*(pTmp + 4)) & 0x1f;
		if (startCodeLen)
			*startCodeLen = 4;

	}
	else if ((*pTmp == 0) && (*(pTmp + 1) == 0) && (*(pTmp + 2) == 1))//
	{
		//printf("===== data : %x\n",*(pTmp+3));
		type = (*(pTmp + 3)) & 0x1f;
		if (startCodeLen)
			*startCodeLen = 3;
	}
	if (type == 7){
		return 7;
	}
	else if (type == 8){
		return 8;
	}
	else if (type == 6){
		return 6;
	}
	else if (type == 5){
		return 5;
	}
	else if (type == 1){
		return 1;
	}
	else
		return -1;
}

char* findFrameOffset(char* p_data,int len, int *aType,int* startCodeLen)
{
	int i;
	*aType = 0;
	if (!p_data || len < 3)
		return NULL;
	for (i = 0; i < len; i++)
	{
		int type;
		char* pTmp = p_data + i;
		if (i + 5 >= len)
			return NULL;
		type = getH264FrameType(pTmp, startCodeLen);
		if (type != -1)
		{
			*aType = type;
			return pTmp;
		}
	}

	return NULL;
}

char* findSepFrameOffset(char* data, int len, int type, int* startCodeLen)
{
	int i=0;
	if(!data || len < 3)
		return NULL;
	for (; i < len; i++)
	{
		char* pTmp = data + i;
		if (i + 5 >= len)
			return NULL;
		if (getH264FrameType(pTmp, startCodeLen) == type)
		{
			//printf("sps\n");
			return pTmp;
			//return pTmp+4;
		}
	}
	return NULL;
}

char* getSepFrame(const char* srcData, int srcDataLen, int frameType, int* startCodeLen, int* frameLen)
{
	int ret = 0;
	int scl = 0;
	int nextLen = 0;
	char* result = NULL;
	if (srcData && srcDataLen > 3 && startCodeLen && frameLen)
	{
		result = findSepFrameOffset(srcData, srcDataLen, frameType, startCodeLen);
		if (result != NULL)
		{
			nextLen = srcDataLen - (result - srcData) - *startCodeLen;
			ret = findStartCode(result + *startCodeLen, nextLen, NULL, NULL);
			if (ret != -1)
			{
				*frameLen = ret;//(result - srcData) - *startCodeLen;
			}
			else
			{
				*frameLen = srcDataLen - *startCodeLen;
			}
		}
	}
	return result;
}

#ifdef __cplusplus
}
#endif