#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"
#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

#include <stdio.h>
#include <iostream>
#define GLUT_NO_LIB_PRAGMA
//##### OpenGL ######
#include <GL/glew.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include <deque>

//#include "openCVStuff.h"
#include "flow.h"
#include "render.h"

#include "opencv2/core/utility.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/optflow.hpp"

#include "frameGrabber.h"

//#include "OpenPoseWrapper.h"

#include <thread>
#include <mutex>

GLFWwindow *window;

gRender grender;

gFlow gflow;

FrameGrabber fGrabber;

cv::Mat newcol;

bool newFrameReady;

/////////////////////////
// KINECT STUFF

const int screenWidth = 1920;
const int screenHeight = 1080;

int colorWidth;
int colorHeight;

//const int depthWidth = 512;
//const int depthHeight = 424;

//float *mainColor[colorWidth * colorHeight];

//unsigned char colorArray[4 * colorWidth * colorHeight];

//float previousColorArray[depthWidth * depthHeight];
//float bigDepthArray[colorWidth * (colorHeight + 2)]; // 1082 is not a typo
													 //float color[512 * 424];
//float depthArray[depthWidth * depthHeight];
//float infraredArray[depthWidth * depthHeight];
//int colorDepthMap[depthWidth * depthHeight];

// depth color points picking
//bool select_color_points_mode = false;
//bool select_depth_points_mode = false;

//std::vector<cv::Point3f> depthPoints;
//std::vector<cv::Point2f> colorPoints;
//cv::Mat newColor;

bool showDepthFlag = false;
bool showBigDepthFlag = false;
bool showInfraFlag = false;
bool showColorFlag = false;
bool showLightFlag = false;
bool showPointFlag = false;
bool useOpenPoseFlag = false;
bool pauseOpenFlowFlag = false;
bool pauseFlowFlag = false;

bool showFlowFlag = true;
bool showEdgesFlag = false;
bool showNormalFlag = false;
bool showVolumeFlag = false;
bool showTrackFlag = false;

float irBrightness = 1.0;
float irLow = 0.0f;
float irHigh = 65536.0f;
float vertFov = 40.0f;

float valA = 0.01f;
float valB = 0.01f;

bool useWebcamFlag = 1;
bool useImagesFlag = 0;
bool useVideosFlag = 0;

float xRot = 0.0f;
float zRot = 0.0f;
float yRot = 0.0f;
float xTran = 0.0f;
float yTran = 0.0f;
float zTran = 2000.0f;
void resetSliders() 
{
	vertFov = 40.0f;
	xRot = 0.0f;
	zRot = 0.0f;
	yRot = 0.0f;
	xTran = 0.0f;
	yTran = 0.0f;
	zTran = 2000.0f;
}

float zModelPC_offset = 0.0f;

//cv::Mat infraGrey;

bool calibratingFlag = false;

//////////////////////////////////////////////////
// SAVING IMAGES

// FUSION STUFF
bool trackDepthToPoint = true;
bool trackDepthToVolume = false;
int counter = 0;
bool reset = true;
bool integratingFlag = true;
bool selectInitialPoseFlag = false;

const char* sizes[] = { "32", "64", "128", "256", "384", "512", "768", "1024" };
static int sizeX = 2;
static int sizeY = 2;
static int sizeZ = 2;
float dimension = 1.0f;
float volSlice = 0.0f;

glm::vec3 iOff;

glm::vec3 initOffset(int pixX, int pixY)
{
	float z = 0;// depthArray[pixY * depthWidth + pixX] / 1000.0f;
	//kcamera.fx(), kcamera.fx(), kcamera.ppx(), kcamera.ppy()

	float x = 0;// (pixX - kcamera.ppx()) * (1.0f / kcamera.fx()) * z;
	float y = 0;// (pixY - kcamera.ppy()) * (1.0f / kcamera.fx()) * z;

	std::cout << "x " << x << " y " << y << " z " << z << std::endl;


	return glm::vec3(x, y, z);

}






// FLOW STUFF
//cv::VideoCapture cap;

std::vector<cv::Mat> imagesFromFile;
std::vector<cv::VideoCapture> videosFromFile;
int videoNumber = 0;
int videoFrameNumber = 0;

std::vector<std::pair<int, int> > resoPresetPair;
int resoPreset = 1;
int imageNumber = 0;

bool changedSource = false;

float mouseX = 0;
float mouseY = 0;



////////////// DLIB 
//dlib stuff
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <dlib/opencv.h>
dlib::frontal_face_detector faceDetector;
dlib::shape_predictor sp;
//std::string faceShapePredictor = "E:\\Data\\Faces\\shapePredictor.dat";
std::string faceShapePredictor = "resources/shapePredictor.dat";

