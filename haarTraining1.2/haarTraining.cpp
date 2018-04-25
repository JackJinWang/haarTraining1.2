#include "haarTraining.h"
#include <vector>
#include <fstream>
#include"tinyxml2.h"
using namespace tinyxml2;
#include<string>
#include <ctype.h>
#include<time.h>
#include <math.h>
#include"myIntergal.h"
#include"delete.h"
#include"classifier.h"
using namespace std;
#define POS_FLAG 1
#define NEG_FLAG 2
/*
* get sum image offsets for <rect> corner points
* step - row step (measured in image pixels!) of sum image
*/
#define CV_SUM_OFFSETS( p0, p1, p2, p3, rect, step )                      \
    /* (x, y) */                                                          \
    (p0) = (rect).x + (step) * (rect).y;                                  \
    /* (x + w, y) */                                                      \
    (p1) = (rect).x + (rect).width + (step) * (rect).y;                   \
    /* (x , y+h) */                                                      \
    (p2) = (rect).x + (step) * ((rect).y + (rect).height);                \
    /* (x + w, y + h) */                                                  \
    (p3) = (rect).x + (rect).width + (step) * ((rect).y + (rect).height);

/*
* get tilted image offsets for <rect> corner points
* step - row step (measured in image pixels!) of tilted image
*/
#define CV_TILTED_OFFSETS( p0, p1, p2, p3, rect, step )                   \
    /* (x, y) */                                                          \
    (p0) = (rect).x + (step) * (rect).y;                                  \
    /* (x - h, y + h) */                                                  \
    (p1) = (rect).x - (rect).height + (step) * ((rect).y + (rect).height);\
    /* (x + w, y + w) */                                                  \
    (p2) = (rect).x + (rect).width + (step) * ((rect).y + (rect).width);  \
    /* (x + w - h, y + w + h) */                                          \
    (p3) = (rect).x + (rect).width - (rect).height                        \
           + (step) * ((rect).y + (rect).width + (rect).height);

