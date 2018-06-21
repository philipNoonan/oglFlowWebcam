#include "main.h"



static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error %d: %s\n", error, description);
}

void gRenderInit()
{
	grender.SetCallbackFunctions();
	grender.compileAndLinkShader();
	grender.setColorSize(colorWidth, colorHeight);

	grender.setLocations();
	grender.setVertPositions();
	grender.allocateBuffers();
	grender.setTextures(gflow.getColorTexture(), gflow.getEdgesTexture()); //needs texture uints from gfusion init
	grender.anchorMW(std::make_pair<int, int>(1920 - 512 - grender.guiPadding().first, grender.guiPadding().second));
	//krender.genTexCoordOffsets(1, 1, 1.0f);
}

void searchForMedia()
{
	videosFromFile.resize(0);
	imagesFromFile.resize(0);

	//cv::String pathVideos("videos/*.wmv"); //select only jpg
	cv::String pathVideos("videos/*.mkv"); //select only mkv

	std::vector<cv::String> fnVideos;
	cv::glob(pathVideos, fnVideos, true); // recurse



	for (size_t k = 0; k<fnVideos.size(); ++k) 
	{
		std::cout << fnVideos[k] << std::endl;

		cv::VideoCapture cap(fnVideos[k]);
		cap.set(CV_CAP_PROP_BUFFERSIZE, 5);
		if (!cap.isOpened())
		{
			std::cout << "cannot open video file" << std::endl;
			//return;
		}

		videosFromFile.push_back(cap);
	}

	outWriter.open("output/outputWiM.wmv", static_cast<int>(videosFromFile[0].get(CV_CAP_PROP_FOURCC)), 30, cv::Size(806, 540), true);
	if (!outWriter.isOpened())
	{
		std::cout << "Could not open the output video for write" << std::endl;
		//return -1;
	}

	cv::String pathImages("images/*.png"); //select only jpg
	std::vector<cv::String> fnImages;
	cv::glob(pathImages, fnImages, true); // recurse
	for (size_t k = 0; k<fnImages.size(); ++k)
	{
		std::cout << fnImages[k] << std::endl;

		cv::Mat im = cv::imread(fnImages[k]);
		if (im.empty())
		{
			std::cout << "empty image " << std::endl;
			continue; //only proceed if sucsessful
		}					  // you probably want to do some preprocessing
							  //if (k == 0)
							  //imagesFromFile.push_back(cv::Mat(im.rows, im.cols, CV_8UC3));
		imagesFromFile.push_back(im);
	}
}


void resetFlowSize()
{
	gflow.firstFrame = true;
	//gflow.clearTexturesAndBuffers();
	gflow.setNumLevels(colorWidth);
	gflow.setTextureParameters(colorWidth, colorHeight);
	gflow.allocateTextures(false);
	gflow.allocateBuffers();

	grender.setColorSize(colorWidth, colorHeight);

	fGrabber.setImageDimensions(colorWidth, colorHeight);
	fGrabber.resetImageDimensions();

	/*cap.release();

	if (cap.open(0)) 
	{
		cap.set(CV_CAP_PROP_FRAME_WIDTH, resoPresetPair[resoPreset].first);
		cap.set(CV_CAP_PROP_FRAME_HEIGHT, resoPresetPair[resoPreset].second);
	}*/


	changedSource = false;
}

void dlibInit()
{
	faceDetector = dlib::get_frontal_face_detector();
	dlib::deserialize(faceShapePredictor) >> sp;
}