//dlib::image_window win, win_faces;
std::mutex mtx_dlib;
std::thread *m_dlib_thread;
int m_dlib_status = 0;
bool useDLibFlag = false;
std::vector<dlib::full_object_detection> m_shapes;




////////////// OPENS POSE

// 3rdparty dependencies
// GFlags: DEFINE_bool, _int32, _int64, _uint64, _double, _string
#include <gflags/gflags.h>
// Allow Google Flags in Ubuntu 14
#ifndef GFLAGS_GFLAGS_H_
namespace gflags = google;
#endif
// OpenPose dependencies
#include <openpose/core/headers.hpp>
#include <openpose/filestream/headers.hpp>
#include <openpose/gui/headers.hpp>
#include <openpose/pose/headers.hpp>
#include <openpose/face/headers.hpp>
#include <openpose/hand/headers.hpp>
#include <openpose/utilities/headers.hpp>

// See all the available parameter options withe the `--help` flag. E.g. `build/examples/openpose/openpose.bin --help`
// Note: This command will show you flags for other unnecessary 3rdparty files. Check only the flags for the OpenPose
// executable. E.g. for `openpose.bin`, look for `Flags from examples/openpose/openpose.cpp:`.
// Debugging/Other
DEFINE_int32(logging_level, 0, "The logging level. Integer in the range [0, 255]. 0 will output any log() message, while"
	" 255 will not output any. Current OpenPose library messages are in the range 0-4: 1 for"
	" low priority messages and 4 for important ones.");
// Producer
DEFINE_string(image_path, "examples/media/COCO_val2014_000000000192.jpg", "Process the desired image.");
// OpenPose
DEFINE_string(model_pose, "COCO", "Model to be used. E.g. `COCO` (18 keypoints), `MPI` (15 keypoints, ~10% faster), "
	"`MPI_4_layers` (15 keypoints, even faster but less accurate).");
DEFINE_string(model_folder, "resources/", "Folder path (absolute or relative) where the models (pose, face, ...) are located.");
DEFINE_string(net_resolution, "368x368", "Multiples of 16. If it is increased, the accuracy potentially increases. If it is"
	" decreased, the speed increases. For maximum speed-accuracy balance, it should keep the"
	" closest aspect ratio possible to the images or videos to be processed. Using `-1` in"
	" any of the dimensions, OP will choose the optimal aspect ratio depending on the user's"
	" input value. E.g. the default `-1x368` is equivalent to `656x368` in 16:9 resolutions,"
	" e.g. full HD (1980x1080) and HD (1280x720) resolutions.");
DEFINE_string(output_resolution, "-1x-1", "The image resolution (display and output). Use \"-1x-1\" to force the program to use the"
	" input image resolution.");
DEFINE_int32(num_gpu_start, 0, "GPU device start number.");
DEFINE_double(scale_gap, 0.3, "Scale gap between scales. No effect unless scale_number > 1. Initial scale is always 1."
	" If you want to change the initial scale, you actually want to multiply the"
	" `net_resolution` by your desired initial scale.");
DEFINE_int32(scale_number, 1, "Number of scales to average.");
// OpenPose Rendering
DEFINE_bool(disable_blending, false, "If enabled, it will render the results (keypoint skeletons or heatmaps) on a black"
	" background, instead of being rendered into the original image. Related: `part_to_show`,"
	" `alpha_pose`, and `alpha_pose`.");
DEFINE_double(render_threshold, 0.05, "Only estimated keypoints whose score confidences are higher than this threshold will be"
	" rendered. Generally, a high threshold (> 0.5) will only render very clear body parts;"
	" while small thresholds (~0.1) will also output guessed and occluded keypoints, but also"
	" more false positives (i.e. wrong detections).");
DEFINE_double(alpha_pose, 0.6, "Blending factor (range 0-1) for the body part rendering. 1 will show it completely, 0 will"
	" hide it. Only valid for GPU rendering.");
DEFINE_string(face_net_resolution, "368x368", "Multiples of 16 and squared. Analogous to `net_resolution` but applied to the face keypoint"
	" detector. 320x320 usually works fine while giving a substantial speed up when multiple"
	" faces on the image.");
DEFINE_string(hand_net_resolution, "368x368", "Multiples of 16 and squared. Analogous to `net_resolution` but applied to the hand keypoint"
	" detector.");



//// OPENPOSE STUFF
std::vector<cv::Mat> detectedKeyPointsPose;
std::vector<cv::Mat> detectedKeyPointsFace;
std::vector<cv::Mat> detectedKeyPointsHands;
//op::Array<float> m_poseKeypoints;

cv::Mat col;// = cv::Mat(1080, 1920, CV_8UC3);
std::mutex mtx;
std::thread *m_thread;
int m_status = 0;
bool m_newPoseFound = false;
bool m_wipeFlowFlag = false;
bool m_useOPFace = false;

bool m_justFlowFace = false;


cv::VideoWriter outWriter;


