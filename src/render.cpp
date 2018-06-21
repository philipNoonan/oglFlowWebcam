#include "render.h"

gRender::~gRender()
{
}

void gRender::GLFWCallbackWrapper::MousePositionCallback(GLFWwindow* window, double positionX, double positionY)
{
	s_application->MousePositionCallback(window, positionX, positionY);
}

void gRender::GLFWCallbackWrapper::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	s_application->MouseButtonCallback(window, button, action, mods);
}


void gRender::GLFWCallbackWrapper::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	s_application->KeyboardCallback(window, key, scancode, action, mods);
}

void gRender::GLFWCallbackWrapper::SetApplication(gRender* application)
{
	GLFWCallbackWrapper::s_application = application;
}

gRender* gRender::GLFWCallbackWrapper::s_application = nullptr;

void gRender::MousePositionCallback(GLFWwindow* window, double positionX, double positionY)
{
	//...
	//std::cout << "mouser" << std::endl;
	m_mouse_pos_x = positionX;
	m_mouse_pos_y = positionY;
}
void gRender::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	float zDist = 1500.0f;
	float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...

																		// the height of the screen at the distance of the image is 2 * halfheight
																		// to go from the middle to the top 

																		//m_model_depth = glm::translate(glm::mat4(1.0f), glm::vec3(-halfWidthAtDistance, halfHeightAtDist - m_depth_height, -zDist));

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && m_selectInitialPoseFlag == true)
	{
		if (m_mouse_pos_x > 32 && m_mouse_pos_x < m_depth_width + 32 && m_mouse_pos_y < 32 + 424 && m_mouse_pos_y > 32)
		{
			m_center_pixX = m_mouse_pos_x - 32;
			m_center_pixY = m_mouse_pos_y - 32;

			//std::cout << "x: " << m_center_pixX  << " y: " << m_center_pixY << std::endl;

			// get depth value, from texture buffer or float array???

			//// need to get depth pixel of this point
			//float x = (pixX - m_cameraParams.z) * (1.0f / m_cameraParams.x) * depth.x;
			//float y = (pixY - m_cameraParams.w) * (1.0f / m_cameraParams.y) * depth.x;
			//float z = depth.x;

			//m_cameraParams.x
		}
	}


	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		// m_depthPixelPoints2D.push_back(std::make_pair(m_mouse_pos_x, m_mouse_pos_y));
		// get correct current offset and scakle for the window
		int depth_pos_x = m_mouse_pos_x / m_render_scale_width;
		int depth_pos_y = m_mouse_pos_y / m_render_scale_height;

		//std::cout <<" x: " << m_mouse_pos_x << " y: " << m_mouse_pos_y << " xS: " << m_render_scale_width << " yS: " << m_render_scale_height << std::endl;
		//std::cout << ((float)h / 424.0f) * m_mouse_pos_y << std::endl;


		if (depth_pos_x < m_depth_width && depth_pos_y < m_depth_height)
		{
			m_depthPixelPoints2D.push_back(std::make_pair(depth_pos_x, depth_pos_y));
			m_depthPointsFromBuffer.resize(m_depthPixelPoints2D.size() * 4); // for 4 floats per vertex (x,y,z, + padding)


		}
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		if (m_depthPixelPoints2D.size() > 0)
		{
			m_depthPixelPoints2D.pop_back();
			m_depthPointsFromBuffer.resize(m_depthPixelPoints2D.size() * 4); // for 4 floats per vertex (x,y,z, + padding)

		}
		// pop_back entry on vector
	}

	if (m_depthPixelPoints2D.size() > 0 && action == GLFW_PRESS)
	{
		std::cout << m_depthPixelPoints2D.size();
		for (auto i : m_depthPixelPoints2D)
		{
			//std::cout << " x: " << i.first << " y: " << i.second << std::endl;
		}
	}
	else if (m_depthPixelPoints2D.size() == 0 && action == GLFW_PRESS)
	{
		std::cout << "no entries yet, left click points on depth image" << std::endl;
	}
	//std::cout << "mouse button pressed: " << button << " " << action << " x: " <<  m_mouse_pos_x << " y: " << m_mouse_pos_y << std::endl;

}

