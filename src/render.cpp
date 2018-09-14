#include "render.h"

gRender::~gRender()
{
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
	m_texLevelID = glGetUniformLocation(renderProg.getHandle(), "texLevel");

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
	m_fromDistanceID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromDistance");
	m_fromQuadtreeID = glGetSubroutineIndex(renderProg.getHandle(), GL_FRAGMENT_SHADER, "fromQuadtree");




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

}


void gRender::setComputeWindowPosition()
{
	glViewport(0, 0, 1920, 1080);
}

void gRender::setRenderingOptions(bool showDepthFlag, bool showBigDepthFlag, bool showInfraFlag, bool showColorFlag, bool showLightFlag, bool showPointFlag, bool showFlowFlag, bool showEdgesFlag, bool showNormalFlag, bool showVolumeSDFFlag, bool showTrackFlag, bool showDistance)
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
	m_showDistanceFlag = showDistance;
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

	if (m_showDistanceFlag)
	{
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, m_textureDistance);
	}



}




void gRender::Render(bool useInfrared)
{
	// set positions
	// set uniforms
	// render textures
	bindTexturesForRendering();
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

		//glTexParameteri(m_textureColor, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		//glTexParameteri(m_textureColor, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glBindVertexArray(m_VAO);

		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromStandardTextureID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromColorID);
		//glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));
		glUniform1i(m_texLevelID, m_texLevel);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 
		  
		//glTexParameteri(m_textureColor, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		//glTexParameteri(m_textureColor, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
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
		glUniform1i(m_texLevelID, m_texLevel);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	if (m_showDistanceFlag)
	{
		glm::vec2 imageSize;

		imageSize = glm::vec2(m_color_width, m_color_height);
		MVP = m_projection * m_view * m_model_color;

		glBindVertexArray(m_VAO);
		MVP = glm::translate(MVP, glm::vec3(0.0f, 0.0f, 5.0f));
		glUniformSubroutinesuiv(GL_VERTEX_SHADER, 1, &m_fromStandardTextureID);
		glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_fromDistanceID);
		//glUniformMatrix4fv(m_ProjectionID, 1, GL_FALSE, glm::value_ptr(m_projection));
		glUniformMatrix4fv(m_MvpID, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniform2fv(m_imSizeID, 1, glm::value_ptr(imageSize));
		glUniform1i(m_texLevelID, m_texLevel);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


	}






}


