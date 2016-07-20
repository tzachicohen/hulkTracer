
#include <math.h>

#include "glew.h"
#include "util.hpp"
#include "GLVisitorCPUTrace.hpp"
#include "GLWindow.h"
#include "Settings.hpp"
#include "Statistics.hpp"


void GLVisitorCPUTrace::postAct(Node *)
{

}
void GLVisitorCPUTrace::initializeVisitor()
{
	if (NULL == m_pKdtree) {
		m_pKdtree = new KDTreeCPU();
	}
	glGenTextures(1,&m_texture);
	glBindTexture(GL_TEXTURE_2D,m_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,m_pScene->m_frameBuffer.getWidth(),m_pScene->m_frameBuffer.getHeight(),0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);

#ifdef _DEBUG
	//verify correctness of the light structure
	for (uint k = 0;  k< m_pScene->m_lights.size();k++) {
	HULK_ASSERT(m_pScene->m_lights[k].color.w == E_directionalLight || m_pScene->m_lights[k].color.w == E_pointLight,"unknown light identifier.\n")
	}
#endif
	if (0 == wcscmp(Settings::getInstance().getValue(L"--saveImages"),L"1")) {
		if (!dirExists(m_pScene->m_sceneName)) {
			CreateDirectoryA(m_pScene->m_sceneName.c_str(),NULL);
		}
	}
}

void GLVisitorCPUTrace::buildTree()
{
	//Build the KD tree;
	Statistics::getInstance().KDTreeBuildStart();
	m_pKdtree->buildKDTree(*m_pScene);
	Statistics::getInstance().KDTreeBuildEnd();

   // m_pKdtree->dumpTree();
}

bool GLVisitorCPUTrace::traverseBegin(Node *)
{
	
	buildTree();

	updateCamera();
	
	traceScene();
	
	updateFrameBuffer();

	glBegin(GL_QUADS);

		glTexCoord2f(0,0);
		glVertex2f( -1, -1 );

		glTexCoord2f(0.0f,1.0f);
		glVertex2f( -1, 1.0f );

		glTexCoord2f(1.0f,1.0f);
		glVertex2f( 1.0f, 1.0f );

		glTexCoord2f(1,0);
		glVertex2f( 1.0f, -1 );
	
	glEnd( );

	if (0 == wcscmp(Settings::getInstance().getValue(L"--saveImages"),L"1")) {
		saveFrameBufferToFile();
	}
	//no need to traverse the graph.
	return false;
}

void GLVisitorCPUTrace::updateFrameBuffer()
{
	//draw the frame buffer using OpenGL
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,m_pScene->m_frameBuffer.getWidth(),m_pScene->m_frameBuffer.getHeight(),0,GL_RGBA,GL_UNSIGNED_BYTE,m_pScene->m_frameBuffer.getBufferBits());
}
GLVisitorCPUTrace::~GLVisitorCPUTrace()
{
	delete m_pCamPos;
	glBindTexture(GL_TEXTURE_2D,0);
	glDeleteTextures(1,&m_texture);
}

GLVisitorCPUTrace::GLVisitorCPUTrace()
:m_texture(0)
,m_pKdtree(NULL)
{
	m_pCamPos = new CameraPos();
}