void getDlibFaces()
{
	// inputs color mat and current body pose plus flow vector
	// get a cropped rect
	// affine warp rect to vertical
	// detect face
	while (m_dlib_status)
	{
		if (!m_useOPFace)
		{
			cv::Mat_ <uchar> intensity, intensityPyr, intensityROI;// , intensityLowRes;
			mtx_dlib.lock();
			cv::cvtColor(newcol, intensity, CV_RGBA2GRAY);
			mtx_dlib.unlock();

			float faceRectSize = 300.0f;
			float topLeftX, topLeftY;
			float offsetX, offsetY;
			// neck joint is from 0 - 1 this defines up

			if (detectedKeyPointsPose.size() > 0 && detectedKeyPointsPose[0].size[1] > 0)
			{
				//mtx_dlib.lock();
				topLeftX = detectedKeyPointsPose[0].at<float>(0, 0, 0); // x (1 === nose point?)
				topLeftY = detectedKeyPointsPose[0].at<float>(0, 0, 1); // y

				topLeftX = topLeftX - (faceRectSize / 2) < 0 ? (faceRectSize / 2) : topLeftX;
				topLeftX = topLeftX + (faceRectSize / 2) > intensity.cols - 1 ? intensity.cols - 1 - (faceRectSize / 2) : topLeftX;

				topLeftY = topLeftY - (faceRectSize / 2) < 0 ? (faceRectSize / 2) : topLeftY;
				topLeftY = topLeftY + (faceRectSize / 2) > intensity.rows - 1 ? intensity.rows - 1 - (faceRectSize / 2) : topLeftY;
				//mtx_dlib.unlock();
				//cv::imshow("rec", intensity(cv::Rect(topLeftX - (faceRectSize / 2), topLeftY - (faceRectSize / 2), faceRectSize, faceRectSize)));
				//cv::waitKey(1);
				intensityROI = intensity(cv::Rect(topLeftX - (faceRectSize / 2), topLeftY - (faceRectSize / 2), faceRectSize, faceRectSize));
				cv::pyrDown(intensityROI, intensityPyr);
				offsetX = topLeftX - (faceRectSize / 2);
				offsetY = topLeftY - (faceRectSize / 2);
			}
			else
			{
				cv::pyrDown(intensity, intensityPyr);
				offsetX = 0;
				offsetY = 0;
			}

			//std::cout << offsetX << " " << offsetY << std::endl;
			//cv::resize(intensity, intensityPyr, cv::Size(960, 540));
			//cv::pyrDown(intensity, intensityPyr);
			//newColor.copyTo(intensity);

			cv::imshow("IR", intensityPyr);
			cv::waitKey(1);
			dlib::cv_image<uchar> cimg;
	
			cimg = intensityPyr;
			


			std::vector<dlib::rectangle> face_detections = faceDetector(cimg, -0.1);

			std::vector<dlib::full_object_detection> shapes;


			//win.clear_overlay();
			//win.set_image(cimg);

			for (unsigned long j = 0; j < face_detections.size(); ++j)
			{
				dlib::full_object_detection shape = sp(cimg, face_detections[j]);

				// You get the idea, you can get all the face part locations if
				// you want them.  Here we just store them in shapes so we can
				// put them on the screen.
				shapes.push_back(shape);

				//win.add_overlay(render_face_detections(shapes));

				//std::cout << shape.part(0).x() << " " << shape.part(0).y() << std::endl;

				//dlib::array<dlib::array2d<dlib::rgb_pixel> > face_chips;
				//extract_image_chips(cimg, get_face_chip_details(shapes), face_chips);
				//win_faces.set_image(tile_images(face_chips));

				std::vector<float> personFace;
				personFace.resize(shapes[0].num_parts() * 3);
				for (int i = 0; i < shapes[0].num_parts() * 3; i += 3)
				{
					personFace[i] = 2 * shapes[0].part(i / 3).x() + offsetX; // x
					personFace[i + 1] = 2 * shapes[0].part(i / 3).y() + offsetY; // x
					personFace[i + 2] = 1.0f; // y
				}
				grender.setFacesPoints(personFace);

			}

			mtx_dlib.lock();
			m_wipeFlowFlag = true;
			//m_shapes = shapes;
			mtx_dlib.unlock();

		}
// sleep for a bit?


	}
	



}

