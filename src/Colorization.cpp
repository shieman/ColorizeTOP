#include "Colorization.h"



#include <cstdio>
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Includes for OpenCV3


//#include "opencv2/core/opengl.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
//#include "opencv2/core.hpp"


using namespace cv;
using namespace cv::dnn;
using namespace std;

// These functions are basic C function, which the DLL loader can find
// much easier than finding a C++ Class.
// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
// you are creating
extern "C"
{

DLLEXPORT
void
FillTOPPluginInfo(TOP_PluginInfo *info)
{
	// This must always be set to this constant
	info->apiVersion = TOPCPlusPlusAPIVersion;

	// Change this to change the executeMode behavior of this plugin.
	info->executeMode = TOP_ExecuteMode::CPUMemWriteOnly;

	// The opType is the unique name for this TOP. It must start with a 
	// capital A-Z character, and all the following characters must lower case
	// or numbers (a-z, 0-9)
	info->customOPInfo.opType->setString("Colorizationtop");

	// The opLabel is the text that will show up in the OP Create Dialog
	info->customOPInfo.opLabel->setString("Colorization TOP");

	// Will be turned into a 3 letter icon on the nodes
	info->customOPInfo.opIcon->setString("CLN");

	// Information about the author of this OP
	info->customOPInfo.authorName->setString("Thierry Cailheton");
	info->customOPInfo.authorEmail->setString("thierry.cailheton@laposte.net");

	// This TOP works with 0 or 1 inputs connected
	info->customOPInfo.minInputs = 1;
	info->customOPInfo.maxInputs = 1;
}


DLLEXPORT
TOP_CPlusPlusBase*
CreateTOPInstance(const OP_NodeInfo* info, TOP_Context *context)
{
	// Return a new instance of your class every time this is called.
	// It will be called once per TOP that is using the .dll
	return new Colorization(info);
}

DLLEXPORT
void
DestroyTOPInstance(TOP_CPlusPlusBase* instance, TOP_Context *context)
{
	// Delete the instance here, this will be called when
	// Touch is shutting down, when the TOP using that instance is deleted, or
	// if the TOP loads a different DLL
	delete (Colorization*)instance;
}

};

Colorization::Colorization(const OP_NodeInfo* info) : myNodeInfo(info)
{

	//ocl::setUseOpenCL(true);
 

}

Colorization::~Colorization()
{

}

void Colorization::setup()
{
	imageDownloadOptions.cpuMemPixelType = OP_CPUMemPixelType::RGBA8Fixed;
//	imageDownloadOptions.cpuMemPixelType = OP_CPUMemPixelType::RGBA32Float;
	imageDownloadOptions.verticalFlip = false;  // openCV is upside-down compared to TouchDesigner so flip the image.
	imageDownloadOptions.downloadType = OP_TOPInputDownloadType::Instant;

	isSetup = true;
}



void
Colorization::getGeneralInfo(TOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
	// Uncomment this line if you want the TOP to cook every frame even
	// if none of it's inputs/parameters are changing.
	ginfo->cookEveryFrame = false;
	//ginfo->memPixelType = OP_CPUMemPixelType::BGRA8Fixed;
	//ginfo->memPixelType = OP_CPUMemPixelType::RGBA32Float;

}

bool
Colorization::getOutputFormat(TOP_OutputFormat* format, const OP_Inputs* inputs, void* reserved1)
{
	// In this function we could assign variable values to 'format' to specify
	// the pixel format/resolution etc that we want to output to.
	// If we did that, we'd want to return true to tell the TOP to use the settings we've
	// specified.
	// In this example we'll return false and use the TOP's settings
	const OP_TOPInput* input = inputs->getInputTOP(0);
	if (!input) {
		return false;
	}

	//format->alphaChannel = true;
	//format->bitsPerChannel = 32;
	//format->floatPrecision = true;
	format->width = inputs->getInputTOP(0)->width;
	format->height = inputs->getInputTOP(0)->height;
	return true;
}

