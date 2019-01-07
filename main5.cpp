#pragma comment(lib,"osgd.lib")
#pragma comment(lib,"osgDBd.lib")
#pragma comment(lib,"osgViewerd.lib")
#pragma comment(lib,"OpenThreadsd.lib")
#pragma comment(lib,"osgUtild.lib")
#pragma comment(lib,"osgGAd.lib")
#pragma comment(lib,"osgTextd.lib")

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/ComputeBoundsVisitor>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osgViewer/Viewer>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/ShapeDrawable>
#include <osg/PolygonMode>
#include <osgUtil/Optimizer>
#include <osg/CullFace>
#include <osgViewer/ViewerEventHandlers>
#include <osg/OcclusionQueryNode>
#include <osgGA/StateSetManipulator>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/LightModel>
#include <osg/Billboard>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <string>
#include <time.h>


struct int3
{
	int x;
	int y;
	int z;
};

static int3 make_int3(int x, int y, int z)
{
	int3 res;
	res.x = x;
	res.y = y;
	res.z = z;
	return res;
}


struct float3
{
	float x;
	float y;
	float z;
};

static float3 make_float3(float x, float y, float z)
{
	float3 res;
	res.x = x;
	res.y = y;
	res.z = z;
	return res;
}

float *pConsumedTime = NULL;
float consumedTime = 0.0f;

float radius = 10.0f;

osg::Drawable* createSquare(const osg::Vec3& corner, const osg::Vec3& width, const osg::Vec3& height, osg::Image* image = NULL)
{
	// set up the Geometry.
	osg::Geometry* geom = new osg::Geometry;

	osg::Vec3Array* coords = new osg::Vec3Array(4);
	(*coords)[0] = corner;
	(*coords)[1] = corner + width;
	(*coords)[2] = corner + width + height;
	(*coords)[3] = corner + height;


	geom->setVertexArray(coords);

	osg::Vec3Array* norms = new osg::Vec3Array(1);
	(*norms)[0] = width^height;
	(*norms)[0].normalize();

	geom->setNormalArray(norms, osg::Array::BIND_OVERALL);

	osg::Vec2Array* tcoords = new osg::Vec2Array(4);
	(*tcoords)[0].set(0.0f, 0.0f);
	(*tcoords)[1].set(1.0f, 0.0f);
	(*tcoords)[2].set(1.0f, 1.0f);
	(*tcoords)[3].set(0.0f, 1.0f);
	geom->setTexCoordArray(0, tcoords);

	osg::Vec4Array* colours = new osg::Vec4Array(1);
	(*colours)[0].set(1.0f, 1.0f, 1.0f, 1.0f);
	geom->setColorArray(colours, osg::Array::BIND_OVERALL);


	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

	if (image)
	{
		osg::StateSet* stateset = new osg::StateSet;
		osg::Texture2D* texture = new osg::Texture2D;
		texture->setImage(image);
		stateset->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
		stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
		stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
		stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		geom->setStateSet(stateset);
	}

	return geom;
}

osg::Image* createBillboardImage(const osg::Vec4& centerColour, unsigned int size, float power)
{
	osg::Vec4 backgroundColour = centerColour;
	backgroundColour[3] = 0.0f;

	osg::Image* image = new osg::Image;
	image->allocateImage(size, size, 1,
		GL_RGBA, GL_UNSIGNED_BYTE);


	float mid = (float(size) - 1)*0.5f;
	float div = 2.0f / float(size);
	for (unsigned int r = 0; r<size; ++r)
	{
		unsigned char* ptr = image->data(0, r, 0);
		for (unsigned int c = 0; c<size; ++c)
		{
			float dx = (float(c) - mid)*div;
			float dy = (float(r) - mid)*div;
			float r = powf(1.0f - sqrtf(dx*dx + dy*dy), power);
			if (r<0.0f) r = 0.0f;
			osg::Vec4 color = centerColour*r + backgroundColour*(1.0f - r);
			// color.set(1.0f,1.0f,1.0f,0.5f);
			*ptr++ = (unsigned char)((color[0])*255.0f);
			*ptr++ = (unsigned char)((color[1])*255.0f);
			*ptr++ = (unsigned char)((color[2])*255.0f);
			*ptr++ = (unsigned char)((color[3])*255.0f);
		}
	}
	return image;

	//return osgDB::readImageFile("spot.dds");
}