void getOPose()
{
	// OPENPOSE STUFF
	//OpenPoseWrapper OP{ cv::Size(320,240), cv::Size(240,240), cv::Size(960,540), "MPI", "resources/", 0, false, OpenPoseWrapper::ScaleMode::PlusMinusOne, true, true };
	
	op::log("OpenPose Starting.", op::Priority::High);
	// ------------------------- INITIALIZATION -------------------------
	// Step 1 - Set logging level
	// - 0 will output all the logging messages
	// - 255 will output nothing
	op::check(0 <= FLAGS_logging_level && FLAGS_logging_level <= 255, "Wrong logging_level value.",
		__LINE__, __FUNCTION__, __FILE__);
	op::ConfigureLog::setPriorityThreshold((op::Priority)FLAGS_logging_level);
	op::log("", op::Priority::Low, __LINE__, __FUNCTION__, __FILE__);
	// Step 2 - Read Google flags (user defined configuration)
	// outputSize
	const auto outputSize = op::flagsToPoint(FLAGS_output_resolution, "-1x-1");
	// netInputSize
	const auto netInputSize = op::flagsToPoint(FLAGS_net_resolution, "368x368");
	// poseModel
	const auto poseModel = op::flagsToPoseModel(FLAGS_model_pose);
	// faceModel
	const auto faceNetInputSize = op::flagsToPoint(FLAGS_face_net_resolution, "368x368 (multiples of 16)");
	// faceModel
	const auto handNetInputSize = op::flagsToPoint(FLAGS_hand_net_resolution, "368x368 (multiples of 16)");

	// Check no contradictory flags enabled
	if (FLAGS_alpha_pose < 0. || FLAGS_alpha_pose > 1.)
		op::error("Alpha value for blending must be in the range [0,1].", __LINE__, __FUNCTION__, __FILE__);
	if (FLAGS_scale_gap <= 0. && FLAGS_scale_number > 1)
		op::error("Incompatible flag configuration: scale_gap must be greater than 0 or scale_number = 1.",
			__LINE__, __FUNCTION__, __FILE__);
	// Logging
	op::log("", op::Priority::Low, __LINE__, __FUNCTION__, __FILE__);
	// Step 3 - Initialize all required classes
	op::ScaleAndSizeExtractor scaleAndSizeExtractor(netInputSize, outputSize, FLAGS_scale_number, FLAGS_scale_gap);
	op::CvMatToOpInput cvMatToOpInput;
	op::CvMatToOpOutput cvMatToOpOutput;
	op::PoseExtractorCaffe poseExtractorCaffe{ poseModel, FLAGS_model_folder, FLAGS_num_gpu_start };
	op::PoseCpuRenderer poseRenderer{ poseModel, (float)FLAGS_render_threshold, !FLAGS_disable_blending,
		(float)FLAGS_alpha_pose };



	op::FaceExtractorCaffe faceExtractor{ faceNetInputSize, faceNetInputSize, FLAGS_model_folder, FLAGS_num_gpu_start };
	op::FaceDetector faceDetector{ poseModel };
	op::FaceCpuRenderer faceRenderer{ 0.4f };
	faceExtractor.initializationOnThread();
	faceRenderer.initializationOnThread();


	op::HandExtractorCaffe handExtractor{ handNetInputSize, handNetInputSize, FLAGS_model_folder, FLAGS_num_gpu_start };
	op::HandDetector handDetector{ poseModel };
	op::HandCpuRenderer handRenderer{ 0.2f };


	op::OpOutputToCvMat opOutputToCvMat;


	poseExtractorCaffe.initializationOnThread();
	poseRenderer.initializationOnThread();
	


	handExtractor.initializationOnThread();
	handRenderer.initializationOnThread(); 





	while (m_status == 1)
	{
		std::vector<cv::Mat> m_detectedKeyPointsPose;
		std::vector<cv::Mat> m_detectedKeyPointsFace;
		std::vector<cv::Mat> m_detectedKeyPointsHands;
		cv::Mat inputImage, colRGBA, pyrDownImage;

		mtx.lock();
		col.copyTo(colRGBA); // DONT NEED TO COLOR CONVERT??

		mtx.unlock();
		cv::cvtColor(colRGBA, inputImage, cv::COLOR_RGBA2RGB);


		if (inputImage.empty())
			op::error("Could not open or find the image: " + FLAGS_image_path, __LINE__, __FUNCTION__, __FILE__);
		const op::Point<int> imageSize{ inputImage.cols, inputImage.rows };
		// Step 2 - Get desired scale sizes
		std::vector<double> scaleInputToNetInputs;
		std::vector<op::Point<int>> netInputSizes;
		double scaleInputToOutput;
		op::Point<int> outputResolution;
		std::tie(scaleInputToNetInputs, netInputSizes, scaleInputToOutput, outputResolution)
			= scaleAndSizeExtractor.extract(imageSize);
		// Step 3 - Format input image to OpenPose input and output formats
		const auto netInputArray = cvMatToOpInput.createArray(inputImage, scaleInputToNetInputs, netInputSizes);
		auto outputArray = cvMatToOpOutput.createArray(inputImage, scaleInputToOutput, outputResolution);
		// Step 4 - Estimate poseKeypoints
		poseExtractorCaffe.forwardPass(netInputArray, imageSize, scaleInputToNetInputs);
		const auto poseKeypoints = poseExtractorCaffe.getPoseKeypoints();

		/*mtx.lock();
		m_poseKeypoints = poseKeypoints.clone();
		mtx.unlock();*/

		const auto faceRectsOP = faceDetector.detectFaces(poseKeypoints, 1.0f);

		faceExtractor.forwardPass(faceRectsOP, inputImage, 1.0f);


		const auto handRectsOP = handDetector.detectHands(poseKeypoints, 1.0f);
		handExtractor.forwardPass(handRectsOP, inputImage, 1.0f);

		// Step 5 - Render poseKeypoints
		//poseRenderer.renderPose(outputArray, poseKeypoints, scaleInputToOutput);
		//const auto faceKeypoints = faceExtractor.getFaceKeypoints();
		//faceRenderer.renderFace(outputArray, faceKeypoints);
		//const auto handKeypoints = handExtractor.getHandKeypoints();
		//handRenderer.renderHand(outputArray, handKeypoints);
		// Step 6 - OpenPose output format to cv::Mat
		//auto outputImage = opOutputToCvMat.formatToCvMat(outputArray);

		auto faces = faceExtractor.getFaceKeypoints();
		m_detectedKeyPointsFace.push_back(faces.getConstCvMat().clone());

		auto hands = handExtractor.getHandKeypoints();
		m_detectedKeyPointsHands.push_back(hands[0].getConstCvMat().clone());
		m_detectedKeyPointsHands.push_back(hands[1].getConstCvMat().clone());

		auto poses = poseExtractorCaffe.getPoseKeypoints().clone();
		m_detectedKeyPointsPose.push_back(poses.getConstCvMat().clone());


		//cv::Mat outputImage = OP.render(pyrDownImageInput);
		//cv::imshow("pose", outputImage);
		//cv::waitKey(1);
		//std::cout << "found poses " << m_detectedKeyPointsPose[0].size() << std::endl;
		mtx.lock();
		m_newPoseFound = true;
		detectedKeyPointsPose = m_detectedKeyPointsPose;
		detectedKeyPointsFace = m_detectedKeyPointsFace;
		detectedKeyPointsHands = m_detectedKeyPointsHands;
		mtx.unlock();

	}

	std::cout << "exiting openpose thread " << std::endl;

	


}




