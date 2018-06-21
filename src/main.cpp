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

	//cv::String pathVideos("videos/*.wmv"); //select only wmv
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



int main(int, char**)
{


	int display_w, display_h;
	// load openGL window
	window = grender.loadGLFWWindow();

	glfwGetFramebufferSize(window, &display_w, &display_h);
	// Setup ImGui binding
	ImGui::CreateContext();

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


			//cv::Mat mag, ang;
			//cv::Mat hsv_split[3], hsv;
			//cv::Mat rgb;
			//cv::cartToPolar(tofl[0], tofl[1], mag, ang, true);
			////cv::normalize(mag, mag, 0, 1, cv::NORM_MINMAX);
			//hsv_split[0] = ang;
			//hsv_split[1] = mag * 0.04;
			//hsv_split[2] = cv::Mat::ones(ang.size(), ang.type());
			//cv::merge(hsv_split, 3, hsv);
			//cv::cvtColor(hsv, rgb, cv::COLOR_HSV2BGR);
			//cv::Mat outrgb, outmix;
			//rgb.convertTo(outrgb, CV_8UC4, 255);
			//cv::addWeighted(outrgb, 0.7, col, 0.3, 1.0, outmix);
			//cv::imshow("totflowrgb", outrgb);


			//outWriter << outmix;
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
			ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

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
	ImGui::DestroyContext();
	glfwTerminate();


	//krender.cleanUp();

	return 0;
}