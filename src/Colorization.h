#include "TOP_CPlusPlusBase.h"

#include "opencv2/dnn.hpp"



class Colorization : public TOP_CPlusPlusBase
{



public:
	Colorization(const OP_NodeInfo *info);
	virtual ~Colorization();

	virtual void		getGeneralInfo(TOP_GeneralInfo *, const OP_Inputs*, void*) override;
	virtual bool		getOutputFormat(TOP_OutputFormat*, const OP_Inputs*, void*) override;


	virtual void		execute(TOP_OutputFormatSpecs*,
		const OP_Inputs*,
		TOP_Context* context,
		void* reserved1) override;

	virtual void		setupParameters(OP_ParameterManager *manager, void *reserved1) override;


private:
	void createAlphaImage(const cv::Mat& mat, cv::Mat& dst, const cv::Mat& alpha);
	void				setup();

	void				setupParams();

	// We don't need to store this pointer, but we do for the example.
	// The OP_NodeInfo class store information about the node that's using
	// this instance of the class (like its name).
	const OP_NodeInfo	*myNodeInfo;

	bool 				isSetup = false, isSetupParams = false;

	int					myErrors;
	/*
	0: No Error
	1: Input image error
	2: imae copy to cpu error
	3: dnn model load error
	*/

	OP_TOPInputDownloadOptions imageDownloadOptions;

	cv::dnn::Net net;
	const int W_in = 224;
	const int H_in = 224;

	int in_height, in_width, out_width, out_height;
	const OP_TOPInput *topInput;
	const uchar* videoSrc;
	cv::Mat framein, frame, alpha;
	cv::Mat blob;
	std::vector<cv::Mat> ress;
	cv::Mat  frameOut;
	cv::Mat res, dst;
	uchar* mem;


	int modulus, frameCounter;

	// the 313 ab cluster centers from pts_in_hull.npy (already transposed)


};