void Colorization::setupParams()
{
	static float hull_pts[] = {
	-90., -90., -90., -90., -90., -80., -80., -80., -80., -80., -80., -80., -80., -70., -70., -70., -70., -70., -70., -70., -70.,
	-70., -70., -60., -60., -60., -60., -60., -60., -60., -60., -60., -60., -60., -60., -50., -50., -50., -50., -50., -50., -50., -50.,
	-50., -50., -50., -50., -50., -50., -40., -40., -40., -40., -40., -40., -40., -40., -40., -40., -40., -40., -40., -40., -40., -30.,
	-30., -30., -30., -30., -30., -30., -30., -30., -30., -30., -30., -30., -30., -30., -30., -20., -20., -20., -20., -20., -20., -20.,
	-20., -20., -20., -20., -20., -20., -20., -20., -20., -10., -10., -10., -10., -10., -10., -10., -10., -10., -10., -10., -10., -10.,
	-10., -10., -10., -10., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 10., 10., 10., 10., 10., 10., 10.,
	10., 10., 10., 10., 10., 10., 10., 10., 10., 10., 10., 20., 20., 20., 20., 20., 20., 20., 20., 20., 20., 20., 20., 20., 20., 20.,
	20., 20., 20., 30., 30., 30., 30., 30., 30., 30., 30., 30., 30., 30., 30., 30., 30., 30., 30., 30., 30., 30., 40., 40., 40., 40.,
	40., 40., 40., 40., 40., 40., 40., 40., 40., 40., 40., 40., 40., 40., 40., 40., 50., 50., 50., 50., 50., 50., 50., 50., 50., 50.,
	50., 50., 50., 50., 50., 50., 50., 50., 50., 60., 60., 60., 60., 60., 60., 60., 60., 60., 60., 60., 60., 60., 60., 60., 60., 60.,
	60., 60., 60., 70., 70., 70., 70., 70., 70., 70., 70., 70., 70., 70., 70., 70., 70., 70., 70., 70., 70., 70., 70., 80., 80., 80.,
	80., 80., 80., 80., 80., 80., 80., 80., 80., 80., 80., 80., 80., 80., 80., 80., 90., 90., 90., 90., 90., 90., 90., 90., 90., 90.,
	90., 90., 90., 90., 90., 90., 90., 90., 90., 100., 100., 100., 100., 100., 100., 100., 100., 100., 100., 50., 60., 70., 80., 90.,
	20., 30., 40., 50., 60., 70., 80., 90., 0., 10., 20., 30., 40., 50., 60., 70., 80., 90., -20., -10., 0., 10., 20., 30., 40., 50.,
	60., 70., 80., 90., -30., -20., -10., 0., 10., 20., 30., 40., 50., 60., 70., 80., 90., 100., -40., -30., -20., -10., 0., 10., 20.,
	30., 40., 50., 60., 70., 80., 90., 100., -50., -40., -30., -20., -10., 0., 10., 20., 30., 40., 50., 60., 70., 80., 90., 100., -50.,
	-40., -30., -20., -10., 0., 10., 20., 30., 40., 50., 60., 70., 80., 90., 100., -60., -50., -40., -30., -20., -10., 0., 10., 20.,
	30., 40., 50., 60., 70., 80., 90., 100., -70., -60., -50., -40., -30., -20., -10., 0., 10., 20., 30., 40., 50., 60., 70., 80., 90.,
	100., -80., -70., -60., -50., -40., -30., -20., -10., 0., 10., 20., 30., 40., 50., 60., 70., 80., 90., -80., -70., -60., -50.,
	-40., -30., -20., -10., 0., 10., 20., 30., 40., 50., 60., 70., 80., 90., -90., -80., -70., -60., -50., -40., -30., -20., -10.,
	0., 10., 20., 30., 40., 50., 60., 70., 80., 90., -100., -90., -80., -70., -60., -50., -40., -30., -20., -10., 0., 10., 20., 30.,
	40., 50., 60., 70., 80., 90., -100., -90., -80., -70., -60., -50., -40., -30., -20., -10., 0., 10., 20., 30., 40., 50., 60., 70.,
	80., -110., -100., -90., -80., -70., -60., -50., -40., -30., -20., -10., 0., 10., 20., 30., 40., 50., 60., 70., 80., -110., -100.,
	-90., -80., -70., -60., -50., -40., -30., -20., -10., 0., 10., 20., 30., 40., 50., 60., 70., 80., -110., -100., -90., -80., -70.,
	-60., -50., -40., -30., -20., -10., 0., 10., 20., 30., 40., 50., 60., 70., -110., -100., -90., -80., -70., -60., -50., -40., -30.,
	-20., -10., 0., 10., 20., 30., 40., 50., 60., 70., -90., -80., -70., -60., -50., -40., -30., -20., -10., 0.
	};

	string protoFile = "colorization_deploy_v2.prototxt";
	string weightsFile = "colorization_release_v2.caffemodel";
	net = dnn::readNetFromCaffe(protoFile, weightsFile);
	
	net.setPreferableBackend(DNN_BACKEND_CUDA);
	net.setPreferableTarget(DNN_TARGET_CUDA);
	if (net.empty()) {
		return;
	}
	
	// setup additional layers:
	int sz[] = { 2, 313, 1, 1 };
	const Mat pts_in_hull(4, sz, CV_32F, hull_pts);
	Ptr<dnn::Layer> class8_ab = net.getLayer("class8_ab");
	class8_ab->blobs.push_back(pts_in_hull);
	Ptr<dnn::Layer> conv8_313_rh = net.getLayer("conv8_313_rh");
	conv8_313_rh->blobs.push_back(Mat(1, 313, CV_32F, Scalar(2.606)));
	
	isSetupParams = true;
}

