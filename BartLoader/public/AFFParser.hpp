/* File:    parse.h
 * Purpose: simple parser for data files, includeing args parsing
 */
#ifndef AFF_PARSER_H
#define AFF_PARSER_H

#include "LoaderInterface.hpp"
#include "Stack.hpp"
#include "scene.hpp"

#ifndef  M_PI
#define  M_PI 3.1415926535897932384626433
#endif

typedef float Vec2f[2];
typedef float Vec3f[3];
typedef float Vec4f[4];

#define V4SET4(vec,val0,val1,val2,val3) \
	vec[0] = val0; \
	vec[1] = val1; \
	vec[2] = val2; \
	vec[3] = val3; 

#define X 0 
#define Y 1  
#define Z 2  
#define W 3  
             
#define R 0  
#define G 1  
#define B 2  
#define A 3 /* alpha */

struct Animation;

class AFFParser : public LoaderInterface
{
public:
	virtual bool viParseFile(FILE *f,const std::string & relPath,Scene & scene);
	virtual bool getTransformation(Scene & scene,const char name [],OUT double rot [4] , double trans[3] ) ;
	virtual Visitor * getTransformVisitor(Scene & scene) ;
	virtual void freeTransformVisitor(Visitor *) ;
	AFFParser();
	~AFFParser();
private:
	bool	viParseArgs(int argc, char *argv[], char **filename,Scene & scene);

	void parseComment(FILE *f);
	void parseViewpoint(FILE *fp);
	void parseLight(FILE *fp);
	void parseBackground(FILE *fp);
	void parseFill(FILE *fp);
	void parseCone(FILE *fp);
	void parseSphere(FILE *fp);
	void parsePoly(FILE *fp);
	void parseInclude(FILE *fp);
	void parseDetailLevel(FILE *fp);
	void parseTexturedTriangle(FILE *fp);
	void parseAnimatedTriangle(FILE *fp);
	void parseTextureStuff(FILE *fp);
	void eatWhitespace(FILE *f);
	void parseKeyFrames(FILE *fp);
	void parseXform(FILE *f);
	void viEndXform();
	void parseAnimParams(FILE *fp);
	void parseA(FILE *f);
	void getVectors(FILE *fp,char *type,int *num_vecs,Vec3f **vecs);
	void getTextureCoords(FILE *fp,char *texturename,int *num,Vec2f **vecs);
	void getTriangles(FILE *fp,int *num_tris,unsigned short **indices,unsigned short **normIndices,unsigned short **texIndices,
			 Vec3f *verts,Vec3f *norms,Vec2f *txts);	
	void parseMesh(FILE *fp);
	bool viParseFile(FILE *f);
/////////////////////
// private members //
 ////////////////////

 std::string m_relPath;
 Scene * m_pScene;
 Stack<Node*> m_nodeStack; // a stack of all the nodes in current traverse
public:
 Stack<Animation*> m_animation;

};

#endif