void gRender::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//...
	//std::cout << "keyer" << std::endl;


	if (key == GLFW_KEY_H && action == GLFW_PRESS)
		m_show_imgui = !m_show_imgui;
}

void gRender::SetCallbackFunctions()
{
	GLFWCallbackWrapper::SetApplication(this);
	glfwSetCursorPosCallback(m_window, GLFWCallbackWrapper::MousePositionCallback);
	glfwSetKeyCallback(m_window, GLFWCallbackWrapper::KeyboardCallback);
	glfwSetMouseButtonCallback(m_window, GLFWCallbackWrapper::MouseButtonCallback);
}

GLFWwindow * gRender::loadGLFWWindow()
{

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_REFRESH_RATE, 30);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	m_window = glfwCreateWindow(m_screen_width, m_screen_height, "oglflow", nullptr, nullptr);

	if (m_window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		//return -1;
	}

	glfwMakeContextCurrent(m_window);
	//glfwSwapInterval(1); // Enable vsync
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK) 
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		//return -1;
	}


	return m_window;
}

void gRender::requestShaderInfo()
{
	renderProg.printActiveUniforms();
}

void gRender::compileAndLinkShader()
{
	try {
		renderProg.compileShader("shaders/vertShader.vs");
		renderProg.compileShader("shaders/fragShader.fs");
		renderProg.link();


	}
	catch (GLSLProgramException &e) {
		std::cerr << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
}

void gRender::setLocations()
{
	m_ProjectionID = glGetUniformLocation(renderProg.getHandle(), "projection");
	m_MvpID = glGetUniformLocation(renderProg.getHandle(), "MVP");
	m_ModelID = glGetUniformLocation(renderProg.getHandle(), "model");
	m_ViewProjectionID = glGetUniformLocation(renderProg.getHandle(), "ViewProjection");
	m_sliceID = glGetUniformLocation(renderProg.getHandle(), "slice");
	m_imSizeID = glGetUniformLocation(renderProg.getHandle(), "imSize");

	m_getPositionSubroutineID = glGetSubroutineUniformLocation(renderProg.getHandle(), GL_VERTEX_SHADER, "getPositionSubroutine");
	m_fromTextureID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromTexture");
	m_fromPosition4DID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromPosition4D");
	m_fromPosition2DID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromPosition2D");
	m_fromPosePoints2DID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromPosePoints2D");
	m_fromFacePoints2DID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromFacePoints2D");
	m_fromHandsPoints2DID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromHandsPoints2D");
	m_fromStandardTextureID = glGetSubroutineIndex(renderProg.getHandle(), GL_VERTEX_SHADER, "fromStandardTexture");

	m_colorSelectionRoutineID = glGetSubroutineUniformLocation(renderProg.getHandle(), GL_FRAGMENT_SHADER, "getColorSelection");
	m_fromDepthID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromDepth");
	m_fromColorID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromColor");
	m_fromRayNormID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromRayNorm");
	m_fromRayVertID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromRayVert");
	m_fromPointsID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromPoints");
	m_fromVolumeID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromVolume");
	m_fromTrackID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromTrack");
	m_fromFlowID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromFlow");
	m_fromEdgesID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromEdges");




	//m_ambientID = glGetUniformLocation(renderProg.getHandle(), "ambient");
	//m_lightID = glGetUniformLocation(renderProg.getHandle(), "light");

	//m_irLowID = glGetUniformLocation(renderProg.getHandle(), "irLow");
	//m_irHighID = glGetUniformLocation(renderProg.getHandle(), "irHigh");

}

void gRender::updateVerts(float w, float h)
{
	std::vector<float> vertices = {
		// Positions		// Texture coords
		w / 2.0f, h / 2.0f, 0.0f, 1.0f, 1.0f, // top right
		w / 2.0f, -h / 2.0f, 0.0f, 1.0f, 0.0f, // bottom right
		-w / 2.0f, -h / 2.0f, 0.0f, 0.0f, 0.0f, // bottom left
		-w / 2.0f, h / 2.0f, 0.0f, 0.0f, 1.0f  // Top left
	};

	m_standard_verts = vertices;

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Standard);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_standard_verts.size() * sizeof(float), m_standard_verts.data());

}

