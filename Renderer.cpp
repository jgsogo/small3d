#include "Renderer.h"
#include "Exception.h"
#include <fstream>
#include <unordered_map>
#include "MathFunctions.h"
#include <dimitrikourk/glm/glm/glm.hpp>
#include <dimitrikourk/glm/glm/gtc/type_ptr.hpp>
#include <miguel/sdl2/include/SDL.h>
#include <cstring>

using namespace std;

namespace small3d
{
    string openglErrorToString(GLenum error);

	string Renderer::loadShaderFromFile(const string &fileLocation)
	{
		initLogger();
		string shaderSource = "";
		ifstream file((SDL_GetBasePath() + fileLocation).c_str());
		string line;
		if (file.is_open())
		{
			while (getline(file, line))
			{
				shaderSource += line + "\n";
			}
		}
		return shaderSource;
	}

	GLuint Renderer::compileShader(const string &shaderSourceFile, const GLenum shaderType)
	{

		GLuint shader = glCreateShader(shaderType);

		string shaderSource = this->loadShaderFromFile(shaderSourceFile);

		const char *shaderSourceChars = shaderSource.c_str();
		glShaderSource(shader, 1, &shaderSourceChars, NULL);

		glCompileShader(shader);

		GLint status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{

			throw Exception(
				"Failed to compile shader:\n" + shaderSource + "\nInfo: "
				+ this->getProgramInfoLog(program));
		}
		else
		{
			LOGINFO("Shader " + shaderSourceFile + " compiled successfully.");
		}

		return shader;
	}

	Renderer::Renderer()
	{
		isOpenGL33Supported = false;
		sdlWindow = 0;
		program = 0;
		textProgram = 0;
		textures = new unordered_map<string, GLuint>();
		font = NULL;
		noShaders = false;
		
	}

	Renderer::~Renderer()
	{

		LOGINFO("Renderer destructor running");
		for (unordered_map<string, GLuint>::iterator it = textures->begin();
			it != textures->end(); ++it)
		{
			LOGINFO("Deleting texture for " + it->first);
			glDeleteTextures(1, &it->second);
		}
		delete textures;

		if (!noShaders)
		{
			glUseProgram(0);
		}

		if(textProgram != 0)
		{
			glDeleteProgram(textProgram);
		}

		if(program != 0)
		{
			glDeleteProgram(program);
		}

		TTF_CloseFont(font);
		TTF_Quit();

		if (sdlWindow != 0)
		{
			SDL_DestroyWindow(sdlWindow);
		}
		SDL_Quit();
	}

	void Renderer::initSDL(int width, int height, bool fullScreen)
	{
		sdlWindow = 0;

		// initialize SDL video
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
		{
			LOGERROR(SDL_GetError());
			throw Exception(string("Unable to initialise SDL"));
		}
#ifdef __APPLE__
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
#endif
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		Uint32 flags = fullScreen ? SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP :
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

		sdlWindow = SDL_CreateWindow("Avoid the Bug 3D", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, width, height,
			flags);

		if (SDL_GL_CreateContext( sdlWindow ) == NULL)
		{
			LOGERROR(SDL_GetError());
			throw Exception(string("Unable to create GL context"));
		}

		if (!sdlWindow)
		{
			LOGERROR(SDL_GetError());
			throw Exception("Unable to set video");
		}

		if(TTF_Init()==-1)
		{
			LOGERROR(TTF_GetError());
			throw Exception("Unable to initialise font system");
		}

		string fontPath = SDL_GetBasePath() + ttfFontPath;
		LOGINFO("Loading font from " + fontPath);
		
		font = TTF_OpenFont(fontPath.c_str(), 48);

		if (!font)
		{
			LOGERROR(TTF_GetError());
			throw Exception("Failed to load font");
		}
		else
		{
			LOGINFO("TTF font loaded successfully");
		}

	}