void GLVisitorCPUTrace::updateCamera()
{
	//	TODO: move camera translation and ray generation code to BartLoader project.
	SPMatrix rotMat;
	SPMatrix transMat;
	//update camera point of view
	//first transform 
	double trans[3] ={0};
	double rot[4] = {0};
	GLWindow::getInstance().m_pLoader->getTransformation(*m_pScene,"camera",rot,trans);
	
	transMat.TranslateMatrix((float)trans[0],(float)trans[1],(float)trans[2]);
	rotMat.rotateMatrix((float)rot[0],(float)rot[1],(float)rot[2],-(float)rot[3]);
	
	//now that we have a tran
	SPVector from (0,0,0);
	SPVector down(0,-1,0);
	SPVector left(-1,0,0);
	SPVector to (0,0,-1);

	m_pCamPos->m_from = from * transMat;
	
	down = down * rotMat;
	left = left * rotMat;
	to = to* rotMat;

	down.w = 0;
	left.w = 0;
	to.w = 0;
	m_pCamPos->m_from.w = 0;

	float verticalFactor = tan(DEG_TO_RAD(m_pScene->m_fov) /2);
	float horizontalFactor = verticalFactor*m_pScene->m_aspect;
	
	down = down * verticalFactor;
	left = left * horizontalFactor;

	m_pCamPos->m_lowerLeft = m_pCamPos->m_from + to + down +left;

	m_pCamPos->m_verticalFactor = left * -2.0;
	m_pCamPos->m_horizontalFactor = down * -2.0 ;
	
	return;
}
void GLVisitorCPUTrace::traceScene()
{
	//clear the frame buffer 
	m_pScene->m_frameBuffer.clearBuffer();

	Statistics::getInstance().frameStart();
#pragma omp parallel for
	for (int i =0 ; i < ((int) m_pScene->m_frameBuffer.getHeight() ); i++)
	{
		HitData hitData;
		SPVector color(0.0f,0.0f,0.0f);
		Ray ray;
		for (uint j =0 ; j < m_pScene->m_frameBuffer.getWidth(); j++)
		{
			
			castRay(j,i,ray);
			if (m_pKdtree->checkCollision(ray,hitData))
			{
				static const SPVector ambiant (0.1,0.1,0.1);
				color = ambiant;
				for (uint k = 0;  k< m_pScene->m_lights.size();k++) {
					
					if (E_directionalLight == m_pScene->m_lights[k].color.w) {
						float intensity = hitData.normal * m_pScene->m_lights[k].data.dir;
						color += m_pScene->m_lights[k].color * MAX(intensity,0);
					}
					//if this is a point light
					if (E_pointLight == m_pScene->m_lights[k].color.w) {
						SPVector toLight = m_pScene->m_lights[k].data.pos - hitData.position;
						toLight.w = 0;
						float distToLight = sqrt(toLight*toLight);
						toLight.normalize();

						HitData shadowHit;
						ray.m_rayDir = toLight;
						ray.m_rayStart = hitData.position + toLight*0.001;
						bool hit = m_pKdtree->checkCollision(ray,shadowHit);
						float intensity = hitData.normal * toLight;
						if (hit && distToLight > shadowHit.time)
						{
							intensity *= 0.5;
						}
						color += m_pScene->m_lights[k].color * MAX(intensity,0);
					}
				}
				m_pScene->m_frameBuffer.replacePixel(j,i,color);
			}

		}
	}
	Statistics::getInstance().frameEnd();
}
bool GLVisitorCPUTrace::castRay(unsigned int row , unsigned int column,OUT Ray & ray)
{
	//by default the camera looks at the Negative Z.
	ray.m_rayStart = m_pCamPos->m_from;

	//
	float relativeRow = float (row)/ float(m_pScene->m_resx);
	float relativecolumn = float(column) / float(m_pScene->m_resy);

	ray.m_rayDir = (m_pCamPos->m_lowerLeft + (m_pCamPos->m_verticalFactor  * relativeRow ) +
		(m_pCamPos->m_horizontalFactor  * relativecolumn)) - m_pCamPos->m_from;
	
	ray.m_rayDir.normalize();

	return true;
}

bool GLVisitorCPUTrace::saveFrameBufferToFile()
{
	static uint frameNum = 0;
	char number [16];
	
	sprintf(number,"_%.3u",frameNum);
	frameNum++;
	std::string fileName = m_pScene->m_sceneName + "\\" + m_pScene->m_sceneName + number;
	return e_ok == m_pScene->m_frameBuffer.saveToFile(fileName);
}


bool GLVisitorCPUTrace::dirExists(const std::string& dirName_in)
{
  DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
  if (ftyp == INVALID_FILE_ATTRIBUTES)
    return false;  //something is wrong with your path!

  if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
    return true;   // this is a directory!

  return false;    // this is not a directory!
}