void gRender::setVertPositions()
{
	std::vector<float> vertices = {
		// Positions				// Texture coords
		1.0f,	1.0f,	0.0f,		1.0f, 1.0f, // top right
		1.0f,	-1.0f,	0.0f,		1.0f, 0.0f, // bottom right
		-1.0f,	-1.0f,	0.0f,		0.0f, 0.0f, // bottom left
		-1.0f,	1.0f,	0.0f,		0.0f, 1.0f  // Top left
	};

	m_standard_verts = vertices;

	m_vertices = vertices;

	std::vector<unsigned int>  indices = {  // Note that we start from 0!
		0, 1, 3, // First Triangle
		1, 2, 3  // Second Triangle
	};

	m_indices = indices;

	m_trackedPoints.resize(1000 * 1000 * 2);

	for (int i = 0; i < 2000; i += 2)
	{
		for (int j = 0; j < 1000; j++)
		{
			m_trackedPoints[j * 2000 + i] = (1920 >> 1) - 500 + (i / 2) * 10;
			m_trackedPoints[j * 2000 + i + 1] = (1080 >> 1) - 500 + j * 10;

		}
	}

	m_facePoints.resize(70 * 3, 0);
	m_posePoints.resize(18 * 3, 0); // for coco 18
	m_handsPoints.resize(21 * 3 * 2, 0);// 21 for each hand

	std::vector<unsigned int> idxPose = {
		0, 1, // neck
		1, 2, // right shoulder
		2, 3, // right top arm
		3, 4, // right forearm
		1, 5, // left shoulder
		5, 6, // left top arm
		6, 7, // left forearm
		1, 8, // right torso
		8, 9, // right thigh
		9, 10, // right shin
		1, 11, // left torso
		11, 12, // left thigh
		12, 13, // left shin
		16, 14, // right ear2eye
		14, 0, // right eye2nose
		0, 15, // left nose2eye
		15, 17 // left eye2ear
	};

	m_idxPose = idxPose;

	// ibug 300W 68 + 2 iris == 70 points
	std::vector<unsigned int> idxFace = {
		// jaw
		0, 1,
		1, 2,
		2, 3,
		3, 4,
		4, 5,
		5, 6,
		6, 7,
		7, 8,
		8, 9,
		9, 10,
		10, 11,
		11, 12,
		12, 13,
		13, 14,
		14, 15,
		15, 16,
		// right eyebrow
		17, 18,
		18, 19,
		19, 20,
		20, 21,
		// left eyebrow
		22, 23,
		23, 24, 
		24, 25, 
		25, 26,
		// nose
		27, 28,
		28, 29,
		29, 30,
		30, 31,
		31, 32,
		32, 33,
		33, 34,
		34, 35,
		35, 30,
		// right eye
		36, 37,
		37, 38,
		38, 39,
		39, 40,
		40, 41,
		41, 36,
		// left eye
		42, 43, 
		43, 44,
		44, 45,
		45, 46,
		46, 47,
		47, 42,
		// mouth outer
		48, 49, 
		49, 50,
		50, 51,
		51, 52,
		52, 53,
		53, 54,
		54, 55,
		55, 56,
		56, 57,
		57, 58,
		58, 59,
		59, 48,
		// mouth inner
		60, 61,
		61, 62,
		62, 63,
		63, 64,
		64, 65,
		65, 66,
		66, 67,
		67, 60
	};
	m_idxFace = idxFace;

	std::vector<unsigned int> idxHands = {
		// left thumb
		0, 1,
		1, 2,
		2, 3,
		3, 4,
		// left index finger
		0, 5,
		5, 6,
		6, 7,
		7, 8,
		// left middle finger
		0, 9,
		9, 10,
		10, 11,
		11, 12,
		// left ring finger
		0, 13,
		13, 14,
		14, 15,
		15, 16,
		// left pinky
		0, 17,
		17, 18,
		18, 19,
		19, 20,
		// right thumb
		21, 22,
		22, 23,
		23, 24,
		24, 25,
		// right index finger
		21, 26,
		26, 27,
		27, 28,
		28, 29,
		// right middle finger
		21, 30,
		30, 31,
		31, 32,
		32, 33,
		// right ring finger
		21, 34,
		34, 35,
		35, 36,
		36, 37,
		// right pinky
		21, 38,
		38, 39,
		39, 40,
		40, 41
	};
	m_idxHands = idxHands;

	m_indices_pose = idxFace;
}