osg::ref_ptr<osg::Group> createSunLight(float size, osg::Vec4 color)
{
	osg::ref_ptr<osg::LightSource> sunLightSource = new osg::LightSource;
	osg::ref_ptr<osg::Light> sunLight = sunLightSource->getLight();
	sunLight->setPosition(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
	sunLight->setAmbient(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));

	sunLightSource->setLight(sunLight);
	sunLightSource->setLocalStateSetModes(osg::StateAttribute::ON);
	sunLightSource->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);
	osg::LightModel* lightModel = new osg::LightModel;
	lightModel->setAmbientIntensity(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
	sunLightSource->getOrCreateStateSet()->setAttribute(lightModel);

	osg::ref_ptr<osg::Billboard> sunBillboard = new osg::Billboard();
	sunBillboard->setMode(osg::Billboard::POINT_ROT_EYE);
	sunBillboard->addDrawable(
		createSquare(osg::Vec3(-size, 0.0f, -size), osg::Vec3(size*2, 0.0f, 0.0f), osg::Vec3(0.0f, 0.0f, size*2), createBillboardImage(color, 64, 2.0)),
		osg::Vec3(0.0f, 0.0f, 0.0f));
	sunLightSource->addChild(sunBillboard);

	return sunLightSource;
	
}

void generatePointsOnSphere(std::vector<float3> &coords, float radius,float step)
{
	//coords.push_back(make_float3(0.0f, 0.0f, radius));
	//coords.push_back(make_float3(0.0f, 0.0f, -radius));
	//return;
	float x, y, z;
	x = -radius;
	for (x = -radius; x < radius; x += step)
	{
		for (y = -radius; y < radius; y += step)
		{
			if (x * x + y * y <= radius * radius)
			{
				z = sqrt(radius * radius - x * x - y * y);
				//if(rand() % 2)
					coords.push_back(make_float3(x, y, z));
				//if (rand() % 2)
					coords.push_back(make_float3(x, y, -z));
					//return;
			}
		}
	}
}

void generateRenderRotateAxis(std::vector<float3> & axises, std::vector<float3> &coords)
{
	for (int i = 0; i < coords.size(); ++i)
	{
		glm::vec3 randomAxis;
		randomAxis.x = rand() % 10000 - 5000;
		randomAxis.y = rand() % 10000 - 5000;
		randomAxis.z = rand() % 10000 - 5000;
		glm::vec3 pointDir = glm::vec3(coords[i].x, coords[i].y, coords[i].z);
		glm::vec3 rAxis = glm::cross(randomAxis, pointDir);
		//int x = rand() % 10000 - 5000;
		//int y = rand() % 10000 - 5000;
		//int z = rand() % 10000 - 5000;
		axises.push_back(make_float3(rAxis.x, rAxis.y, rAxis.z));
	}
}

class ConsumedTimeCallback :public osg::UniformCallback
{
public:
	float *pConsumedTime;
	ConsumedTimeCallback(float *pConsumedTime)
	{
		this->pConsumedTime = pConsumedTime;
	}
	void operator()(osg::Uniform* uniform, osg::NodeVisitor* nv)
	{
		consumedTime = clock();
		//std::cout << consumedTime << std::endl;
		uniform->set(*pConsumedTime);
	}
};


osg::ref_ptr<osg::Vec3Array> createVertexArrayOfVoxel(float cube_Size) {

	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;

	vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
	vertices->push_back(osg::Vec3(cube_Size, 0.0f, 0.0f));
	vertices->push_back(osg::Vec3(cube_Size, cube_Size, 0.0f));
	vertices->push_back(osg::Vec3(cube_Size, cube_Size, 0.0f));
	//vertices->push_back(osg::Vec3(0.0f, cube_Size, 0.0f));
	//vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));

	//vertices->push_back(osg::Vec3(0.0f, 0.0f, cube_Size));
	//vertices->push_back(osg::Vec3(cube_Size, 0.0f, cube_Size));
	//vertices->push_back(osg::Vec3(cube_Size, cube_Size, cube_Size));
	//vertices->push_back(osg::Vec3(cube_Size, cube_Size, cube_Size));
	//vertices->push_back(osg::Vec3(0.0f, cube_Size, cube_Size));
	//vertices->push_back(osg::Vec3(0.0f, 0.0f, cube_Size));

	//vertices->push_back(osg::Vec3(0.0f, cube_Size, cube_Size));
	//vertices->push_back(osg::Vec3(0.0f, cube_Size, 0.0f));
	//vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
	//vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
	//vertices->push_back(osg::Vec3(0.0f, 0.0f, cube_Size));
	//vertices->push_back(osg::Vec3(0.0f, cube_Size, cube_Size));

	//vertices->push_back(osg::Vec3(cube_Size, cube_Size, cube_Size));
	//vertices->push_back(osg::Vec3(cube_Size, cube_Size, 0.0f));
	//vertices->push_back(osg::Vec3(cube_Size, 0.0f, 0.0f));
	//vertices->push_back(osg::Vec3(cube_Size, 0.0f, 0.0f));
	//vertices->push_back(osg::Vec3(cube_Size, 0.0f, cube_Size));
	//vertices->push_back(osg::Vec3(cube_Size, cube_Size, cube_Size));

	//vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
	//vertices->push_back(osg::Vec3(cube_Size, 0.0f, 0.0f));
	//vertices->push_back(osg::Vec3(cube_Size, 0.0f, cube_Size));
	//vertices->push_back(osg::Vec3(cube_Size, 0.0f, cube_Size));
	//vertices->push_back(osg::Vec3(0.0f, 0.0f, cube_Size));
	//vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));

	//vertices->push_back(osg::Vec3(0.0f, cube_Size, 0.0f));
	//vertices->push_back(osg::Vec3(cube_Size, cube_Size, 0.0f));
	//vertices->push_back(osg::Vec3(cube_Size, cube_Size, cube_Size));
	//vertices->push_back(osg::Vec3(cube_Size, cube_Size, cube_Size));
	//vertices->push_back(osg::Vec3(0.0f, cube_Size, cube_Size));
	//vertices->push_back(osg::Vec3(0.0f, cube_Size, 0.0f));

	return vertices;
}


