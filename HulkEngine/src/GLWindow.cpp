
#include <stdlib.h>
#include <time.h>

#include "GLWindow.h"
#include "Settings.hpp"

#include "LoaderInterface.hpp"

#include "BoundingBoxVisitor.hpp"
#include "GLVisitor.hpp"
#include "GLVisitorT.hpp"
#include "GLVisitorCPUTrace.hpp"
#include "GLVisitorGPUTrace.hpp"
#include "GLVisitorGPUTraceSSD.hpp"
#include "Statistics.hpp"

GLWindow * GLWindow::s_instance(NULL);

GLWindow::GLWindow() 
:m_pScene(NULL),
 m_startTime(0),
 m_transform(0)
{
}

GLWindow::~GLWindow()
{

}

void MyExit(int window)
{
	if (0 == wcscmp(Settings::getInstance().getValue(L"--dumpStats"),L"1")) {
		Statistics::getInstance().dumpResults();
	}
	
	GLWindow & win = GLWindow::getInstance();	
	
	win.deinitGLWindow();

	exit(0);
}

void
GLWindow::mainLoop()
{
    // Setup callback functions
    glutDisplayFunc(display);
   // glutKeyboardFunc(::keyboard);
   // glutMouseFunc(::mouse);
   // glutMotionFunc(::motion);
    //glutReshapeFunc(resize);
    m_startTime = timeGetTime();
    // Start the glut main loop, never returns
    glutMainLoop();
}

void GLWindow::initGLWindow(Scene & scene,LoaderInterface & loader)
{
	m_pScene = &scene;
	m_pLoader = &loader;
	m_transform = loader.getTransformVisitor(scene);
	
	Statistics::getInstance().initStats(&scene);

	if (0 == wcscmp(Settings::getInstance().getValue(L"--renderType"),L"GL")) {
		m_drawVisitor = new GLVisitor();
	} 
	else if (0 == wcscmp(Settings::getInstance().getValue(L"--renderType"),L"CPUTrace")) {
		m_drawVisitor = new GLVisitorCPUTrace();
	}
	else if (0 == wcscmp(Settings::getInstance().getValue(L"--renderType"),L"GPUTrace")) {
		m_drawVisitor = new GLVisitorGPUTrace();
	}
    else if (0 == wcscmp(Settings::getInstance().getValue(L"--renderType"), L"GPUTraceSSD")) {
        m_drawVisitor = new GLVisitorGPUTraceSSD();
    }
	else {
		m_drawVisitor = new GLVisitorT();
	}
	//TODO : add this assignment as a part of the constructor
	m_drawVisitor->m_pScene = m_pScene;

    // Create the window
    glutInitWindowSize(m_pScene->m_resx, m_pScene->m_resy);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowPosition(100,100);
    __glutCreateWindowWithExit("gl view",MyExit);

	//should be once before first traverse and after GL
	// context initialization
	m_drawVisitor->initializeVisitor();

    // Initialize some OpenGL state
    glClearColor(0.25f, 0.25f, 0.25f, 1);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);	
}

void GLWindow::deinitGLWindow()
{
	m_pLoader->freeTransformVisitor(m_transform);
	m_transform = NULL;
	delete m_drawVisitor;
	//add some more deinitialization code here
}
void
GLWindow::display()
{
	GLWindow & win = GLWindow::getInstance();	
	
	DWORD currentTime = timeGetTime();
	float time =  ((float) currentTime - win.m_startTime) /500;
	if (0 == wcscmp(Settings::getInstance().getValue(L"--fixedTic"),L"1"))
	{
		static int tick = 0 ;
		time = ((float)tick) /10;
		tick += 1;
		if (59 == tick || 421 == tick) {
			tick++;
		}
	}
	win.m_pScene->setTime(time);

	//update geometry
	win.m_transform->traverse(win.m_pScene->m_pRoot);

	//update bounding boxes
	BoundingBoxVisitor bbVisitor(*win.m_pScene);
	bbVisitor.traverse(win.m_pScene->m_pRoot);

	//draw the image to the screen
	win.m_drawVisitor->traverse(win.m_pScene->m_pRoot);

    if (win.m_pScene->m_time < 5.0f)
        glutPostRedisplay();
    else
    {
        static int firstTime = 1;
        if (firstTime)
        {
            Statistics::getInstance().dumpResults();
        }

    }
        
 
	if (0 == wcscmp(Settings::getInstance().getValue(L"--singleFrame"),L"1"))
		exit(0);
}