void gRender::allocateBuffers()
{
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO_Standard);
	glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);

	// standard verts
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Standard);
	glBufferData(GL_ARRAY_BUFFER, m_standard_verts.size() * sizeof(float), &m_standard_verts[0], GL_DYNAMIC_DRAW);
	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_DYNAMIC_DRAW);
	// Position attribute for Depth
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(4);
	//// TexCoord attribute for Depth
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	glEnableVertexAttribArray(5);



	glBindVertexArray(0);

	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_posBufMC);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, 128*128*128*4 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &m_VAO_MC);
	glGenBuffers(1, &m_bufferTrackedPoints);
	glGenBuffers(1, &m_bufferFacePoints);
	glGenBuffers(1, &m_bufferPosePoints);
	glGenBuffers(1, &m_bufferHandsPoints);

	glGenBuffers(1, &m_EBO_Pose);


	glBindVertexArray(m_VAO_MC);

	glBindBuffer(GL_ARRAY_BUFFER, m_bufferTrackedPoints);
	glBufferData(GL_ARRAY_BUFFER, 100 * 100 * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, 0, 0); // 2  floats per vertex, x,y
	glEnableVertexAttribArray(7);

	glBindBuffer(GL_ARRAY_BUFFER, m_bufferFacePoints);
	glBufferData(GL_ARRAY_BUFFER, 70 * 3 * sizeof(float), m_facePoints.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, 0, 0); // 3  floats per vertex, x,y, weight
	glEnableVertexAttribArray(8);

	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_Pose);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices_pose.size() * sizeof(unsigned int), &m_indices_pose[0], GL_DYNAMIC_DRAW); // so long as indicies pose is the big one?

	glBindBuffer(GL_ARRAY_BUFFER, m_bufferPosePoints);
	glBufferData(GL_ARRAY_BUFFER, 18 * 3 * sizeof(float), m_posePoints.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, 0, 0); // 3  floats per vertex, x,y, weight
	glEnableVertexAttribArray(9);

	glBindBuffer(GL_ARRAY_BUFFER, m_bufferHandsPoints);
	glBufferData(GL_ARRAY_BUFFER, 21 * 3 * 2 * sizeof(float), m_handsPoints.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(10, 3, GL_FLOAT, GL_FALSE, 0, 0); // 3  floats per vertex, x,y, weight
	glEnableVertexAttribArray(10);

	glBindVertexArray(0);

	//glBindBuffer(GL_ARRAY_BUFFER, m_posBufMC);
	//glBufferData(GL_ARRAY_BUFFER, 128 * 128 * 128 * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	//glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 0, 0); // 4  floats per vertex, x,y,z and 1 for padding? this is annoying...
	//glEnableVertexAttribArray(6);

	//glBindVertexArray(0);


	//glGenBuffers(1, &m_bufferTrackedPoints);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_bufferTrackedPoints);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, m_bufferTrackedPoints);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, m_trackedPoints.size() * sizeof(float), m_trackedPoints.data(), GL_DYNAMIC_DRAW);


}

void gRender::setWindowLayout()
{
	m_anchorMW = std::make_pair<int, int>(50, 50);


}

void gRender::setComputeWindowPosition()
{
	glViewport(0, 0, 1920, 1080);
}

//
//void kRender::setColorDepthMapping(int* colorDepthMap)
//{
//	// 2d array index is given by
//	// p.x = idx / size.x
//	// p.y = idx % size.x
//
//	//for m_colorDepthMapping[j + 1] = y color image axis, 1 at top
//	// m_colorDepthMapping[j] = x axis, 0 on the left, 
//
//	// MAP ME¬¬¬
//	int j = 0;
//	for (int i = 0; i < (m_depth_width * m_depth_height); i++, j+=2)
//	{
//		int yCoord = colorDepthMap[i] / m_color_width;
//		int xCoord = colorDepthMap[i] % m_color_width;
//		m_colorDepthMapping[j] = ((float)xCoord) / (float)m_color_width;
//		m_colorDepthMapping[j + 1] = (1080.0f - (float)yCoord) / (float)m_color_height;
//
//
//
//	}
//
//
//
//	glBindBuffer(GL_ARRAY_BUFFER, m_buf_color_depth_map);
//	glBufferSubData(GL_ARRAY_BUFFER, 0, m_colorDepthMapping.size() * sizeof(float), m_colorDepthMapping.data());
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//	//// Other way to copy data to buffer, taken from https://learnopengl.com/#!Advanced-OpenGL/Advanced-Data
//	//glBindBuffer(GL_ARRAY_BUFFER, m_buf_color_depth_map);
//	//void *ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
//	//memcpy_s(ptr, m_colorDepthMapping.size() * sizeof(float), m_colorDepthMapping.data(), m_colorDepthMapping.size() * sizeof(float));
//	//glUnmapBuffer(GL_ARRAY_BUFFER);
//
//}


