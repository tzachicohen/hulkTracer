//////////////////////////////////////////////////////////////////
// OBJLoader.cpp												//
// Hulk renderer - Create by Tzachi Cohen 2010					//
// all rights reserved(c)										//
// tzachi_cohen@hotmail.com										//
//////////////////////////////////////////////////////////////////
#include <vector>

#include "OBJLoader.hpp"
#include "AnimatedTransform.hpp"
#include "DynamicMeshNode.hpp"
#include "OBJTransformVisitor.hpp"
		  
//load the object from file
bool OBJLoader::viParseFile(FILE *f,const std::string &relPath,Scene & scene)
{
	size_t cur_mat = 0; //start with the dafault meterial
	char token[256];
	char faceToken[3][128];		// buffer to read polygon data 
	float x,y,z; //temporary vector to read vector data

	scene.m_pRoot = new AnimatedTransform();

	if (NULL == f)
	{
		return false;
	}
	Stack<SPVector> vertices;
	Stack<SPVector> normals;
	Stack<unsigned int> indices;
	while(!feof(f))
	{
		token[0] = NULL;
		fscanf(f,"%s", token);

		if (!strcmp(token,"g"))
		{
			fscanf(f,"%s",token);
		}

		else if (!strcmp(token,"mtllib"))
		{
			//HULK_ASSERT(false,"add support for OBJ material load.\n")
  			//fscanf(f,"%s", token);
			//loadMaterial(path+ "\\" + token,object); //remove first two notes
		}

		else if (!strcmp(token,"usemtl"))
		{
			//HULK_WARNING(false,"add support for OBJ material load.\n")
			//fscanf(f,"%s",token);
			//cur_mat = object.m_materialNames[token];
		}

		else if (!strcmp(token,"v"))
		{
			fscanf(f,"%f %f %f",&x,&y,&z);
			SPVector temp(x,y,z);
			vertices.push_back(temp);
		}

		else if (!strcmp(token,"vn"))
		{
			fscanf(f,"%f %f %f",&x,&y,&z);
			SPVector temp(x,y,z);
			normals.push_back(temp);
		}
		else if (!strcmp(token,"vt"))
		{
			//HULK_ASSERT(false,"add support for OBJ texture coordinates .\n")
			//fscanf(f,"%f %f",&x,&y);
			//object.m_texCoordList.push_back(R2Vec(x,y));
		}

		else if (!strcmp(token,"f"))
		{
			for (int i=0;i<3;i++)
			{
				fscanf(f,"%s",faceToken[i]);
			}

			for (int i=0;i<3;i++)		// for each vertex of this face
			{
				char str[20] ; // temporary string to hold index number
				char  ch; 
				int base,offset; //base and offset indice of the string being parsed
				unsigned int vertexIndex ,texIndex,normalIndex; //
				base = offset = 0;
				
				// calculate vertex-list index
				while( (ch=faceToken[i][base+offset]) != '/' && (ch=faceToken[i][base+offset]) != '\0')
				{
					str[offset] = ch;
					offset++;
				}
				str[offset] = '\0';
				vertexIndex = atoi(str);
				base += (ch == '\0')? offset : offset+1;
				offset = 0;

				// calculate texture-list index
				while( (ch=faceToken[i][base+offset]) != '/' && (ch=faceToken[i][base+offset]) != '\0')
				{
					str[offset] = ch;
					offset++;
				}
				str[offset] = '\0';
				texIndex = atoi(str);	
				base += (ch == '\0')? offset : offset+1;
				offset = 0;

				// calculate normal-list index
				while( (ch=faceToken[i][base+offset]) != '\0')
				{
					str[offset] = ch;
					offset++;
				}
				str[offset] = '\0';
				normalIndex = atoi(str);	

				indices.push_back( vertexIndex - 1); //convert to zero based index
				//tempPoly.m_normalIndices[i] = normalIndex - 1;
				//tempPoly.m_texIndices[i] = texIndex - 1;
			}
			
		}  //end of : 'if (!strcmp(token,"f"))'
		else if (!strcmp(token,"#"))	 
		{
			fgets(token,100,f);
		}
	} // end of : 'while(!feof(f))'

	// add rotation transform
	AnimatedTransform * transform  = new AnimatedTransform();
	scene.m_pRoot->addSon(transform);

	// add dynamic Mesh node
	DynamicMeshNode * mesh = new DynamicMeshNode();
	transform->addSon(mesh);
	{
		//allocate space in the global list for the vertices
		uint verticesOffset = scene.allocateVertices(vertices.size());
		mesh->m_meshData.e_vertexData =1 ;
		// all indices are located in the a single joint global list pointed from the 'Scene' class.
		uint indicesOffset = scene.setVertexIndices(indices.data(),indices.size()/3,verticesOffset);
		// the untransform vertices and normals are located in the nodes
		mesh->setIndexInfo(indicesOffset,indices.size()/3);
		mesh->setVertices(vertices.data(),vertices.size(),verticesOffset);
	}
	getVerticesNormals(indices,vertices,normals);
	{
		uint normalsOffset = scene.allocateNormals(normals.size());
		mesh->m_meshData.e_normalData= 1 ;
		uint normIndicesOffset = scene.setNormIndices(indices.data(),indices.size()/3,normalsOffset);
		mesh->setNormalIndexInfo(normIndicesOffset);
		// the untransform vertices and normals are located in the nodes
		mesh->setNorms(normals.data(),normals.size(),normalsOffset);
	}
	if (0 !=fclose(f))
	{
		return false;
	}
	//global scene data
	scene.m_resx = 800;
	scene.m_resy = 600;
	scene.m_aspect = ((float) scene.m_resx) / ((float) scene.m_resy);
	Light light;
	SPVector dir(-1.0f,1.0f,1.0f) ;
	dir.normalize();
	light.data.dir = dir.vec;
	light.color = SPVector(0.9f,0.9f,0.9f,E_directionalLight);
	scene.m_lights.push_back(light);
	return true;
}