osg::ref_ptr<osg::Geode> createVoxelsUseInstance(float cube_Size, std::vector<float3> coords, std::vector<float3> colors, std::vector<float3> axises, std::string texName)
{
	//create geometry
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	geometry->setVertexArray(createVertexArrayOfVoxel(cube_Size));
	geometry->setUseDisplayList(false);
	geometry->setUseVertexBufferObjects(true);
	geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, 4, coords.size()));

	//set every instance attribute data
	//load the instance attribute into the image, and compute the boundbox
	osg::ref_ptr<osg::Image> image = new osg::Image;
	image->allocateImage(1024, 1024, 1, GL_RGB, GL_FLOAT);
	osg::BoundingBox boundBox;
	float* data = (float*)image->data();
	for (int i = 0; i < coords.size(); ++i) {
		osg::Vec3 pos = osg::Vec3((coords[i].x * cube_Size / cube_Size), (coords[i].y * cube_Size / cube_Size), (coords[i].z * cube_Size / cube_Size));
		boundBox.expandBy(pos);
		*(data++) = pos[0];
		*(data++) = pos[1];
		*(data++) = pos[2];
	}
	geometry->setInitialBound(boundBox);
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
	texture->setImage(image);
	texture->setInternalFormat(GL_RGB32F_ARB);
	texture->setSourceFormat(GL_RGB);
	texture->setSourceType(GL_FLOAT);
	texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	geometry->getOrCreateStateSet()->setTextureAttributeAndModes(0, texture.get());
	geometry->getOrCreateStateSet()->addUniform(new osg::Uniform("instanceMatrixTexture", 0));
	geometry->getOrCreateStateSet()->addUniform(new	osg::Uniform("width", (int)image->s()));

	osg::ref_ptr<osg::Image> image2 = new osg::Image;
	image2->allocateImage(1024, 1024, 1, GL_RGB, GL_FLOAT);
	float* data2 = (float*)image2->data();
	for (int i = 0; i < colors.size(); ++i) {
		*(data2++) = colors[i].x;
		*(data2++) = colors[i].y;
		*(data2++) = colors[i].z;
	}
	osg::ref_ptr<osg::Texture2D> texture2 = new osg::Texture2D;
	texture2->setImage(image2);
	texture2->setInternalFormat(GL_RGB32F_ARB);
	texture2->setSourceFormat(GL_RGB);
	texture2->setSourceType(GL_FLOAT);
	texture2->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	texture2->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	geometry->getOrCreateStateSet()->setTextureAttributeAndModes(1, texture2.get());
	geometry->getOrCreateStateSet()->addUniform(new osg::Uniform("instanceColorTexture", 1));

	osg::ref_ptr<osg::Image> image3 = new osg::Image;
	image3->allocateImage(1024, 1024, 1, GL_RGB, GL_FLOAT);
	float *data3 = (float*)image3->data();
	for (int i = 0; i < axises.size(); ++i)
	{
		*(data3++) = axises[i].x;
		*(data3++) = axises[i].y;
		*(data3++) = axises[i].z;
	}
	osg::ref_ptr<osg::Texture2D> texture3 = new osg::Texture2D;
	texture3->setImage(image3);
	texture3->setInternalFormat(GL_RGB32F_ARB);
	texture3->setSourceFormat(GL_RGB);
	texture3->setSourceType(GL_FLOAT);
	texture3->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
	texture3->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	geometry->getOrCreateStateSet()->setTextureAttributeAndModes(2, texture3.get());
	geometry->getOrCreateStateSet()->addUniform(new osg::Uniform("instanceAxisTexture", 2));


	osg::ref_ptr<osg::Image> image4 = osgDB::readImageFile(texName);
	osg::ref_ptr<osg::Texture2D> texture4 = new osg::Texture2D;
	texture4->setImage(image4);
	geometry->getOrCreateStateSet()->setTextureAttributeAndModes(3, texture4.get());
	geometry->getOrCreateStateSet()->addUniform(new osg::Uniform("chakraTexture", 3));

	//create shaders
	osg::ref_ptr<osg::Program> program = new osg::Program;
	osg::ref_ptr<osg::Shader> vertShader = osg::Shader::readShaderFile(osg::Shader::VERTEX, "Shaders/cube.vert");
	osg::ref_ptr<osg::Shader> geomShader = osg::Shader::readShaderFile(osg::Shader::GEOMETRY, "Shaders/cube.geom");
	osg::ref_ptr<osg::Shader> fragShader = osg::Shader::readShaderFile(osg::Shader::FRAGMENT, "Shaders/cube.frag");
	program->addShader(vertShader);
	program->addShader(geomShader);
	program->addShader(fragShader);
	geometry->getOrCreateStateSet()->setAttributeAndModes(program, osg::StateAttribute::ON);

	//create geode
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(geometry);
	//set uniform
	osg::ref_ptr<osg::Uniform> uConsumedTime = new osg::Uniform(osg::Uniform::FLOAT, "t");
	pConsumedTime = &consumedTime;
	uConsumedTime->setUpdateCallback(new ConsumedTimeCallback(pConsumedTime));
	geode->getOrCreateStateSet()->addUniform(uConsumedTime);
	osg::BoundingBox bound_box;
	bound_box.expandBy(osg::Vec3(-radius,-radius,-radius));
	bound_box.expandBy(osg::Vec3(radius, radius, radius));
	geometry->setInitialBound(bound_box);

	geometry->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
	//geometry->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	return geode.release();
}