void gRender::setRenderingOptions(bool showDepthFlag, bool showBigDepthFlag, bool showInfraFlag, bool showColorFlag, bool showLightFlag, bool showPointFlag, bool showFlowFlag, bool showEdgesFlag, bool showNormalFlag, bool showVolumeSDFFlag, bool showTrackFlag)
{
	m_showDepthFlag = showDepthFlag;
	m_showBigDepthFlag = showBigDepthFlag;
	m_showInfraFlag = showInfraFlag;
	m_showColorFlag = showColorFlag;
	m_showLightFlag = showLightFlag;
	m_showPointFlag = showPointFlag;
	m_showFlowFlag = showFlowFlag;
	m_showEdgesFlag = showEdgesFlag;
	m_showNormalFlag = showNormalFlag;
	m_showVolumeSDFFlag = showVolumeSDFFlag;
	m_showTrackFlag = showTrackFlag;
}

void gRender::setTextures(GLuint colorTex, GLuint edgesTex)
{

	m_textureColor = colorTex;
	m_textureEdges = edgesTex;
	

}
void gRender::setFlowTexture(GLuint flowTex)
{
	m_textureFlow = flowTex;
}

void gRender::bindTexturesForRendering()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	if (m_showFlowFlag)
	{
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_textureFlow);


	}

	if (m_showColorFlag)
	{
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, m_textureColor);
	}


	if (m_showEdgesFlag)
	{
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, m_textureEdges);
	}



}

void gRender::bindBuffersForRendering()
{
	glBindVertexArray(m_VAO_MC);

	glBindBuffer(GL_ARRAY_BUFFER, m_bufferTrackedPoints);
	glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(7);





	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_posBufMC);
	//glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, 0); // 4  floats per vertex, x,y,z and 1 for padding? this is annoying...
	//glEnableVertexAttribArray(4);


}

void gRender::bindOpenPosePoints()
{
	// mask zero weighted joints
	m_idxFaceMask = m_idxFace;
	m_idxPoseMask = m_idxPose;
	m_idxHandsMask = m_idxHands;

	for (int i = 0; i < m_posePoints.size(); i += 3)
	{
		if (m_posePoints[i] <= 0.0f)
		{
			for (int j = 0; j < m_idxPose.size(); j++)
			{
				if (m_idxPose[j] == i / 3)
				{
					if (j % 2 == 0)
					{
						m_idxPoseMask[j] = 888;
						m_idxPoseMask[j + 1] = 888;
					}
					else
					{
						m_idxPoseMask[j] = 888;
						m_idxPoseMask[j - 1] = 888;
					}
				}
			}
		}
	}

	m_idxPoseMask.erase(std::remove(m_idxPoseMask.begin(), m_idxPoseMask.end(), 888), m_idxPoseMask.end());

	for (int i = 0; i < m_facePoints.size(); i += 3)
	{
		if (m_facePoints[i] <= 0.0f)
		{
			for (int j = 0; j < m_idxFace.size(); j++)
			{
				if (m_idxFace[j] == i / 3)
				{
					if (j % 2 == 0)
					{ 
						m_idxFaceMask[j] = 888;
						m_idxFaceMask[j + 1] = 888;
					}
					else  
					{
						m_idxFaceMask[j] = 888;
						m_idxFaceMask[j - 1] = 888;
					}
				}
			}
		}
	} 

	m_idxFaceMask.erase(std::remove(m_idxFaceMask.begin(), m_idxFaceMask.end(), 888), m_idxFaceMask.end());

	for (int i = 0; i < m_handsPoints.size(); i += 3)
	{
		//std::cout << "hands potins " << m_handsPoints[i] << " " << m_handsPoints[i + 1] << " " << m_handsPoints[i + 2] << std::endl;
	}
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferFacePoints);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_facePoints.size() * sizeof(float), m_facePoints.data());
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferFacePoints);
	glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(8);

	glBindBuffer(GL_ARRAY_BUFFER, m_bufferPosePoints);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_posePoints.size() * sizeof(float), m_posePoints.data());
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferPosePoints);
	glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(9);

	glBindBuffer(GL_ARRAY_BUFFER, m_bufferHandsPoints);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_handsPoints.size() * sizeof(float), m_handsPoints.data());
	glBindBuffer(GL_ARRAY_BUFFER, m_bufferHandsPoints);
	glVertexAttribPointer(10, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(10);


}


