#ifndef GLWINDOW_H_INCLUDED
#define GLWINDOW_H_INCLUDED

#include <Windows.h>

#include "Scene.hpp"
#include "glut.h"
#include "GLVisitor.hpp"
#include "LoaderInterface.hpp"

class GLWindow
{
public:
    void mainLoop();
    void initGLWindow(Scene & scene,LoaderInterface & loader);
	void deinitGLWindow();
    // GLUT Event handlers
    static void display();
    static GLWindow & getInstance()
	{
		static GLWindow s_instance;
		return  s_instance;
	}

    LoaderInterface * m_pLoader;
protected:
	static GLWindow * s_instance;
	GLWindow();
	~GLWindow();
	Scene * m_pScene;
	Visitor* m_transform;
	GLVisitor* m_drawVisitor;
	DWORD m_startTime;
};

#endif // GLWINDOW_H_INCLUDED