int main()
{
	//加载坐标
	std::vector<float3> coords;
	generatePointsOnSphere(coords,radius,0.25);
	//加载颜色
	std::vector<float3> colors;
	for(int i = 0;i < coords.size();++i)
		colors.push_back(make_float3(18.0f/255.0f, 150.0f/255.0f, 219.0f/255.0f));
	//加载旋转轴
	std::vector<float3> axises;
	generateRenderRotateAxis(axises, coords);

	//加载坐标
	std::vector<float3> coords2;
	generatePointsOnSphere(coords2, radius / 3, 0.1);
	//加载颜色
	std::vector<float3> colors2;
	for (int i = 0; i < coords2.size(); ++i)
		colors2.push_back(make_float3(18.0f / 255.0f, 150.0f / 255.0f, 219.0f / 255.0f));
	//加载旋转轴
	std::vector<float3> axises2;
	generateRenderRotateAxis(axises2, coords2);

	osgViewer::Viewer viewer;
	osg::ref_ptr<osg::Group> root = new osg::Group;
	root->addChild(createVoxelsUseInstance(0.05f, coords, colors, axises,"chakra2.png"));
	root->addChild(createVoxelsUseInstance(0.05f, coords2, colors2, axises2, "chakra3.png"));
	root->addChild(createSunLight(30.0,osg::Vec4(6.0f / 255.0f, 122.0f / 255.0f, 228.0f / 255.0f, 1.0)));

	viewer.setSceneData(root);
	viewer.addEventHandler(new osgViewer::HelpHandler);
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
	viewer.addEventHandler(new osgViewer::StatsHandler);


	{
		osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
		traits->x = 40;
		traits->y = 40;
		traits->width = 600;
		traits->height = 480;
		traits->windowDecoration = true;
		traits->doubleBuffer = true;
		traits->sharedContext = 0;
		traits->samples = 8;
		osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());

		osg::ref_ptr<osg::Camera> camera = new osg::Camera;
		camera->setClearColor(osg::Vec4f(0.0f, 0.0f, 0.0f, 0.0f));
		camera->setGraphicsContext(gc.get());
		camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
		GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
		camera->setDrawBuffer(buffer);
		camera->setReadBuffer(buffer);

		viewer.addSlave(camera.get());
	}

	viewer.run();

	return 0;
}