void
Colorization::execute(TOP_OutputFormatSpecs* outputFormat,
	const OP_Inputs* inputs,
	TOP_Context *context,
	void* reserved1)
{

	// Use the first input TOP (here we assume it exists but in reality it might not)
	topInput = inputs->getInputTOP(0);
	if (!topInput) {
		myErrors = 1;
		return;
	}

	if (!isSetup) {
		setup();
	}

	videoSrc = (const uchar*)inputs->getTOPDataInCPUMemory(topInput, &imageDownloadOptions);
	if (!videoSrc) {
		myErrors = 3;
		return;
	}
	
	if (!isSetupParams)
		setupParams();

	if (!isSetupParams)
		return;

	//const char* Modelfile = inputs->getParFilePath("Modelfile");

	modulus = inputs->getParInt("Frameskip") + 1;
	frameCounter = (frameCounter + 1) % modulus;

	if (frameCounter > 0)
		return;

	in_height = topInput->height;
	in_width = topInput->width;

	out_width = outputFormat->width;
	out_height = outputFormat->height;
	
		
	framein = Mat(in_height, in_width, CV_8UC4, (void*)videoSrc);
	/*	std::vector<cv::Mat> matChannels;
		cv::split(framein, matChannels);
		alpha = matChannels.at(3);
		matChannels.pop_back();
		cv::merge(matChannels, frame);
		*/
	cv::cvtColor(framein, frame, cv::COLOR_RGBA2RGB);
	

	// extract L channel and subtract mean
	Mat lab, L, input;
	frame.convertTo(frame, CV_32F, 1.0 / 255);
	cvtColor(frame, lab, COLOR_RGB2Lab);
	extractChannel(lab, L, 0);
	resize(L, input, Size(W_in, H_in));
	input -= 50;

	// run the L channel through the network
	Mat inputBlob = blobFromImage(input);
	net.setInput(inputBlob);
	Mat result = net.forward();

	// retrieve the calculated a,b channels from the network output
	Size siz(result.size[2], result.size[3]);
	Mat a = Mat(siz, CV_32F, result.ptr(0, 0));
	Mat b = Mat(siz, CV_32F, result.ptr(0, 1));

	resize(a, a, frame.size());
	resize(b, b, frame.size());

	// merge, and convert back to BGR
	Mat coloredFrame, chn[] = { L, a, b };
	merge(chn, 3, lab);
	cvtColor(lab, coloredFrame, COLOR_Lab2BGR);

	coloredFrame = coloredFrame * 255;
	coloredFrame.convertTo(coloredFrame, CV_8U);





	
	//createAlphaImage(res,dst,alpha);
	cv::cvtColor(coloredFrame, dst, cv::COLOR_RGB2RGBA);

	mem = (uchar*)outputFormat->cpuPixelData[0];


	memcpy(mem, dst.data, dst.total()*dst.elemSize());

	outputFormat->newCPUPixelDataLocation = 0;



}



void
Colorization::setupParameters(OP_ParameterManager* manager, void *reserved1)
{

	
	{
		OP_NumericParameter	np;

		np.name = "Frameskip";
		np.label = "Frame Skip";
		np.defaultValues[0] = 0.0;
		np.clampMins[0] = true;
		np.minValues[0] = 0;
		np.minSliders[0] = 0;
		np.maxSliders[0] = 10;

		OP_ParAppendResult res = manager->appendInt(np);
		assert(res == OP_ParAppendResult::Success);
	}
}

void Colorization::createAlphaImage(const cv::Mat& mat, cv::Mat& dst, const cv::Mat& alpha)
{
	std::vector<cv::Mat> matChannels;
	cv::split(mat, matChannels);

	// create alpha channel
	matChannels.push_back(alpha);

	cv::merge(matChannels, dst);
}