	string Renderer::getProgramInfoLog(const GLuint linkedProgram) const
	{

		GLint infoLogLength;
		glGetProgramiv(linkedProgram, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar *infoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(linkedProgram, infoLogLength, NULL, infoLog);
		string infoLogStr = infoLog;

		delete[] infoLog;

		return infoLogStr;

	}

	void Renderer::detectOpenGLVersion()
	{
#ifdef __APPLE__
        glewExperimental = GL_TRUE;
#endif
		int initResult = glewInit();
		
		if (initResult != GLEW_OK)
		{
			throw Exception("Error initialising GLEW");
		}
		else
		{
			string glewVersion = (char*) glewGetString(GLEW_VERSION);
			LOGINFO("Using GLEW version " + glewVersion);
		}
        
        checkForOpenGLErrors("initialising GLEW", false);

		string glVersion = (char*) glGetString(GL_VERSION);
		glVersion = "OpenGL version supported by machine: " + glVersion;
		LOGINFO(glVersion);

		if (glewIsSupported("GL_VERSION_3_3"))
		{
			LOGINFO("Ready for OpenGL 3.3");
			isOpenGL33Supported = true;
		}
		else if (glewIsSupported("GL_VERSION_2_1"))
		{
			LOGINFO("Ready for OpenGL 2.1");
		}
		else
		{
			noShaders = true;
			throw Exception(
				"None of the supported OpenGL versions (3.3 nor 2.1) are available.");
		}

	}
    
    void Renderer::checkForOpenGLErrors(string when, bool abort) {
        GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            LOGERROR("OpenGL error while " + when);
            
            do
            {
                LOGERROR(openglErrorToString(errorCode));
                errorCode = glGetError();
            }
            while(errorCode != GL_NO_ERROR);
            
            if (abort)
                throw Exception("OpenGL error while " + when);
        }
    }

	void Renderer::init(const int width, const int height, const bool fullScreen, 
		const string ttfFontPath, const string shadersPath)
	{
		this->ttfFontPath = ttfFontPath;

		this->initSDL(width, height, fullScreen);

		this->detectOpenGLVersion();

		string vertexShaderPath;
		string fragmentShaderPath;
		string textVertexShaderPath;
		string textFragmentShaderPath;

		if (isOpenGL33Supported)
		{
			vertexShaderPath = shadersPath + "OpenGL33/perspectiveMatrixLightedShader.vert";
			fragmentShaderPath = shadersPath + "OpenGL33/textureShader.frag";
			textVertexShaderPath =
				shadersPath + "OpenGL33/textShader.vert";
			textFragmentShaderPath = shadersPath + "OpenGL33/textShader.frag";

		}
		else
		{
			vertexShaderPath =
				shadersPath + "OpenGL21/perspectiveMatrixLightedShader.vert";
			fragmentShaderPath = shadersPath + "OpenGL21/textureShader.frag";
			textVertexShaderPath =
				shadersPath + "OpenGL21/textShader.vert";
			textFragmentShaderPath = shadersPath + "OpenGL21/textShader.frag";
		}

		glViewport(0, 0, (GLsizei) width, (GLsizei) height);

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
		glDepthRange(0.0f, 10.0f);

		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		GLuint vertexShader = compileShader(vertexShaderPath,
			GL_VERTEX_SHADER);
		GLuint fragmentShader = compileShader(fragmentShaderPath,
			GL_FRAGMENT_SHADER);

		program = glCreateProgram();
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);

		glLinkProgram(program);

		GLint status;
		glGetProgramiv(program, GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			throw Exception("Failed to link program:\n" + this->getProgramInfoLog(program));
		}
		else
		{
			LOGINFO("Linked main rendering program successfully");

			glUseProgram(program);

			// Perspective

			GLuint perspectiveMatrixUniform = glGetUniformLocation(program,
				"perspectiveMatrix");

			float perspectiveMatrix[16];
			memset(perspectiveMatrix, 0, sizeof(float) * 16);
			perspectiveMatrix[0] = 1.0f; // frustum scale
			perspectiveMatrix[5] = 1.8f; // frustum scale
			perspectiveMatrix[10] = (1.0f + 25.0f) / (1.0f - 25.0f); // (zNear + zFar) / (zNear - zFar)
			perspectiveMatrix[14] = 2.0f * 1.0f * 25.0f / (1.0f - 25.0f); // 2 * zNear * zFar / (zNear - zFar);
			perspectiveMatrix[11] = -1.0f; //cameraPos.z? or just the -1 factor...

			glUniformMatrix4fv(perspectiveMatrixUniform, 1, GL_FALSE,
				perspectiveMatrix);

			glUseProgram(0);
		}
		glDetachShader(program, vertexShader);
		glDetachShader(program, fragmentShader);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
		glClearDepth(10.0f);