void gRender::Render(bool useInfrared)
{
	// set positions
	// set uniforms
	// render textures
	bindTexturesForRendering();
	bindBuffersForRendering();
	//setDepthImageRenderPosition();
	//setNormalImageRenderPosition();
	//setViewport(0, 0, 1920, 1080);

	renderLiveVideoWindow(useInfrared);



}



void gRender::setColorImageRenderPosition(float vertFov)
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	//// if setting z dist
	//float zDist = 8000.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	//// else if setting size on window
	float zDist;
	zDist = ((float)m_color_height * 1) / tan(vertFov * M_PI / 180.0f);
	//float halfHeightAtDist = (float)h * 4;
	//float halfWidthAtDistance = (float)w * 4;
	//m_model_color = glm::translate(glm::mat4(1.0f), glm::vec3(-m_color_width / 2.0f, -halfHeightAtDist, -zDist));
	//glm::vec3 scaleVec = glm::vec3(6.f, 6.f, 1.0f);

	//m_model_color = glm::scale(glm::mat4(1.0f), scaleVec);
	m_model_color = glm::translate(glm::mat4(1.0f), glm::vec3(-m_color_width / 2.0f, -m_color_height / 2.0f, -zDist));

	//std::cout << "zDis" << zDist << "w " << w << " h" << h << " ad " << halfWidthAtDistance << std::endl;
	//m_model_color = glm::translate(glm::mat4(1.0f), glm::vec3(-m_color_width / 2.0f, 0.0f, -2000.0f));
}


void gRender::setFlowImageRenderPosition(int height, int width, float vertFov) 
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	//// if setting z dist
	//float zDist = 8000.0f;
	//float halfHeightAtDist = zDist * tan(22.5f * M_PI / 180.0f);
	//float halfWidthAtDistance = halfHeightAtDist * (float)w / (float)h; // notsure why this ratio is used here...
	//// else if setting size on window
	float zDist;
	zDist = ((float)height * 1) / tan(vertFov * M_PI / 180.0f);
	//float halfHeightAtDist = (float)h * 4;
	//float halfWidthAtDistance = (float)w * 4;
	//m_model_color = glm::translate(glm::mat4(1.0f), glm::vec3(-m_color_width / 2.0f, -halfHeightAtDist, -zDist));
	glm::vec3 scaleVec = glm::vec3(1.f,1.f, 1.0f);

	//m_model_flow = glm::scale(glm::mat4(1.0f), scaleVec);
	m_model_flow = glm::translate(glm::mat4(1.0f), glm::vec3(-width / 2.0f, -height / 2.0f, -zDist));

	}


void gRender::setViewMatrix(float xRot, float yRot, float zRot, float xTran, float yTran, float zTran)
{
	glm::mat4 t0, t1, r0;
	m_view = glm::mat4(1.0f);

	t0 = glm::translate(glm::mat4(1.0), glm::vec3(xTran, yTran, zTran));
	t1 = glm::translate(glm::mat4(1.0), glm::vec3(-xTran, -yTran, -zTran));

	r0 = glm::eulerAngleXYZ(glm::radians(xRot), glm::radians(yRot), glm::radians(zRot));


	m_view = t1 * r0 * t0;
	//m_view = glm::translate(m_view, glm::vec3(0.0f, 0.0f, 0.0f));

}