int main(int, char**)
{


	int display_w, display_h;
	// load openGL window
	window = grender.loadGLFWWindow();

	glfwGetFramebufferSize(window, &display_w, &display_h);
	// Setup ImGui binding
	ImGui_ImplGlfwGL3_Init(window, true);
	ImVec4 clear_color = ImColor(114, 144, 154);

	resoPresetPair.push_back(std::make_pair(640, 480));
	resoPresetPair.push_back(std::make_pair(960, 540));
	resoPresetPair.push_back(std::make_pair(1280, 720));
	resoPresetPair.push_back(std::make_pair(1920, 1080));

	colorWidth = resoPresetPair[resoPreset].first;
	colorHeight = resoPresetPair[resoPreset].second;

	gflow.setupEKF();
	// op flow init
	gflow.compileAndLinkShader();
	gflow.setLocations();

	gflow.setNumLevels(colorWidth);
	gflow.setTextureParameters(colorWidth, colorHeight);
	gflow.allocateTextures(false);

	gflow.allocateBuffers();

	gRenderInit();

	dlibInit();

	fGrabber.start();
	fGrabber.setImageDimensions(colorWidth, colorHeight);

	//const auto faceNetInputSize = op::flagsToPoint(FLAGS_face_net_resolution, "368x368 (multiples of 16)");
	//const auto poseModel = op::flagsToPoseModel(FLAGS_model_pose);

	//op::FaceExtractorCaffe faceExtractor{ faceNetInputSize, faceNetInputSize, FLAGS_model_folder, FLAGS_num_gpu_start };
	//op::FaceDetector faceDetector{ poseModel };
	//op::FaceCpuRenderer faceRenderer{ 0.4f };
	//faceExtractor.initializationOnThread();
	//faceRenderer.initializationOnThread();


	/*if (!cap.open(0))
		return 0;
	cap.set(CV_CAP_PROP_FRAME_WIDTH, colorWidth);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, colorHeight);*/



	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		glfwGetFramebufferSize(window, &display_w, &display_h);
		grender.renderScaleHeight((float)display_h / 1080.0f);
		grender.renderScaleWidth((float)display_w / 1920.0f);

		grender.anchorMW(std::make_pair<int, int>(50, 1080 - 424 - 50 ));

		//// Rendering
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		if (m_justFlowFace)
		{
			float topLeftX, topLeftY;
			float offsetX, offsetY;
			float faceRectSize = 400;
			// neck joint is from 0 - 1 this defines up
			//std::cout << " pre " << std::endl;
			//cap >> col;

			//if (m_newPoseFound && !pauseOpenFlowFlag)
			//{
			//	if (detectedKeyPointsPose[0].size().height != 0 && detectedKeyPointsPose[0].size[1] > 0)
			//	{

			//		topLeftX = detectedKeyPointsPose[0].at<float>(0, 0, 0); // x (1 === nose point?)
			//		topLeftY = detectedKeyPointsPose[0].at<float>(0, 0, 1); // y

			//		topLeftX = topLeftX - (faceRectSize / 2) < 0 ? (faceRectSize / 2) : topLeftX;
			//		topLeftX = topLeftX + (faceRectSize / 2) > col.cols - 1 ? col.cols - 1 - (faceRectSize / 2) : topLeftX;

			//		topLeftY = topLeftY - (faceRectSize / 2) < 0 ? (faceRectSize / 2) : topLeftY;
			//		topLeftY = topLeftY + (faceRectSize / 2) > col.rows - 1 ? col.rows - 1 - (faceRectSize / 2) : topLeftY;

			//		cv::Mat tcol = col(cv::Rect(topLeftX - (faceRectSize / 2), topLeftY - (faceRectSize / 2), faceRectSize, faceRectSize));


			//		col = tcol;
			//	}
			//}
			//else
			//{
				cv::Mat tcol = col(cv::Rect(1920 / 2 - 200, 1080 / 2 - 200, faceRectSize, faceRectSize));


				col = tcol;
			//}
		}
		else
		{

			if (useVideosFlag) // send me to a seperate thread
			{
				cv::Mat tempCol;
				videosFromFile[videoNumber].set(CV_CAP_PROP_POS_FRAMES, videoFrameNumber);
				videoFrameNumber++;
				if (videoFrameNumber > videosFromFile[videoNumber].get(CV_CAP_PROP_FRAME_COUNT) - 50)
				{
					gflow.wipeFlow();
					gflow.firstFrame = true;
					videoFrameNumber = 0;
				}

				videosFromFile[videoNumber] >> tempCol;

				tempCol.copyTo(col);
				newFrameReady = true;


				//col = fGrabber.framesVideo(newFrameReady);


			}
			else if (useImagesFlag)
			{
				cv::Mat tempCol = imagesFromFile[imageNumber];
				if (imageNumber == imagesFromFile.size() - 1 ? imageNumber = 0 : imageNumber++);
				if (imageNumber == 0)
				{
					gflow.wipeFlow();
					gflow.firstFrame = true;
				}
				if (imageNumber % 10 == 0)
				{
					gflow.clearPoints();
				}
				cv::imshow("sdf", tempCol);
				cv::waitKey(20);

				tempCol.copyTo(col);
				newFrameReady = true;
			}
			else if (useWebcamFlag)
			{
				col = fGrabber.frames(newFrameReady);
				//std::cout << col.rows << " " << col.cols << std::endl;
				//std::cout << " xx " << colorHeight << " " << colorWidth << std::endl;

				//cap >> col;
			}
		}

		if (newFrameReady)
		{
			newFrameReady = false;
			cv::cvtColor(col, newcol, CV_RGB2RGBA, 4);

			gflow.setTexture(newcol.data);

			gflow.calc(false);


			grender.setTrackedPointsBuffer(gflow.getTrackedPointsBuffer());

			grender.setFlowTexture(gflow.getFlowTexture());

			if (useOpenPoseFlag)
			{
				// detectedKeyPointsPose is a [n][k][3] opencv mat
				// n number of items detected
				// k number of points for each item
				// 3 is x y and weight
				if (m_newPoseFound && !pauseOpenFlowFlag)
				{
					if (detectedKeyPointsFace.size() > 0 && m_useOPFace)
					{
						std::vector<float> personFace;
						personFace.resize(detectedKeyPointsFace[0].size[1] * detectedKeyPointsFace[0].size[2]);
						for (int i = 0; i < detectedKeyPointsFace[0].size[1] * 3; i += 3)
						{
							personFace[i] = detectedKeyPointsFace[0].at<float>(0, i / 3, 0); // x
							personFace[i + 1] = detectedKeyPointsFace[0].at<float>(0, i / 3, 1); // y
							personFace[i + 2] = detectedKeyPointsFace[0].at<float>(0, i / 3, 2); // y
						}
						grender.setFacesPoints(personFace);
					}

					if (detectedKeyPointsPose.size() > 0)
					{
						std::vector<float> personPose;
						personPose.resize(detectedKeyPointsPose[0].size[1] * detectedKeyPointsPose[0].size[2]);
						for (int i = 0; i < detectedKeyPointsPose[0].size[1] * 3; i += 3)
						{
							personPose[i] = detectedKeyPointsPose[0].at<float>(0, i / 3, 0); // x
							personPose[i + 1] = detectedKeyPointsPose[0].at<float>(0, i / 3, 1); // y
							personPose[i + 2] = detectedKeyPointsPose[0].at<float>(0, i / 3, 2); // y
						}
						grender.setPosePoints(personPose);
					}

					if (detectedKeyPointsHands.size() > 0)
					{

						std::vector<float> personHands;

						personHands.resize(detectedKeyPointsHands[0].size[1] * detectedKeyPointsHands[0].size[2] * 2);
						for (int i = 0; i < detectedKeyPointsHands[0].size[1] * 3; i += 3)
						{
							personHands[i] = detectedKeyPointsHands[0].at<float>(0, i / 3, 0); // x
							personHands[i + 1] = detectedKeyPointsHands[0].at<float>(0, i / 3, 1); // y
							personHands[i + 2] = detectedKeyPointsHands[0].at<float>(0, i / 3, 2); // y
						}
						for (int i = 0; i < detectedKeyPointsHands[1].size[1] * 3; i += 3)
						{
							personHands[i + (21 * 3)] = detectedKeyPointsHands[1].at<float>(0, i / 3, 0); // x
							personHands[i + (21 * 3) + 1] = detectedKeyPointsHands[1].at<float>(0, i / 3, 1); // y
							personHands[i + (21 * 3) + 2] = detectedKeyPointsHands[1].at<float>(0, i / 3, 2); // y
						}
						grender.setHandsPoints(personHands);

					}

					mtx.lock();
					m_newPoseFound = false;
					m_wipeFlowFlag = true;
					mtx.unlock();
				}
			}
			grender.bindOpenPosePoints();

			//getDlibFaces();

			if (!pauseFlowFlag)
			{
				gflow.track(grender.getPoseBuffer(), 18);
				gflow.track(grender.getFaceBuffer(), 70);
				//gflow.track(grender.getHandsBuffer(), );


			}

			if (m_wipeFlowFlag)
			{
				gflow.wipeSumFlow();
				m_wipeFlowFlag = false;
			}


			//getOPose();
			//auto outputArray = cvMatToOpOutput.createArray(col, scaleInputToOutput, outputResolution);
			//if (detectedKeyPointsPose.size() > 0)
			//{
			//	detectedKeyPointsFace.resize(1);
			//	const auto faceRectsOP = faceDetector.detectFaces(m_poseKeypoints, 1.0f);
			//	faceExtractor.forwardPass(faceRectsOP, col, 1.0f);
			//	const auto faceKeypoints = faceExtractor.getFaceKeypoints();

			//	detectedKeyPointsFace.push_back(faceKeypoints.getConstCvMat().clone());

			//	if (detectedKeyPointsFace.size() > 0)
			//	{
			//		std::vector<float> personFace;
			//		personFace.resize(detectedKeyPointsFace[0].size[1] * detectedKeyPointsFace[0].size[2]);
			//		for (int i = 0; i < detectedKeyPointsFace[0].size[1] * 3; i += 3)
			//		{
			//			personFace[i] = detectedKeyPointsFace[0].at<float>(0, i / 3, 0); // x
			//			personFace[i + 1] = detectedKeyPointsFace[0].at<float>(0, i / 3, 1); // y
			//			personFace[i + 2] = detectedKeyPointsFace[0].at<float>(0, i / 3, 2); // y
			//		}
			//		grender.setFacesPoints(personFace);
			//	}
			//}



			//faceRenderer.renderFace(outputArray, faceKeypoints);

			//auto outputImage = opOutputToCvMat.formatToCvMat(outputArray);

			cv::Mat totflow = cv::Mat(colorHeight, colorWidth, CV_32FC2);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, gflow.getFlowTexture());
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, totflow.data);// this is importnant, you are using GL_RED_INTEGETER!!!!
			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(0);

			cv::Mat tofl[2];
			cv::split(totflow, tofl);

			//cv::imshow("wwee", tofl[0] - tofl[1]);
			//cv::imshow("dwerwev", tofl[1]); 


			cv::Mat mag, ang;
			cv::Mat hsv_split[3], hsv;
			cv::Mat rgb;
			cv::cartToPolar(tofl[0], tofl[1], mag, ang, true);
			//cv::normalize(mag, mag, 0, 1, cv::NORM_MINMAX);
			hsv_split[0] = ang;
			hsv_split[1] = mag * 0.04;
			hsv_split[2] = cv::Mat::ones(ang.size(), ang.type());
			cv::merge(hsv_split, 3, hsv);
			cv::cvtColor(hsv, rgb, cv::COLOR_HSV2BGR);
			cv::Mat outrgb, outmix;
			rgb.convertTo(outrgb, CV_8UC4, 255);
			cv::addWeighted(outrgb, 0.7, col, 0.3, 1.0, outmix);
			cv::imshow("totflowrgb", outrgb);


			outWriter << outmix;
		}
			


			glfwPollEvents();
			ImGui_ImplGlfwGL3_NewFrame();

			grender.setRenderingOptions(showDepthFlag, showBigDepthFlag, showInfraFlag, showColorFlag, showLightFlag, showPointFlag, showFlowFlag, showEdgesFlag, showNormalFlag, showVolumeFlag, showTrackFlag);

				grender.setColorImageRenderPosition(vertFov);

				grender.setFlowImageRenderPosition(colorHeight, colorWidth, vertFov);

				grender.setViewMatrix(xRot, yRot, zRot, xTran, yTran, zTran);
				grender.setProjectionMatrix();

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);

			grender.Render(false);


			if (grender.showImgui())
			{
				ImGui::SetNextWindowPos(ImVec2(1600 - 32 - 528 - 150, 32));
				ImGui::SetNextWindowSize(ImVec2(528 + 150, 424), ImGuiSetCond_Always);
				ImGuiWindowFlags window_flags = 0;
				window_flags |= ImGuiWindowFlags_NoTitleBar;
				//window_flags |= ImGuiWindowFlags_ShowBorders;
				window_flags |= ImGuiWindowFlags_NoResize;
				window_flags |= ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoCollapse;

				float arr[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
				arr[0] = gflow.getTimeElapsed();
				arr[8] = arr[0] + arr[1] + arr[2] + arr[3] + arr[4] + arr[5] + arr[6] + arr[7];
				GLint total_mem_kb = 0;
				glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX,
					&total_mem_kb);

				GLint cur_avail_mem_kb = 0;
				glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX,
					&cur_avail_mem_kb);

				bool showGUI = grender.showImgui();
				ImGui::Begin("Menu", &showGUI, window_flags);
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", arr[8], 1000.0f / arr[8]);
				ImGui::Text("GPU Memory Usage %d MB out of %d (%.1f %%)", (total_mem_kb - cur_avail_mem_kb) / 1024, total_mem_kb / 1024, 100.0f * (1.0f - (float)cur_avail_mem_kb / (float)total_mem_kb));




				//ImGui::PushItemWidth(-krender.guiPadding().first);
				//ImGui::SetWindowPos(ImVec2(display_w - (display_w / 4) - krender.guiPadding().first, ((krender.guiPadding().second) + (0))));
				ImGui::Text("Help menu - press 'H' to hide");
				ImGui::Separator();
				ImGui::Text("Scan for media");
				if (ImGui::Button("Scan")) searchForMedia();
				
				if (m_justFlowFace)
				{
					//useWebcamFlag = 1;
					//useImagesFlag = 0;
					//useVideosFlag = 0;
					//changedSource = 1;
					//colorWidth = 200;
					//colorHeight = 200;
				}
				else
				{
					if (videosFromFile.size() > 0)
					{
						if (ImGui::Button("Use webcam")) {
							useWebcamFlag = 1;
							useImagesFlag = 0;
							useVideosFlag = 0;
							changedSource = 1;
							colorWidth = resoPresetPair[resoPreset].first;
							colorHeight = resoPresetPair[resoPreset].second;
						}

						int previousPreset = resoPreset;
						ImGui::SameLine();
						ImGui::Checkbox("", &useWebcamFlag);
						ImGui::SameLine();
						ImGui::SliderInt("resolution preset", &resoPreset, 0, resoPresetPair.size() - 1);
						if (resoPreset != previousPreset)
						{
							changedSource = 1;
							colorWidth = resoPresetPair[resoPreset].first;
							colorHeight = resoPresetPair[resoPreset].second;
						}
					}

					if (imagesFromFile.size() > 0)
					{
						if (ImGui::Button("Use images")) {
							useWebcamFlag = 0;
							useImagesFlag = 1;
							useVideosFlag = 0;
							changedSource = 1;
							colorWidth = imagesFromFile[0].cols;
							colorHeight = imagesFromFile[0].rows;
						}
						ImGui::SameLine();
						ImGui::Checkbox("", &useImagesFlag);
					}

					if (videosFromFile.size() > 0)
					{
						if (ImGui::Button("Use videos")) {
							useWebcamFlag = 0;
							useImagesFlag = 0;
							useVideosFlag = 1;
							changedSource = 1;
							colorWidth = videosFromFile[videoNumber].get(CV_CAP_PROP_FRAME_WIDTH);
							colorHeight = videosFromFile[videoNumber].get(CV_CAP_PROP_FRAME_HEIGHT);

							//// WORK OUT WHAT HAPPENS WHEN YOU SWITCH SOURCE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
							//fGrabber.setVideoCaptureTarget(videosFromFile[videoNumber]);
							//fGrabber.startVideo();


						}
						int previousVideoNumber = videoNumber;
						ImGui::SameLine();
						ImGui::Checkbox("", &useVideosFlag);
						ImGui::SameLine();
						ImGui::SliderInt("video file", &videoNumber, 0, videosFromFile.size() - 1);

						if (videoNumber != previousVideoNumber)
						{
							fGrabber.setVideoCaptureTarget(videosFromFile[videoNumber]);
							changedSource = 1;  
							colorWidth = videosFromFile[videoNumber].get(CV_CAP_PROP_FRAME_WIDTH);
							colorHeight = videosFromFile[videoNumber].get(CV_CAP_PROP_FRAME_HEIGHT);
						}
					}
				}
				

				ImGui::Separator();
				ImGui::Text("openPose Options");
				if (ImGui::Button("use openPose"))
				{
					useOpenPoseFlag ^= 1; 
					if (useOpenPoseFlag)
					{
						std::cout << "starting thread" << std::endl;
						if (m_status == 0)
						{
							m_status = 1;
							m_thread = new std::thread(getOPose);

						}
					}
					else
					{
						if (m_status == 1)
						{
							m_status = 0;
							if (m_thread->joinable())
							{
								m_thread->join();
							}

							m_thread = nullptr;

						}
					}
				}
				ImGui::SameLine(); ImGui::Checkbox("", &useOpenPoseFlag);
				if (ImGui::Button("pause openPose")) pauseOpenFlowFlag ^= 1;  ImGui::SameLine(); ImGui::Checkbox("", &pauseOpenFlowFlag);
				if (ImGui::Button("pause flow")) pauseFlowFlag ^= 1;  ImGui::SameLine(); ImGui::Checkbox("", &pauseFlowFlag);
				if (ImGui::Button("use OPFace")) m_useOPFace ^= 1;  ImGui::SameLine(); ImGui::Checkbox("", &m_useOPFace);

				if (ImGui::Button("use DLib"))
				{
					useDLibFlag ^= 1;
					if (useDLibFlag)
					{
						std::cout << "starting dlib thread" << std::endl;
						if (m_dlib_status == 0)
						{
							m_dlib_status = 1;
							m_dlib_thread = new std::thread(getDlibFaces);

						}
					}
					else
					{
						if (m_dlib_status == 1)
						{
							m_dlib_status = 0;
							if (m_dlib_thread->joinable())
							{
								m_dlib_thread->join();
							}

							m_dlib_thread = nullptr;

						}
					}
				}
				ImGui::SameLine(); ImGui::Checkbox("", &useDLibFlag);

				if (ImGui::Button("just flow face"))
				{
					useWebcamFlag = 1;
					useImagesFlag = 0;
					useVideosFlag = 0;
					changedSource = 1;
					colorWidth = 400;
					colorHeight = 400;
					resoPreset = 3;

					m_justFlowFace ^= 1;
				}
				ImGui::SameLine(); ImGui::Checkbox("", &m_justFlowFace);



				ImGui::Separator();
				ImGui::Text("View Options");

				if (ImGui::Button("Show Color")) showColorFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showColorFlag); ImGui::SameLine(); if (ImGui::Button("Show Flow")) showFlowFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showFlowFlag);
				
				if (ImGui::Button("Show Point")) showPointFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showPointFlag); ImGui::SameLine(); if (ImGui::Button("Show Edges")) showEdgesFlag ^= 1; ImGui::SameLine(); ImGui::Checkbox("", &showEdgesFlag);

				ImGui::Separator();
				ImGui::Text("Other Options");

				if (ImGui::Button("Reset flow points")) gflow.clearPoints();
				//if (ImGui::Button("Reset")) OCVStuff.resetColorPoints();

				//if (ImGui::Button("Reset Depth")) krender.resetRegistrationMatrix();

				//if (ImGui::Button("Export PLY")) krender.setExportPly(true);
				//if (ImGui::Button("Export PLY")) krender.exportPointCloud();
				//if (ImGui::Button("Save Color")) OCVStuff.saveImage(0); // saving color image (flag == 0)


				ImGui::Separator();
				ImGui::Text("View Transforms");
				ImGui::SliderFloat("vFOV", &vertFov, 1.0f, 90.0f);

				ImGui::SliderFloat("valA", &valA, 0.00001f, 0.5f);
				ImGui::SliderFloat("valB", &valB, 0.00001f, 0.5f);

				grender.setFov(vertFov);
				gflow.setVals(valA, valB);

				if (ImGui::Button("Reset Sliders")) resetSliders();




				ImGui::End();

			}




			ImGui::Render();

			if (changedSource)
			{
				// if use video
				// do soemthign 
				// if use webcam
				// do somethign else
				resetFlowSize();
			}


			//grender.setComputeWindowPosition();
			//gfusion.render();
			glfwSwapBuffers(window);
		}

	

	// Cleanup DO SOME CLEANING!!!
	ImGui_ImplGlfwGL3_Shutdown();


	//krender.cleanUp();

	return 0;
}