/*
* icvCreateIntHaarFeatures
*
* Create internal representation of haar features
*
* mode:
*  0 - BASIC = Viola
*  1 - CORE  = All upright
*  2 - ALL   = All features
*/
static
CvIntHaarFeatures* icvCreateIntHaarFeatures(MySize winsize,
	int mode,
	int symmetric)
{
	CvIntHaarFeatures* features = NULL;
	CvTHaarFeature haarFeature;

	//CvMemStorage* storage = NULL;
	//CvSeq* seq = NULL;
	//CvSeqWriter writer;

	vector<CvTHaarFeature> seq;

	int s0 = 36; /* minimum total area size of basic haar feature     */
	int s1 = 12; /* minimum total area size of tilted haar features 2 */
	int s2 = 18; /* minimum total area size of tilted haar features 3 */
	int s3 = 24; /* minimum total area size of tilted haar features 4 */

	int x = 0;
	int y = 0;
	int dx = 0;
	int dy = 0;

#if 0
	float factor = 1.0F;

	factor = ((float)winsize.width) * winsize.height / (24 * 24);

	s0 = (int)(s0 * factor);
	s1 = (int)(s1 * factor);
	s2 = (int)(s2 * factor);
	s3 = (int)(s3 * factor);
#else
	s0 = 1;
	s1 = 1;
	s2 = 1;
	s3 = 1;
#endif

	/* CV_VECTOR_CREATE( vec, CvIntHaarFeature, size, maxsize ) */
	//	storage = cvCreateMemStorage();
	//	cvStartWriteSeq(0, sizeof(CvSeq), sizeof(haarFeature), storage, &writer);

	for (x = 0; x < winsize.width; x++)
	{
		for (y = 0; y < winsize.height; y++)
		{
			for (dx = 1; dx <= winsize.width; dx++)
			{
				for (dy = 1; dy <= winsize.height; dy++)
				{
					// haar_x2 //�ң���
					if ((x + dx * 2 <= winsize.width) && (y + dy <= winsize.height)) {
						if (dx * 2 * dy < s0) continue;
						if (!symmetric || (x + x + dx * 2 <= winsize.width)) {
							haarFeature = cvHaarFeature("haar_x2",
								x, y, dx * 2, dy, -1,
								x + dx, y, dx, dy, +2);
							/* CV_VECTOR_PUSH( vec, CvIntHaarFeature, haarFeature, size, maxsize, step ) */
							//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
							seq.push_back(haarFeature);
						}
					}

					// haar_y2 �¼���
					if ((x + dx <= winsize.width) && (y + dy * 2 <= winsize.height)) {
						if (dx * 2 * dy < s0) continue;
						if (!symmetric || (x + x + dx <= winsize.width)) {
							haarFeature = cvHaarFeature("haar_y2",
								x, y, dx, dy * 2, -1,
								x, y + dy, dx, dy, +2);
							//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
							seq.push_back(haarFeature);
						}
					}

					// haar_x3  �м�-���� ��
					if ((x + dx * 3 <= winsize.width) && (y + dy <= winsize.height)) {
						if (dx * 3 * dy < s0) continue;
						if (!symmetric || (x + x + dx * 3 <= winsize.width)) {
							haarFeature = cvHaarFeature("haar_x3",
								x, y, dx * 3, dy, -1,
								x + dx, y, dx, dy, +3);
							//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
							seq.push_back(haarFeature);
						}
					}

					// haar_y3 �м�-���� ��
					if ((x + dx <= winsize.width) && (y + dy * 3 <= winsize.height)) {
						if (dx * 3 * dy < s0) continue;
						if (!symmetric || (x + x + dx <= winsize.width)) {
							haarFeature = cvHaarFeature("haar_y3",
								x, y, dx, dy * 3, -1,
								x, y + dy, dx, dy, +3);
							//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
							seq.push_back(haarFeature);
						}
					}

					if (mode != 0 /*BASIC*/) {
						// haar_x4
						if ((x + dx * 4 <= winsize.width) && (y + dy <= winsize.height)) {
							if (dx * 4 * dy < s0) continue;
							if (!symmetric || (x + x + dx * 4 <= winsize.width)) {
								haarFeature = cvHaarFeature("haar_x4",
									x, y, dx * 4, dy, -1,
									x + dx, y, dx * 2, dy, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}

						// haar_y4
						if ((x + dx <= winsize.width) && (y + dy * 4 <= winsize.height)) {
							if (dx * 4 * dy < s0) continue;
							if (!symmetric || (x + x + dx <= winsize.width)) {
								haarFeature = cvHaarFeature("haar_y4",
									x, y, dx, dy * 4, -1,
									x, y + dy, dx, dy * 2, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}
					}

					// x2_y2
					if ((x + dx * 2 <= winsize.width) && (y + dy * 2 <= winsize.height)) {
						if (dx * 4 * dy < s0) continue;
						if (!symmetric || (x + x + dx * 2 <= winsize.width)) {
							haarFeature = cvHaarFeature("haar_x2_y2",
								x, y, dx * 2, dy * 2, -1,
								x, y, dx, dy, +2,
								x + dx, y + dy, dx, dy, +2);
							//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
							seq.push_back(haarFeature);
						}
					}

					if (mode != 0 /*BASIC*/) {
						// point
						if ((x + dx * 3 <= winsize.width) && (y + dy * 3 <= winsize.height)) {
							if (dx * 9 * dy < s0) continue;
							if (!symmetric || (x + x + dx * 3 <= winsize.width)) {
								haarFeature = cvHaarFeature("haar_point",
									x, y, dx * 3, dy * 3, -1,
									x + dx, y + dy, dx, dy, +9);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}
					}

					if (mode == 2 /*ALL*/) {
						// tilted haar_x2                                      (x, y, w, h, b, weight)
						if ((x + 2 * dx <= winsize.width) && (y + 2 * dx + dy <= winsize.height) && (x - dy >= 0)) {
							if (dx * 2 * dy < s1) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_x2",
									x, y, dx * 2, dy, -1,
									x, y, dx, dy, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}

						// tilted haar_y2                                      (x, y, w, h, b, weight)
						if ((x + dx <= winsize.width) && (y + dx + 2 * dy <= winsize.height) && (x - 2 * dy >= 0)) {
							if (dx * 2 * dy < s1) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_y2",
									x, y, dx, 2 * dy, -1,
									x, y, dx, dy, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}

						// tilted haar_x3                                   (x, y, w, h, b, weight)
						if ((x + 3 * dx <= winsize.width) && (y + 3 * dx + dy <= winsize.height) && (x - dy >= 0)) {
							if (dx * 3 * dy < s2) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_x3",
									x, y, dx * 3, dy, -1,
									x + dx, y + dx, dx, dy, +3);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}

						// tilted haar_y3                                      (x, y, w, h, b, weight)
						if ((x + dx <= winsize.width) && (y + dx + 3 * dy <= winsize.height) && (x - 3 * dy >= 0)) {
							if (dx * 3 * dy < s2) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_y3",
									x, y, dx, 3 * dy, -1,
									x - dy, y + dy, dx, dy, +3);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}


						// tilted haar_x4                                   (x, y, w, h, b, weight)
						if ((x + 4 * dx <= winsize.width) && (y + 4 * dx + dy <= winsize.height) && (x - dy >= 0)) {
							if (dx * 4 * dy < s3) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_x4",


									x, y, dx * 4, dy, -1,
									x + dx, y + dx, dx * 2, dy, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}

						// tilted haar_y4                                      (x, y, w, h, b, weight)
						if ((x + dx <= winsize.width) && (y + dx + 4 * dy <= winsize.height) && (x - 4 * dy >= 0)) {
							if (dx * 4 * dy < s3) continue;

							if (!symmetric || (x <= (winsize.width / 2))) {
								haarFeature = cvHaarFeature("tilted_haar_y4",
									x, y, dx, 4 * dy, -1,
									x - dy, y + dy, dx, 2 * dy, +2);
								//	CV_WRITE_SEQ_ELEM(haarFeature, writer);
								seq.push_back(haarFeature);
							}
						}


						/*

						// tilted point
						if ( (x+dx*3 <= winsize.width - 1) && (y+dy*3 <= winsize.height - 1) && (x-3*dy>= 0)) {
						if (dx*9*dy < 36) continue;
						if (!symmetric || (x <= (winsize.width / 2) ))  {
						haarFeature = cvHaarFeature( "tilted_haar_point",
						x, y,    dx*3, dy*3, -1,
						x, y+dy, dx  , dy,   +9 );
						CV_WRITE_SEQ_ELEM( haarFeature, writer );
						}
						}
						*/
					}
				}
			}
		}
	}

	//seq = cvEndWriteSeq(&writer);
	features = (CvIntHaarFeatures*)malloc(sizeof(CvIntHaarFeatures) +
		(sizeof(CvTHaarFeature) + sizeof(CvFastHaarFeature)) * seq.size());
	features->feature = (CvTHaarFeature*)(features + 1);
	features->fastfeature = (CvFastHaarFeature*)(features->feature + seq.size());
	features->count = seq.size();
	features->winsize = winsize;
	//	cvCvtSeqToArray(seq, (CvArr*)features->feature);
	for (int i = 0;i < seq.size();i++)
	{
		features->feature[i] = seq[i];
	}
	//	cvReleaseMemStorage(&storage);

	icvConvertToFastHaarFeature(features->feature, features->fastfeature,
		features->count, (winsize.width + 1));

	return features;
}
/*
*������������
*/
void icvConvertToFastHaarFeature(CvTHaarFeature* haarFeature,
	CvFastHaarFeature* fastHaarFeature,
	int size, int step)
{
	int i = 0;
	int j = 0;

	for (i = 0; i < size; i++)
	{
		fastHaarFeature[i].tilted = haarFeature[i].tilted;
		if (!fastHaarFeature[i].tilted)
		{
			for (j = 0; j < CV_HAAR_FEATURE_MAX; j++)
			{
				fastHaarFeature[i].rect[j].weight = haarFeature[i].rect[j].weight;
				if (fastHaarFeature[i].rect[j].weight == 0.0F)
				{
					break;
				}
				CV_SUM_OFFSETS(fastHaarFeature[i].rect[j].p0,
					fastHaarFeature[i].rect[j].p1,
					fastHaarFeature[i].rect[j].p2,
					fastHaarFeature[i].rect[j].p3,
					haarFeature[i].rect[j].r, step)
			}

		}
		else
		{
			for (j = 0; j < CV_HAAR_FEATURE_MAX; j++)
			{
				fastHaarFeature[i].rect[j].weight = haarFeature[i].rect[j].weight;
				if (fastHaarFeature[i].rect[j].weight == 0.0F)
				{
					break;
				}
				CV_TILTED_OFFSETS(fastHaarFeature[i].rect[j].p0,
					fastHaarFeature[i].rect[j].p1,
					fastHaarFeature[i].rect[j].p2,
					fastHaarFeature[i].rect[j].p3,
					haarFeature[i].rect[j].r, step);
			}
		}
	}
}
/*
* icvCreateHaarTrainingData
*
* Create haar training data used in stage training
*/
static
CvHaarTrainigData* icvCreateHaarTrainingData(MySize winsize, int maxnumsamples)
{
	CvHaarTrainigData* data;
	data = NULL;
	uchar* ptr = NULL;
	size_t datasize = 0;

	datasize = sizeof(CvHaarTrainigData) +
		/* sum and tilted */
		(2 * (winsize.width + 1) * (winsize.height + 1) * sizeof(sum_type) +
			sizeof(float) +      /* normfactor */
			sizeof(float) +      /* cls */
			sizeof(float)        /* weight */
			) * maxnumsamples;
	data = (CvHaarTrainigData*)malloc(datasize);
	memset((void*)data, 0, datasize);

	data->maxnum = maxnumsamples;
	data->winsize = winsize;
	ptr = (uchar*)(data + 1);

	data->sum = myMat(maxnumsamples, (winsize.width + 1) * (winsize.height + 1), ONE_CHANNEL, INT_TYPE, (void*)ptr);
	ptr += sizeof(sum_type) * maxnumsamples * (winsize.width + 1) * (winsize.height + 1);

	data->tilted = myMat(maxnumsamples, (winsize.width + 1) * (winsize.height + 1), ONE_CHANNEL, INT_TYPE, (void*)ptr);
	ptr += sizeof(sum_type) * maxnumsamples * (winsize.width + 1) * (winsize.height + 1);

	data->normfactor = myMat(1, maxnumsamples, ONE_CHANNEL, FLOAT_TYPE, (void*)ptr);
	ptr += sizeof(float) * maxnumsamples;

	data->cls = myMat(1, maxnumsamples, ONE_CHANNEL, FLOAT_TYPE, (void*)ptr);
	ptr += sizeof(float) * maxnumsamples;

	data->weights = myMat(1, maxnumsamples, ONE_CHANNEL, FLOAT_TYPE, (void*)ptr);

	data->valcache = NULL;
	data->idxcache = NULL;

	return data;
}
typedef struct CvBackgroundData
{
	int    count;
	char** filename;
	int    last;
	int    round;
	MySize winsize;
} CvBackgroundData;
/*��ͼƬ*/
CvBackgroundData* cvbgdata = NULL;          //��סҪ�ͷ�
/*��ͼƬ*/
CvBackgroundData* cvposdata = NULL;      
/*������ȡ����*/
int trainingdata_number = 0;

static
CvBackgroundData* icvCreateBackgroundData(const char* filename,MySize winsize)
{
	CvBackgroundData* data = NULL;

	const char* dir = NULL;
	char full[PATH_MAX];
	char* imgfilename = NULL;
	size_t datasize = 0;
	int    count = 0;
	FILE*  input = NULL;
	char*  tmp = NULL;
	int    len = 0;

	assert(filename != NULL);

	dir = strrchr(filename, '\\');
	if (dir == NULL)
	{
		dir = strrchr(filename, '/');
	}
	if (dir == NULL)
	{
		imgfilename = &(full[0]);
	}
	else
	{
		strncpy(&(full[0]), filename, (dir - filename + 1));
		imgfilename = &(full[(dir - filename + 1)]);
	}

	input = fopen(filename, "r");
	if (input != NULL)
	{
		count = 0;
		datasize = 0;

		/* count */
		while (!feof(input))
		{
			*imgfilename = '\0';
			if (!fgets(imgfilename, PATH_MAX - (int)(imgfilename - full) - 1, input))
				break;
			len = (int)strlen(imgfilename);
			for (; len > 0 && isspace(imgfilename[len - 1]); len--)
				imgfilename[len - 1] = '\0';
			if (len > 0)
			{
				if ((*imgfilename) == '#') continue; /* comment */
				count++;
				datasize += sizeof(char) * (strlen(&(full[0])) + 1);
			}
		}
		if (count > 0)
		{
			//rewind( input );
			fseek(input, 0, SEEK_SET);
			datasize += sizeof(*data) + sizeof(char*) * count;
			data = (CvBackgroundData*)malloc(datasize);
			memset((void*)data, 0, datasize);
			data->count = count;
			data->filename = (char**)(data + 1);
			data->last = 0;
			data->round = 0;
			data->winsize = winsize;
			tmp = (char*)(data->filename + data->count);
			count = 0;
			while (!feof(input))
			{
				*imgfilename = '\0';
				if (!fgets(imgfilename, PATH_MAX - (int)(imgfilename - full) - 1, input))
					break;
 				len = (int)strlen(imgfilename);
				if (len > 0 && imgfilename[len - 1] == '\n')
					imgfilename[len - 1] = 0, len--;
				if (len > 0)
				{
					if ((*imgfilename) == '#') continue; /* comment */
					data->filename[count++] = tmp;
					strcpy(tmp, &(full[0]));
					tmp += strlen(&(full[0])) + 1;
				}
			}
		}
		fclose(input);
	}

	return data;
}
/*
* icvInitBackgroundReaders
*
* Initialize background reading process.
* <cvbgreader> and <cvbgdata> are initialized.
* Must be called before any usage of background
*
* filename - name of background description file
* winsize  - size of images will be obtained from background
*
* return 1 on success, 0 otherwise.
*/
static
int icvInitBackgroundReaders(const char* filename, MySize winsize)
{
	if (cvbgdata == NULL && filename != NULL)
	{
		cvbgdata = icvCreateBackgroundData(filename, winsize);
	}
	return (cvbgdata != NULL);
}
static
int icvInitPostiveReaders(const char* filename, MySize winsize)
{
	if (cvposdata == NULL && filename != NULL)
	{
		cvposdata = icvCreateBackgroundData(filename, winsize);
	}
	return (cvposdata != NULL);
}

static void getPicture(CvHaarTrainingData* training_data,int *number_all,int number,int flag,MySize mysize)
{
	switch (flag)
	{
	case NEG_FLAG:
	{
		MyMat *tempMat = createMyMat(mysize.height, mysize.width, ONE_CHANNEL, UCHAR_TYPE);                                  //ע�����Ҫ�ͷ�	
		MyMat *tempSum = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, INT_TYPE);//ע�����Ҫ�ͷ�
	//  MyMat *tempTitle = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, INT_TYPE);//ע�����Ҫ�ͷ�
	 //   MyMat *tempSqsum = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, DOUBLE_TYPE);//ע�����Ҫ�ͷ�
		for (int i = 0;i < number;i++)
		{

			int temp = number_all[i];
			tempMat = transMat(tempMat, cvbgdata->filename[temp]);
			if (tempMat != nullptr)
			{
				//�������ͼ
		//		myIntegral(tempMat, tempSum, tempTitle, tempSqsum);
				int *address = training_data->sum.data.i;
				GetGrayIntegralImage(tempMat->data.ptr, tempSum->data.i, mysize.width, mysize.height, tempMat->step);
				//GetGraySqImage(tempMat->data.ptr, tempSqsum->data.i, mysize.width, mysize.height, tempMat->step);
				//����ͼ���Ƶ�training_data��
				address = trainingdata_number * (mysize.width + 1)*(mysize.height + 1) + address;
				memcpy(address, tempSum->data.i, sizeof(int)*(mysize.width + 1)*(mysize.height + 1));
				training_data->cls.data.fl[trainingdata_number] = 0.0;
				trainingdata_number++;
			}

		}
		releaseMyMat(tempMat);
		releaseMyMat(tempSum);
		//	releaseMyMat(tempTitle);
		//releaseMyMat(tempSqsum);
		break;
	}
	case POS_FLAG:
	{
		MyMat *tempMat = createMyMat(mysize.height, mysize.width, ONE_CHANNEL, UCHAR_TYPE);                                  //ע�����Ҫ�ͷ�	
		MyMat *tempSum = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, INT_TYPE);//ע�����Ҫ�ͷ�
	//	MyMat *tempTitle = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, INT_TYPE);//ע�����Ҫ�ͷ�
	//	MyMat *tempSqsum = createMyMat(mysize.height + 1, mysize.width + 1, ONE_CHANNEL, DOUBLE_TYPE);//ע�����Ҫ�ͷ�
		for (int i = 0;i < number;i++)
		{
			
			int temp = number_all[i];
			tempMat = transMat(tempMat, cvposdata->filename[temp]);
			if (tempMat != nullptr)
			{
				//�������ͼ
				//myIntegral(tempMat, tempSum, tempTitle, tempSqsum);
				int *address = training_data->sum.data.i;
				GetGrayIntegralImage(tempMat->data.ptr, tempSum->data.i, mysize.width, mysize.height, tempMat->step);
				//����ͼ���Ƶ�training_data��
				address = trainingdata_number * (mysize.width + 1)*(mysize.height + 1) + address;
				memcpy(address, tempSum->data.i,sizeof(int)*(mysize.width+1)*(mysize.height +1));
				training_data->cls.data.fl[trainingdata_number] = 1.0;
				trainingdata_number++;
			}

		}
		releaseMyMat(tempMat);
		releaseMyMat(tempSum);
	//	releaseMyMat(tempTitle);
	//	releaseMyMat(tempSqsum);
		break;
	}
	default:
		break;
	}
}
/*
*��������ֵ
*/
float cvEvalFastHaarFeature(const CvFastHaarFeature* feature,
	const sum_type* sum, const sum_type* tilted)
{
	const sum_type* img = feature->tilted ? tilted : sum;
	float ret = feature->rect[0].weight*
		(img[feature->rect[0].p0] - img[feature->rect[0].p1] -
			img[feature->rect[0].p2] + img[feature->rect[0].p3]) +
		feature->rect[1].weight*
		(img[feature->rect[1].p0] - img[feature->rect[1].p1] -
			img[feature->rect[1].p2] + img[feature->rect[1].p3]);

	if (feature->rect[2].weight != 0.0f)
		ret += feature->rect[2].weight *
		(img[feature->rect[2].p0] - img[feature->rect[2].p1] -
			img[feature->rect[2].p2] + img[feature->rect[2].p3]);
	return ret;
}


/*
*�������ֵ�������洦��
*numprecalculated �ڴ����� ������
*fileOrMem ����ֵ��ŵ��ļ������ڴ� 0 �ڴ棬1�ļ�
*/
static
void icvPrecalculate(int stage,int num_samples,CvHaarTrainingData* data, CvIntHaarFeatures* haarFeatures,
	int numprecalculated,int fileOrMem,const char* filedirname)
{
	switch (fileOrMem)
	{
	case SAVE_FEATURE_FILE:
	{
		//��������ֵ
		char fileName[100];
		float val = 0.0;
		//ofstream *file;
		//file = new ofstream[haarFeatures->count];
		ofstream file;
		sprintf(fileName, "%s//feature%d.txt", filedirname, stage);
		file.open(fileName, ios::out|ios::app);
		for (int i = 0; i < haarFeatures->count; i++)
		{
			for (int j = 0; j < num_samples; j++)
			{
				val = cvEvalFastHaarFeature(haarFeatures->fastfeature+i,data->sum.data.i + j * data->sum.width, data->sum.data.i);			
				file << val << " ";			
			}
			file << endl;
		}
		file.close();

	//	delete[]file;
		break;
	} 
	case SAVE_FEATURE_MEM:
	{
		
		break;
	}
	default:
		break;
	}
}
/*
*����Ȩ��
*/
static void icvSetWeightsAndClasses(CvHaarTrainingData* training_data,
	int num1, float weight1, float cls1,
	int num2, float weight2, float cls2)
{
	int j;

	assert(num1 + num2 <= training_data->maxnum);

	for (j = 0; j < num1; j++)
	{
		training_data->weights.data.fl[j] = weight1;
		training_data->cls.data.fl[j] = cls1;
	}
	for (j = num1; j < num1 + num2; j++)
	{
		training_data->weights.data.fl[j] = weight2;
		training_data->cls.data.fl[j] = cls2;
	}
}
static 
void saveXML(int stage,vector<MyStumpClassifier> strongClassifier,const char* dirname)
{
	XMLDocument doc;
	MyStumpClassifier weakClassifier;
	// ������Ԫ��<China>  
	XMLElement* root = doc.NewElement("root");
	doc.InsertEndChild(root);
	char son[100];
	for (int i = 0;i < strongClassifier.size();i++)
	{
		weakClassifier = strongClassifier[i];

		XMLElement* sonElement = doc.NewElement("weak");
		sonElement->SetAttribute("id", i);
		root->InsertEndChild(sonElement);
		XMLElement* sunElement1 = doc.NewElement("haarfeature");
		sunElement1->SetText(weakClassifier.compidx);
		sonElement->InsertEndChild(sunElement1);
		
		XMLElement* sunElement2 = doc.NewElement("error");
		sunElement2->SetText(weakClassifier.error);
		sonElement->InsertEndChild(sunElement2);
		XMLElement* sunElement3 = doc.NewElement("left");
		sunElement3->SetText(weakClassifier.left);
		sonElement->InsertEndChild(sunElement3);

		XMLElement* sunElement4 = doc.NewElement("right");
		sunElement4->SetText(weakClassifier.right);
		sonElement->InsertEndChild(sunElement4);
		XMLElement* sunElement5 = doc.NewElement("threshold");
		sunElement5->SetText(weakClassifier.threshold);
		sonElement->InsertEndChild(sunElement5);


	}
	char docName[100];
	sprintf(docName, "%s//change_stage%d.xml", dirname, stage);
	doc.SaveFile(docName);
}
/*
*��ͼƬ����Ԥ��
*/
static 
int* predict(int* preResult,int pictureNum,CvIntHaarFeatures* haarFeatures, CvHaarTrainigData* haarTrainingData, vector<MyStumpClassifier> strongClassifier)
{
	/*
	*����Ӻ� 0.5 * (a1 + a2 + a3 +...)
	*/
	int length = strongClassifier.size();
	float *at = new float[length];
	float sum = 0.0f;
	for (int i = 0;i < length;i++)
	{
		float error = strongClassifier[i].error;
		float b = error / (1 - error);
		at[i] = log(1 / b);
		sum = sum + at[i];
	}
	/*
	*��ͼƬ����Ԥ�����
	*/
	sum = sum * 0.5;
	for (int i = 0;i < pictureNum;i++)
	{
		float val = 0.0f;
		float predictSum = 0.0;
		for (int j = 0;j < length;j++)
		{
			int featureNum = strongClassifier[j].compidx;
			val = cvEvalFastHaarFeature(haarFeatures->fastfeature + featureNum, haarTrainingData->sum.data.i + i * haarTrainingData->sum.width, haarTrainingData->sum.data.i);
			if ((val < strongClassifier[j].threshold) && (1 == strongClassifier[j].left))
			{
				predictSum = predictSum + at[j];
			}
			else if ((val > strongClassifier[j].threshold) && (1 == strongClassifier[j].right))
			{
				predictSum = predictSum + at[j];
			}
		}
		if (predictSum >= sum)
			preResult[i] = 1;
		else
			preResult[i] = 0;
	}
	delete[]at;
	return preResult;

}

/*
*���㵱ǰ����boost����Ҫ��������������Ȳ���numsplits
*/
static 
void icvBoost(int stage, CvIntHaarFeatures* haarFeatures,CvHaarTrainigData* haarTrainingData,
	const char* featdirname,const char* resultname, int num_pos, int num_neg,int numsplits,int equalweights,const char* dirname, float minhitrate, float maxfalsealarms)
{
	float posweight, negweight;
	int feature_size = haarFeatures->count;
	char fileName[100];	                 
	ifstream istream;
	string str;
	float threshold;//��ֵ
	int sampleNumber = num_pos + num_neg;
	MyStumpClassifier tempWeakClassifier;//������������
	vector<MyStumpClassifier> strongClassifier;//ǿ������
	//MyStumpClassifier *currentWeakClassifier = new MyStumpClassifier[numsplits];    
	MyStumpClassifier currentWeakClassifier; //��ǰ����������
	int *vector_feat = new int[sampleNumber];              //������������סҪ�ͷ�
	int *idx = new int[sampleNumber];						//����������	
	int *m_result = new int[sampleNumber];       //��ʱ������
	int *current_result = new int[sampleNumber]; //t�׷�����
	int *strong_result = new int[sampleNumber]; //t��ǿ������
	int *predit_result = new int[sampleNumber]; //�׶�Ԥ��
												//����TP,FP
	float hitRate_real = 0;
	float maxFalse_real = 1;
	int T = 0;
	double start, end,one,two,three,four,five,six;
	//for (int T = 0; T < 50;T++)
	//while(((hitRate_real<=minhitrate)||(maxFalse_real >= maxfalsealarms))&&(T <= 200))
	while (T <= 1)
	{
		start = clock();
	//	cout <<T<< "***********************��ʼ*************************"<<endl;
		cout << T << "��ʼ" ;
		//�ļ�����
		sprintf(fileName, "%s//feature%d.txt", featdirname, stage);
		istream.open(fileName, ios::in);
		if (!istream)
		{
			printf("%s�򿪴���\n", fileName);
			return;
		}
		//����Ȩ��

		if (T == 0)
		{
			posweight = (equalweights) ? 1.0F / (num_pos + num_neg) : (0.5F / num_pos);
			negweight = (equalweights) ? 1.0F / (num_pos + num_neg) : (0.5F / num_neg);
			icvSetWeightsAndClasses(haarTrainingData,
				num_pos, posweight, 1.0F, num_neg, negweight, 0.0F);
		}
		else
		{ //m_result[]=1 ��һ��Ԥ����ȷ
			float bt;
			bt = currentWeakClassifier.error / (1.0f - currentWeakClassifier.error);
			float sum = 0.0f;
			for (int ll = 0;ll < sampleNumber;ll++)
			{
				if(strong_result[ll]==1)
				haarTrainingData->weights.data.fl[ll] = haarTrainingData->weights.data.fl[ll] * pow(bt, 1);
				else
					haarTrainingData->weights.data.fl[ll] = haarTrainingData->weights.data.fl[ll] * pow(bt, 0);
				sum = sum + haarTrainingData->weights.data.fl[ll];
			}
			//��һ��
			
			for (int ll = 0;ll < sampleNumber;ll++)
			{
				haarTrainingData->weights.data.fl[ll] = haarTrainingData->weights.data.fl[ll] / sum;
			}
		}
		//��ʼ������������

		//for (int i = 0;i < feature_size;i++)
		int i = 0;//��������ѭ������
		
//		str = "begin";
//		while (str.size()>0)   //���ж�ȡ,�������з�����
		while(getline(istream, str))
		{
	//		one = clock();
	//		getline(istream, str);
	//		two = clock();
	//		cout << "����һ�У�" << (two - one) / CLOCKS_PER_SEC * 1000 << endl;

			int result;
			//���ַ�������input�� 
			stringstream input(str);
			//cout << i << ":" << str <<endl;
			//���������result�У�������res�� 
			int count = 0;
			while (input >> result)
			{
				vector_feat[count] = result;
				idx[count] = count;
				count++;
			}	
	//		three = clock();
	//		cout << "�ֽ⵽���飺" << (three - two) / CLOCKS_PER_SEC * 1000 << endl;
			//��vector����
			bubbleSort(vector_feat, idx, sampleNumber);
			//quickSort(vector_feat, idx, 0,sampleNumber - 1);
			//four = clock();
			//cout << "����" << (four - three) / CLOCKS_PER_SEC * 1000 << endl;
			//��vector��ֵ����ɸѡ
			vector <float> threArray;
			threArray.push_back(vector_feat[0]);
			for (int ii = 0;ii < sampleNumber - 1;ii++)
			{
				if (vector_feat[ii + 1] == vector_feat[ii])
				{
					continue;
				}
				else
				{
					threArray.push_back(vector_feat[ii+1]);
				}
			}
//			five = clock();
			
			//��������ѵ��
			//for (int k = 0;k < sampleNumber - 1;k++)
			for (int k = 0;k < threArray.size() - 1;k++)
			{
				//ѭ��Ĭ����ֵ���Ϊ1���Ҳ�Ϊ0 �� �������
			//	threshold = 0.5F * (vector_feat[k] + vector_feat[k + 1]);    //��ȡ��ֵ
				threshold = 0.5F * (threArray[k] + threArray[k + 1]);    //��ȡ��ֵ
				float errorL = 0.0F; //��������            
				float errorR = 0.0F; //�Ҳ������
				float error = 0.0F; //������
				float left = 1;  //����ǩ
				float right = 0;  //�Ҳ��ǩ
				for (int n = 0;n < sampleNumber;n++)
				{
					float object = haarTrainingData->cls.data.fl[idx[n]];  //ȡ����ǰ������ǩ
					
					
					if ((vector_feat[n] < threshold) && (object != left))    //��������������
					{
						errorL = errorL + haarTrainingData->weights.data.fl[idx[n]] * 1.0;
						m_result[idx[n]] = 0;
					}
					else if (vector_feat[n] < threshold)
						m_result[idx[n]] = 1;
					if ((vector_feat[n] > threshold) && (object != right))    //����Ҳ���������
					{
						errorR = errorR + haarTrainingData->weights.data.fl[idx[n]] * 1.0;
						m_result[idx[n]] = 0;
					}
					else if (vector_feat[n] > threshold)
						m_result[idx[n]] = 1;
						
				}
				
				if (errorL + errorR > 0.5)//���Ҽ��Խ���
				{
					left = 0;
					right = 1;
					error = 1 - errorL - errorR;
					for (int mm = 0;mm < sampleNumber;mm++)
					{
						if (m_result[mm] == 0)
							m_result[mm] = 1;
						else
							m_result[mm] = 0;
					}
				}
				else
					error = errorL + errorR;
				//��������������������ԭ�����ָ������
				if (error < 0)
				{
					error = -1 * error;
					//error = 0.000000000001;
				}
				
				if (k == 0)
				{
					tempWeakClassifier.compidx = i;
					tempWeakClassifier.left = left;
					tempWeakClassifier.right = right;
					tempWeakClassifier.threshold = threshold;
					tempWeakClassifier.error = error;
					//��һ��Ԥ������¼
					for (int il = 0;il < sampleNumber;il++)
					{
						current_result[il] = m_result[il];
					}
				}
				else if (error < tempWeakClassifier.error)
				{
					tempWeakClassifier.compidx = i;
					tempWeakClassifier.left = left;
					tempWeakClassifier.right = right;
					tempWeakClassifier.threshold = threshold;
					tempWeakClassifier.error = error;
					//��һ��Ԥ������¼
					for (int il = 0;il < sampleNumber;il++)
					{
						current_result[il] = m_result[il];
					}
				}
			}
	//		six = clock();
	//		cout << "��������ѵ����" << (six - five) / CLOCKS_PER_SEC * 1000 << endl;
			if (i == 0)
			{
				currentWeakClassifier.compidx = tempWeakClassifier.compidx;
				currentWeakClassifier.left = tempWeakClassifier.left;
				currentWeakClassifier.right = tempWeakClassifier.right;
				currentWeakClassifier.threshold = tempWeakClassifier.threshold;
				currentWeakClassifier.error = tempWeakClassifier.error;

				//��һ��Ԥ������¼
				for (int il = 0;il < sampleNumber;il++)
				{
					strong_result[il] = current_result[il];
				}
			}
			else if (tempWeakClassifier.error < currentWeakClassifier.error)
			{
				currentWeakClassifier.compidx = tempWeakClassifier.compidx;
				currentWeakClassifier.left = tempWeakClassifier.left;
				currentWeakClassifier.right = tempWeakClassifier.right;
				currentWeakClassifier.threshold = tempWeakClassifier.threshold;
				currentWeakClassifier.error = tempWeakClassifier.error;
				//��һ��Ԥ������¼
				for (int il = 0;il < sampleNumber;il++)
				{
					strong_result[il] = current_result[il];
				}
			}
		
			i++;
	//		double seven = clock();
	//		cout << "��ǰ�����꣺" << (seven - one) / CLOCKS_PER_SEC * 1000 << endl;
		}
		
		strongClassifier.push_back(currentWeakClassifier);
		//Ԥ��
		/*
		predit_result = predict(predit_result,sampleNumber, haarFeatures, haarTrainingData,strongClassifier);
	
		for (int iii = 0;iii < sampleNumber;iii++)
		{
			if ((predit_result[iii] == 1) && (haarTrainingData->cls.data.fl[iii] == 1.0))
				hitRate_real++;
			else if ((predit_result[iii] == 1) && (haarTrainingData->cls.data.fl[iii] == 0.0))
				maxFalse_real++;
		}
		hitRate_real = hitRate_real / num_pos;
		maxFalse_real = maxFalse_real / num_neg;
		*/
		end = clock();
	//	cout <<"**********************"<< T << "����,��ʱ��" << (end - start) / CLOCKS_PER_SEC * 1000 <<"****************"<< endl;
		cout << T << "����,��ʱ��" << (end - start) / CLOCKS_PER_SEC * 1000 << endl;
		T++;
		istream.close();
	}
	
	saveXML(stage, strongClassifier,dirname);
 	delete[]vector_feat;
	delete[]idx;
	//delete[]m_result;
	//delete[]current_result;
	delete[]strong_result;
}
/*
* �ͷſռ�
*/
static
void icvReleaseHaarTrainingDataCache(CvHaarTrainigData** haarTrainingData)
{
	if (haarTrainingData != NULL && (*haarTrainingData) != NULL)
	{
		if ((*haarTrainingData)->valcache != NULL)
		{
			releaseMyMat((*haarTrainingData)->valcache);
			(*haarTrainingData)->valcache = NULL;
		}
		if ((*haarTrainingData)->idxcache != NULL)
		{
			releaseMyMat((*haarTrainingData)->idxcache);
			(*haarTrainingData)->idxcache = NULL;
		}
	}
}
static
void icvReleaseIntHaarFeatures(CvIntHaarFeatures** intHaarFeatures)
{
	if (intHaarFeatures != NULL && (*intHaarFeatures) != NULL)
	{
		free((*intHaarFeatures));
		(*intHaarFeatures) = NULL;
	}
}

static
void icvReleaseHaarTrainingData(CvHaarTrainigData** haarTrainingData)
{
	if (haarTrainingData != NULL && (*haarTrainingData) != NULL)
	{
		
		icvReleaseHaarTrainingDataCache(haarTrainingData);
		free((*haarTrainingData));
	}
}
static
void icvReleaseBackgroundData(CvBackgroundData** data)
{
	assert(data != NULL && (*data) != NULL);

	free((*data));
}

void myHaarTraining(const char* dirname,
	const char* posfilename,
	const char* bgfilename,
	const char* featuredir,
	int npos, int nneg, int nstages,
	int numprecalculated,
	int numsplits,
	float minhitrate, float maxfalsealarm,
	float weightfraction,
	int mode, int symmetric,
	int equalweights,
	int winwidth, int winheight,
	int boosttype, int stumperror,
	int maxtreesplits, int minpos, bool bg_vecfile,bool pos_vecfile)
{
	
	CvIntHaarFeatures* haar_features = NULL;
	CvHaarTrainingData* training_data = NULL;           //��סҪ�ͷſռ�(�Ѿ��ͷ�)
	MySize winsize;
	int *number_pos = new int[npos];  //��������ż���
	int *number_neg = new int[nneg];  //��������ż���         �Ѿ��ͷſռ�
	int current_stage = 0;
	winsize = mySize(winwidth, winheight);
	haar_features = icvCreateIntHaarFeatures(winsize, mode, symmetric); // ����haar��������
	printf("Number of features used : %d\n", haar_features->count);
	training_data = icvCreateHaarTrainingData(winsize, npos + nneg); //��ȡhaar����
	if (!bg_vecfile)
		if (!icvInitBackgroundReaders(bgfilename, winsize) && nstages > 0)
		{
			printf("Unable to read negative images");
			__MY_EXIT__
		}
	if (!pos_vecfile)
		if (!icvInitPostiveReaders(posfilename, winsize) && nstages > 0)
		{
			printf("Unable to read postive images");
			__MY_EXIT__
		}
	double start, read_image_time, calc_feature_time, end;
	cout << "����ʼ" << endl;
	start = clock();
	//����ͼ��
	number_pos = getRand(number_pos,0, cvposdata->count - 1,npos);
	number_neg = getRand(number_neg, 0, cvbgdata->count - 1, nneg);
	
	for (int i = 0;i < npos;i++)
	{
		number_pos[i] = i;
	}
	for (int i = 0;i < nneg;i++)
	{
		number_neg[i] = i;
	}
	
	//�ռ����� ����Ҫ�滻����
	trainingdata_number = 0;
	getPicture(training_data, number_pos,npos,POS_FLAG,winsize);
	getPicture(training_data, number_neg, nneg, NEG_FLAG, winsize);
	read_image_time = clock();
	cout << "�����ռ���ʱ��" << (read_image_time - start) / CLOCKS_PER_SEC * 1000 << "ms" << endl;

	//boost����
	//��������
//	icvPrecalculate(current_stage,npos+nneg,training_data, haar_features,numprecalculated, SAVE_FEATURE_FILE, featuredir);
//	cout << "ֱ�Ӽ���" << endl;
	calc_feature_time = clock();
	cout << "�������㣨���򣩺�ʱ��" << (calc_feature_time - read_image_time) / CLOCKS_PER_SEC * 1000 << "ms" << endl;
	icvBoost(current_stage, haar_features, training_data,
		featuredir, dirname, npos, nneg, numsplits, equalweights, dirname,minhitrate,maxfalsealarm);
	_MY_END_
	if (cvbgdata != NULL)
	{
		icvReleaseBackgroundData(&cvbgdata);
		cvbgdata = NULL;
	}
	if (cvposdata != NULL)
	{
		icvReleaseBackgroundData(&cvposdata);
		cvposdata = NULL;
	}
	free(number_pos);
	free(number_neg);
	icvReleaseIntHaarFeatures(&haar_features);
	icvReleaseHaarTrainingData(&training_data);
}