void gRender::setProjectionMatrix()
{
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	m_projection = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 1.0f, 10000.0f); // scaling the texture to the current window size seems to work
	glViewport(0, 0, w, h);


}



void gRender::renderLiveVideoWindow(bool useInfrared)
{
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	renderProg.use();
	glm::mat4 MVP;


	if (m_showColorFlag)
	{
		glm::vec2 imageSize;

		imageSize = glm::vec2(m_color_width, m_color_height);
		MVP = m_projection * m_view * m_model_color;

		glBindVertexArray(m_VAO);

		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromStandardTextureID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromColorID);
		//glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 
		  

	}

	if (m_showEdgesFlag)
	{
		glm::vec2 imageSize;

		imageSize = glm::vec2(m_color_width, m_color_height);
		MVP = m_projection * m_view * m_model_color;

		glBindVertexArray(m_VAO);
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromStandardTextureID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromEdgesID);
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	if (m_showFlowFlag)
	{
		glm::vec2 imageSize;

		imageSize = glm::vec2(m_color_width, m_color_height);
		MVP = m_projection * m_view * m_model_color;

		glBindVertexArray(m_VAO);
		MVP = glm::translate(MVP, glm::vec3(0.0f, 0.0f, 5.0f));
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromStandardTextureID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromFlowID);
		//glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}



	if (m_showPointFlag)
	{
		glBindVertexArray(m_VAO_MC);

		glEnable(GL_PROGRAM_POINT_SIZE);

		MVP = m_projection * m_view * m_model_flow;
		//MVP = m_projection * m_view * m_model_color; 
		glm::vec2 imageSize;

		imageSize = glm::vec2(m_color_width, m_color_height);
		
		glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));

		MVP = glm::translate(MVP, glm::vec3(0.0f, 0.0f, 10.0f));
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromPosition2DID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromPointsID);
		//glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection)); 
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		//glDrawArrays(GL_TRIANGLES, 0, 3);
		glDrawArrays(GL_POINTS, 0, m_trackedPoints.size() / 2);



	} 


	
	glBindVertexArray(m_VAO_MC);
	glEnable(GL_PROGRAM_POINT_SIZE);
	//MVP = m_projection * m_view * m_model_flow;
	//glm::vec2 imageSize; 
	//imageSize = glm::vec2(m_color_width, m_color_height);
	//glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));  
	//MVP = glm::translate(MVP, glm::vec3(0.0f, 0.0f, 10.0f));
	//glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromFacePoints2DID);
	//glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromPointsID);
	//glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
	//glDrawArrays(GL_POINTS, 0, m_facePoints.size() / 3);


	MVP = m_projection * m_view * m_model_flow;
	glm::vec2 imageSize;
	imageSize = glm::vec2(m_color_width, m_color_height);
	MVP = glm::translate(MVP, glm::vec3(0.0f, 0.0f, 10.0f));

	if (m_idxFaceMask.size() > 0)
	{
		glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromFacePoints2DID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromPointsID);
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glLineWidth(3);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_Pose);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_idxFaceMask.size() * sizeof(unsigned int), m_idxFaceMask.data());

		glDrawElements(GL_LINES, m_idxFaceMask.size(), GL_UNSIGNED_INT, (void*)0);
		glDrawArrays(GL_POINTS, 68, 2);
	}


	if (m_idxPoseMask.size() > 0)
	{
		glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromPosePoints2DID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromPointsID);
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glLineWidth(5);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_Pose);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_idxPoseMask.size() * sizeof(unsigned int), m_idxPoseMask.data());

		glDrawElements(GL_LINES, m_idxPoseMask.size(), GL_UNSIGNED_INT, (void*)0);
	}

	if (m_idxHandsMask.size() > 0)
	{

		glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromHandsPoints2DID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromPointsID);
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glLineWidth(3);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_Pose);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_idxHandsMask.size() * sizeof(unsigned int), m_idxHandsMask.data());
		//glDrawArrays(GL_POINTS, 0, m_handsPoints.size() / 3);
		glDrawElements(GL_LINES, m_idxHandsMask.size(), GL_UNSIGNED_INT, (void*)0);

	}


	glBindVertexArray(0);

}