bool OBJLoader::getVerticesNormals(const Stack<unsigned int> & indices,const Stack<SPVector> &vertices, 
		OUT Stack<SPVector>  & normals)
{
	Stack<SPVector> polygonNomals;
	normals.resize(vertices.size());
	normals.setZeros();

	for ( uint32 i = 0; i < indices.size();i +=3) {
		SPVector p1 = vertices[indices[i]];
		SPVector p2 = vertices[indices[i+1]];
		SPVector p3 = vertices[indices[i+2]];

		SPVector edge1 = p2 - p1;
		SPVector edge2 = p3 - p2;
		SPVector edge3 = p1 - p3;
		SPVector polyNormal = (edge1 % edge2)+ (edge2 % edge3) + (edge3 % edge1);
		polygonNomals.push_back(polyNormal);
	}
	for ( uint32 i = 0; i < indices.size();i +=3) 
	{
		uint32 indexNum = i/3;
		normals[indices[i]] += polygonNomals[indexNum];
		normals[indices[i+1]] += polygonNomals[indexNum];
		normals[indices[i+2]] += polygonNomals[indexNum];
	}
	//normalize the vertex mormals
	for (uint32 i = 0 ; i < normals.size(); i++) {
		normals[i].normalize();
	}

	return true;
}
EHulkResult  OBJLoader::loadMaterial(std::string fileName)
{
	HULK_ASSERT(false,"add support for OBJ material load.\n")
	/*
	char token[100]; //temporary buffer to read tokens
	float r,g,b; //temporary veriables to read data
	int intVal; //temporary veriables to read data

	size_t cur_mat = 0;

	FILE * f = fopen(fileName.c_str(),"r");
	if (NULL == f)
	{
		return e_fileOpenFail;
	}
	while(!feof(f))
	{
		token[0] = NULL;
		fscanf(f,"%s", token);		

		if (0 == strcmp(token,"newmtl"))
		{
			fscanf(f,"%s",token);
			Material temp;
			object.m_materials.push_back(temp);
			cur_mat = object.m_materials.size() - 1;
			object.m_materialNames[std::string(token)] = cur_mat;
		}

		else if (!strcmp(token,"Ka"))
		{
			fscanf(f,"%f %f %f",&r,&g,&b);
			object.m_materials[cur_mat].m_ambiant[0] = r;
			object.m_materials[cur_mat].m_ambiant[1] = g;
			object.m_materials[cur_mat].m_ambiant[2] = b;
		}
		else if (!strcmp(token,"Kd"))
		{
			fscanf(f,"%f %f %f",&r,&g,&b);
			object.m_materials[cur_mat].m_diffuse[0] = r;
			object.m_materials[cur_mat].m_diffuse[1] = g;
			object.m_materials[cur_mat].m_diffuse[2] = b;
		}
		else if (!strcmp(token,"Ks"))
		{
			fscanf(f,"%f %f %f",&r,&g,&b);
			object.m_materials[cur_mat].m_specular[0] = r;
			object.m_materials[cur_mat].m_specular[1] = g;
			object.m_materials[cur_mat].m_specular[2] = b;
		}
		else if (!strcmp(token,"d"))
		{
			fscanf(f,"%f",&r);
			object.m_materials[cur_mat].m_d = r;
		}
		else if (!strcmp(token,"Ns"))
		{
			fscanf(f,"%d",&intVal);
			object.m_materials[cur_mat].m_ns = intVal;
		}
		else if (!strcmp(token,"illum"))
		{
			fscanf(f,"%d",&intVal);
			object.m_materials[cur_mat].m_illum= intVal;
		}

		else if (!strcmp(token,"#")) //read till the end of the line 
		{
			fgets(token,100,f);
		}
	} //while (!feof(f)

	if (0 ==fclose(f))
	{
		return e_fileCloseFail;
	}
	*/
	return e_ok;
}

bool OBJLoader::getTransformation(Scene & scene,const char name [] , OUT double rot [4] , OUT double trans[3] ) 
{

	if (0 == strcmp(name,"camera"))
	{
		rot[0] = 0.0f;
		rot[1] = 1.0f;
		rot[2] = 0.0f;
        rot[3] = scene.m_time/10; //= 45 degrees

        SPMatrix rotMat;
        rotMat.rotateMatrix(0.0, 1.0, 0.0, -scene.m_time/10);
        SPMatrix transMat;
        transMat.TranslateMatrix(1000.0, 100.0, 0.0);
        SPVector from(0, 0, 0);
        from = from *  transMat * rotMat;
        
        trans[0] = from.x;
        trans[1] = from.y;
        trans[2] = from.z;
	}
	else
	{
        trans[0] = 0;
        trans[1] = 0;
        trans[2] = 0;
		rot[0] = 0.0f;
		rot[1] = 1.0f;
		rot[2] = 0.0f;
		rot[3] = 0; //= 90 degrees
	}

	return true;
}

Visitor * OBJLoader::getTransformVisitor(Scene & scene) 
{
	return new OBJTransformVisitor(scene);
}

void OBJLoader::freeTransformVisitor(Visitor * vis) 
{
	delete vis;
}