		// Program (with shaders) for rendering text

		GLuint textVertexShader = compileShader(textVertexShaderPath,
			GL_VERTEX_SHADER);
		GLuint textFragmentShader = compileShader(textFragmentShaderPath,
			GL_FRAGMENT_SHADER);

		textProgram = glCreateProgram();
		glAttachShader(textProgram, textVertexShader);
		glAttachShader(textProgram, textFragmentShader);

		glLinkProgram(textProgram);

		glGetProgramiv(textProgram, GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			throw Exception("Failed to link program:\n" + this->getProgramInfoLog(textProgram));
		}
		else
		{
			LOGINFO("Linked text rendering program successfully");
		}
		glUseProgram(0);
	}

	GLuint Renderer::generateTexture(const string &name, const float *texture, const int width, const int height )
	{
		GLuint textureHandle;

		glGenTextures(1, &textureHandle);

		glBindTexture(GL_TEXTURE_2D, textureHandle);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		GLint internalFormat = isOpenGL33Supported ? GL_RGBA32F : GL_RGBA;

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA,
			GL_FLOAT, &texture[0]);

		textures->insert(make_pair(name, textureHandle));

		return textureHandle;
	}


	GLuint Renderer::getTextureHandle(const string &name)
	{
		GLuint handle = 0;

		unordered_map<string, GLuint>::iterator nameTexturePair = textures->find(name);

		if(nameTexturePair != textures->end())
		{
			handle = nameTexturePair->second;
		}

		return handle;
	}


	void Renderer::renderTexturedQuad(const float *vertices, const string &textureName)
	{
        float triangleVerts[24] =
        {
            vertices[0], vertices[1], vertices[2], 1.0f,
            vertices[4], vertices[5], vertices[6], 1.0f,
            vertices[12], vertices[13], vertices[14], 1.0f,
            vertices[12], vertices[13], vertices[14], 1.0f,
            vertices[4], vertices[5], vertices[6], 1.0f,
            vertices[8], vertices[9], vertices[10], 1.0f
        };
        
        
        GLuint textureHandle = getTextureHandle(textureName);
        
        if (textureHandle == 0)
        {
            throw Exception("Texture " + textureName + "has not been generated");
        }
        
        GLuint vao = 0;
        if (isOpenGL33Supported)
        {
            // Generate VAO
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
        }
        
        glBindTexture(GL_TEXTURE_2D, textureHandle);
        
        glUseProgram(textProgram);
        
        glEnableVertexAttribArray(0);
        
        GLuint boxBuffer = 0;
        glGenBuffers(1, &boxBuffer);
        
        glBindBuffer(GL_ARRAY_BUFFER, boxBuffer);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(float) * 24,
                     triangleVerts,
                     GL_STATIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
        
        float textureCoords[12] =
        {
            1.0f, 0.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            0.0f, 1.0f
        };
        
        GLuint coordBuffer = 0;
        
        glGenBuffers(1,&coordBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, coordBuffer);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(float) * 12,
                     &textureCoords,
                     GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glDeleteBuffers(1, &boxBuffer);
        glDeleteBuffers(1, &coordBuffer);
        
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        if (isOpenGL33Supported)
        {
            glDeleteVertexArrays(1, &vao);
            glBindVertexArray(0);
        }
        
        checkForOpenGLErrors("rendering textured quad", true);
	}

	void Renderer::clearScreen()
	{
		// Clear the buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Renderer::renderSceneObjects(const shared_ptr<vector<shared_ptr<SceneObject> > > sceneObjects)
	{

		// Use the shaders prepared at initialisation
		glUseProgram(program);

		// Pick up each model in the scene and render it.
		for (std::vector<shared_ptr<SceneObject> >::iterator it =
			sceneObjects->begin(); it != sceneObjects->end(); it++)
		{
			GLuint vao = 0;
			if (isOpenGL33Supported)
			{
				// Generate VAO
				glGenVertexArrays(1, &vao);
				glBindVertexArray(vao);
			}

			GLuint positionBufferObject = 0;
			GLuint indexBufferObject = 0;
			GLuint normalsBufferObject = 0;
			GLuint sampler = 0;
			GLuint texture = 0;
			GLuint uvBufferObject = 0;

			// Pass the vertex positions to the shaders
			glGenBuffers(1, &positionBufferObject);

			glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
			glBufferData(GL_ARRAY_BUFFER,
				it->get()->getModel().getVertexDataSize(),
				it->get()->getModel().getVertexData(),
				GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

			// Pass vertex indexes

			glGenBuffers(1, &indexBufferObject);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				it->get()->getModel().getIndexDataSize(),
				it->get()->getModel().getIndexData(),
				GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
			
			// Normals
			glGenBuffers(1, &normalsBufferObject);
			glBindBuffer(GL_ARRAY_BUFFER, normalsBufferObject);
			glBufferData(GL_ARRAY_BUFFER,
				it->get()->getModel().getNormalsDataSize(),
				it->get()->getModel().getNormalsData(), GL_STATIC_DRAW);
            glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			// Find the colour uniform
			GLuint colourUniform = glGetUniformLocation(program, "colour");

			// Add texture if that is contained in the model
			shared_ptr<Image> textureObj = it->get()->getTexture();

			if (textureObj)
			{
				// "Disable" colour since there is a texture
				float cl[4] = {0.0f, 0.0f, 0.0f, 0.0f};

				glUniform4fv(colourUniform, 1, cl);

				texture = this->getTextureHandle(it->get()->getName());

				if (texture == 0)
				{
					texture = generateTexture(it->get()->getName(), textureObj->getData(), textureObj->getWidth(), textureObj->getHeight());
				}

				glBindTexture(GL_TEXTURE_2D, texture);

				// UV Coordinates

				glGenBuffers(1, &uvBufferObject);
				glBindBuffer(GL_ARRAY_BUFFER, uvBufferObject);
				glBufferData(GL_ARRAY_BUFFER,
					it->get()->getModel().getTextureCoordsDataSize(),
					it->get()->getModel().getTextureCoordsData(), GL_STATIC_DRAW);
				glEnableVertexAttribArray(2);
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);

			}
			else
			{
				// If there is no texture, use the colour of the object
				glUniform4fv(colourUniform, 1, glm::value_ptr(*it->get()->getColour()));
			}
            
            // Lighting
            
            glm::vec3 lightDirection(0.0f, 0.9f, 0.2f);
            GLuint lightDirectionUniform = glGetUniformLocation(program,
                                                                "lightDirection");
            glUniform3fv(lightDirectionUniform, 1,
                         glm::value_ptr(lightDirection));

			// Rotation

			GLuint xRotationMatrixUniform = glGetUniformLocation(program,
				"xRotationMatrix");
			GLuint yRotationMatrixUniform = glGetUniformLocation(program,
				"yRotationMatrix");
			GLuint zRotationMatrixUniform = glGetUniformLocation(program,
				"zRotationMatrix");

			glUniformMatrix4fv(xRotationMatrixUniform, 1, GL_TRUE, glm::value_ptr(rotateX(it->get()->getRotation()->x)));
			glUniformMatrix4fv(yRotationMatrixUniform, 1, GL_TRUE, glm::value_ptr(rotateY(it->get()->getRotation()->y)));
			glUniformMatrix4fv(zRotationMatrixUniform, 1, GL_TRUE, glm::value_ptr(rotateZ(it->get()->getRotation()->z)));

			GLuint offsetUniform = glGetUniformLocation(program, "offset");
			glUniform3fv(offsetUniform, 1, glm::value_ptr(*it->get()->getOffset()));

			// Throw an exception if there was an error in OpenGL, during
			// any of the above.
            checkForOpenGLErrors("rendering scene", true);

			// Draw
			glDrawElements(GL_TRIANGLES,
				it->get()->getModel().getIndexDataIndexCount(),
				GL_UNSIGNED_INT, 0);

			// Clear stuff
			if (textureObj)
			{
				glDisableVertexAttribArray(2);
			}

			if (positionBufferObject != 0)
			{
				glDeleteBuffers(1, &positionBufferObject);
			}

			if ( indexBufferObject != 0)
			{
				glDeleteBuffers(1, &indexBufferObject);
			}
			if ( normalsBufferObject != 0)
			{
				glDeleteBuffers(1, &normalsBufferObject);
			}
			if ( sampler != 0)
			{
				glDeleteSamplers(1, &sampler);
			}
			if ( uvBufferObject != 0)
			{
				glDeleteBuffers(1, &uvBufferObject);
			}

			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(0);

			if (isOpenGL33Supported)
			{
				glDeleteVertexArrays(1, &vao);
				glBindVertexArray(0);
			}

		} // for std::vector<SceneObject>::iterator

		glUseProgram(0);

	}

	void Renderer::renderText(const string &text, const SDL_Color &colour, 
		const float &topX, const float &topY, const float &bottomX, const float &bottomY)
	{

		GLuint textHandle = getTextureHandle("text_" + text);

		if (textHandle == 0)
		{

			SDL_Surface *textSurface = TTF_RenderText_Blended(font,
				text.c_str(), colour);
			int numPixels = textSurface->h * textSurface->w;

			Uint32 *pix = static_cast<Uint32*>(textSurface->pixels);

			float *texturef = new float[numPixels * 4];

			for (int pidx = 0; pidx < numPixels; ++pidx)
			{
				Uint32 r = pix[pidx] & textSurface->format->Rmask;
				Uint32 g = pix[pidx] & textSurface->format->Gmask;
				Uint32 b = pix[pidx] & textSurface->format->Bmask;
				Uint32 a = pix[pidx] & textSurface->format->Amask;

				r = r >> textSurface->format->Rshift;
				g = g >> textSurface->format->Gshift;
				b = b >> textSurface->format->Bshift;
				a = a >> textSurface->format->Ashift;

				float ttuple[4] = {(float)r, // used to be boost::numeric_cast<float, Uint32>
					(float)g,
					(float)b,
					(float)a
				};

				ttuple[0]= floorf(100.0f * (ttuple[0] / 255.0f) + 0.5f) / 100.0f;
				ttuple[1]= floorf(100.0f * (ttuple[1] / 255.0f) + 0.5f) / 100.0f;
				ttuple[2]= floorf(100.0f * (ttuple[2] / 255.0f) + 0.5f) / 100.0f;
				ttuple[3]= floorf(100.0f * (ttuple[3] / 255.0f) + 0.5f) / 100.0f;

				memcpy(&texturef[pidx * 4], &ttuple, sizeof(ttuple));

			}

			textHandle = generateTexture("text_" + text, texturef, textSurface->w, textSurface->h);

			delete[] texturef;
			texturef = NULL;
			SDL_FreeSurface(textSurface);

		}

		float boxVerts[16] =
		{
			bottomX, topY, -0.5f, 1.0f,
			topX, topY, -0.5f, 1.0f,
			topX, bottomY, -0.5f, 1.0f,
			bottomX, bottomY, -0.5f, 1.0f
		};

		this->renderTexturedQuad(boxVerts, "text_" + text);
	}

	void Renderer::swapBuffers()
	{
		SDL_GL_SwapWindow(sdlWindow);
	}
    
    string openglErrorToString(GLenum error)
    {
        string errorString;
        
        switch(error) {
            case GL_NO_ERROR:
                errorString="GL_NO_ERROR: No error has been recorded. The value of this symbolic constant is guaranteed to be 0.";
                break;
            case  GL_INVALID_ENUM:
                errorString="GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.";
                break;
            case  GL_INVALID_VALUE:
                errorString="GL_INVALID_VALUE: A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.";
                break;
            case  GL_INVALID_OPERATION:
                errorString="GL_INVALID_OPERATION: The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.";
                break;
            case  GL_INVALID_FRAMEBUFFER_OPERATION:
                errorString="GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.";
                break;
            case  GL_OUT_OF_MEMORY:
                errorString="GL_OUT_OF_MEMORY: There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
                break;
            case  GL_STACK_UNDERFLOW:
                errorString="GL_STACK_UNDERFLOW: An attempt has been made to perform an operation that would cause an internal stack to underflow.";
                break;
            case   GL_STACK_OVERFLOW:
                errorString="GL_STACK_OVERFLOW: An attempt has been made to perform an operation that would cause an internal stack to overflow.";
                break;
            default:
                errorString="Unknown error";
                break;
        }
        return errorString;
    }

}

