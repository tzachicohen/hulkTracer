// bartLoader.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <tchar.h>
#include "glut.h"
#include "scene.hpp"
#include "AFFParser.hpp"
#include "OBJLoader.hpp"
#include "GLWindow.h"
#include "AFFTransformVisitor.hpp"
#include "Settings.hpp"
int _tmain(int argc, _TCHAR* argv[])
{
	glutInit(&argc, (char**)argv);

	bool retVal = Settings::getInstance().parseStrings(argc,argv);
	if (!retVal || argc <= 1) {
		 Settings::getInstance().printHelp();
		 return 0;
	}
	LoaderInterface * loader = NULL;
	char path[128];
	char * it1 = path;
	if (argc < 2) 
	{
		printf("missing file name\n");
		return 0;
	}
	_TCHAR * it = argv[1] ;
	while (  *it1++ = (char) *it++);
	std::string fileName = path;

	if ( fileName.find(".aff") != -1 )
	{
		loader = new AFFParser();
	}

	if ( fileName.find(".obj") != -1 )
	{
		loader = new OBJLoader();
	}
	std::string relPath = fileName.substr(0,fileName.rfind('\\')+1);
	FILE * pFile = fopen(fileName.c_str(),"r");
	if (pFile) 
	{
		std::string sceneName = fileName.substr(0,fileName.rfind('.'));
		sceneName = sceneName.substr( fileName.rfind('\\')+1,sceneName.size());
		Scene myScene(sceneName);
		bool result = loader->viParseFile(pFile,relPath,myScene);

		myScene.postLoadInitialize();
	
		GLWindow & window= GLWindow::getInstance();
		window.initGLWindow(myScene,*loader);
		window.mainLoop();
		window.deinitGLWindow();
	}
	else {
		printf ("no input file\n");
	}

	delete loader;
	return 0;
}

