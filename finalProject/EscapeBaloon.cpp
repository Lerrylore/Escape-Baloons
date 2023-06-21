// This has been adapted from the Vulkan tutorial

#include "Starter.hpp"
#include "model.cpp"
#include <list>
#include <cmath>

#define WAVE_SIZE 20
#define MIN_AREA -5.0f
#define MAX_AREA 5.0f

// The uniform buffer objects data structures
// Remember to use the correct alignas(...) value
//        float : alignas(4)
//        vec2  : alignas(8)
//        vec3  : alignas(16)
//        vec4  : alignas(16)
//        mat3  : alignas(16)
//        mat4  : alignas(16)

struct MeshUniformBlock {
	alignas(4) float amb;
	alignas(4) float gamma;
	alignas(16) glm::vec3 sColor;
	alignas(16) glm::mat4 mvpMat[WAVE_SIZE];
	alignas(16) glm::mat4 mMat[WAVE_SIZE];
	alignas(16) glm::mat4 nMat[WAVE_SIZE];
};

struct OverlayUniformBlock {
	alignas(4) float visible;
};

struct GlobalUniformBlock {
	alignas(4) float cosout;
	alignas(4) float cosin;
	alignas(16) glm::vec3 lightPos;
	alignas(16) glm::vec3 DlightDir;
	alignas(16) glm::vec3 DlightColor;
	alignas(16) glm::vec3 eyePos;
};

// The vertices data structures
struct VertexMesh {
	glm::vec3 pos;
	glm::vec3 norm;
	glm::vec2 UV;
};

struct VertexOverlay {
	glm::vec2 pos;
	glm::vec2 UV;
};

struct VertexNormMap {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec4 tangent;
	glm::vec2 texCoord;
};

struct MeshCounters {
	int alien = 0;
	int opal = 0;
	int knit = 0;
	int shatter = 0;
};


float gameTime;


void GameLogic(float deltaT, float Ar, glm::mat4 &ViewPrj, glm::mat4 &World);
// MAIN ! 
class SlotMachine : public BaseProject {
	protected:

	// Current aspect ratio (used by the callback that resized the window
	float Ar;
	bool flaggswag = false;

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSLGubo, DSLMesh, DSLOverlay, DSLNormMap;

	// Vertex formats
	VertexDescriptor VMesh;
	//VertexDescriptor VBoundaries;
	VertexDescriptor VOverlay;
	VertexDescriptor VNorm;

	// Pipelines [Shader couples]
	Pipeline PMesh;
	Pipeline PBoundaries;
	Pipeline POverlay;
	Pipeline PNormMap;
	Pipeline PNayar;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex structure
	Model<VertexMesh> MCharacter, MFloor, MSphere, MBoundaries;
	Model<VertexOverlay> MKey, MSplash, MStartGame, MGameOver;
	Model<VertexNormMap> MSphereGLTF;
	DescriptorSet DSGubo, DSCharacter, DSSphere1, DSSphere2, DSSphere3, DSSphere4, DSSphereS, DSBall, DSFloor, DSBoundaries, DSStartGame, DSGameOver;
	Texture TCharacter, TFloor, TSphere1, TSphere2, TSphere3, TSphere4, TSphere1N, TSphere1M, TSphere4N, TSphere4M, TBoundaries, TStartGame, TGameOver;
	
	// C++ storage for uniform variables
	MeshUniformBlock uboCharacter, uboFloor, uboBoundaries, uboSphere1, uboSphere2, uboSphere3, uboSphere4, uboSphereS;
	GlobalUniformBlock gubo;
	OverlayUniformBlock uboKey, uboSplash, uboStartGame, uboGameOver;


	// Other application parameters
	float CamH, CamRadius, CamPitch, CamYaw;
	MeshCounters meshCounters;
	int gameState = 0;


	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "Escape Baloon";
    	windowResizable = GLFW_TRUE;
		initialBackgroundColor = {0.0f, 0.005f, 0.01f, 1.0f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 20;
		texturesInPool = 20;
		setsInPool = 20;
		
		Ar = (float)windowWidth / (float)windowHeight;
	}
	
	// What to do when the window changes size
	void onWindowResize(int w, int h) {
		Ar = (float)w / (float)h;
	}

	void createBoundariesMesh(std::vector<VertexMesh> &vDef, std::vector<uint32_t> &vIdx) {
		int vertexCount = 0;
		
		addBlock(-5.5f, 5.5f, 5.0f, 5.0f, vDef, vIdx, vertexCount); //bottom
		addBlock(-5.0f, -5.5f, 5.5f, -5.0f, vDef, vIdx, vertexCount);//top
		addBlock(-5.5f, -5.5f, -5.0f, 5.0f, vDef, vIdx, vertexCount);//left
		addBlock(5.5f, -5.0f, 5.0f, 5.5f, vDef, vIdx, vertexCount); //right
	}
	void addBlock(float firstX, float firstZ, float lastX, float lastZ, std::vector<VertexMesh> &vDef, std::vector<uint32_t> &vIdx, int &vertexCount) {
		glm::vec2 uv = {0.0, 0.0};
		
		glm::vec3 A = {firstX, 2.5f, firstZ};
		glm::vec3 B = {lastX, 2.5f, firstZ};
		glm::vec3 C = {lastX, 2.5f, lastZ};
		glm::vec3 D = {firstX, 2.5f, lastZ};
		glm::vec3 E = {firstX, 0.1f, firstZ};
		glm::vec3 F = {lastX, 0.1f, firstZ};
		glm::vec3 G = {lastX, 0.1f, lastZ};
		glm::vec3 H = {firstX, 0.1f, lastZ};
		
		//face up
		vDef.push_back({ A, {0.0f, 1.0f, 0.0f}, uv });
		vDef.push_back({ B, {0.0f, 1.0f, 0.0f}, uv });
		vDef.push_back({ D, {0.0f, 1.0f, 0.0f}, uv });
		vDef.push_back({ C, {0.0f, 1.0f, 0.0f}, uv });
		
		vertexCount += 4;
		
		vIdx.push_back(vertexCount - 4); vIdx.push_back(vertexCount - 3); vIdx.push_back(vertexCount - 2);	// First triangle
		vIdx.push_back(vertexCount - 3); vIdx.push_back(vertexCount - 2); vIdx.push_back(vertexCount - 1);	// Second triangle
		
		//face front
		vDef.push_back({ B, {1.0f, 0.0f, 0.0f}, uv });
		vDef.push_back({ F, {1.0f, 0.0f, 0.0f}, uv });
		vDef.push_back({ C, {1.0f, 0.0f, 0.0f}, uv });
		vDef.push_back({ G, {1.0f, 0.0f, 0.0f}, uv });
		
		vertexCount += 4;
		
		vIdx.push_back(vertexCount - 4); vIdx.push_back(vertexCount - 3); vIdx.push_back(vertexCount - 2);	// First triangle
		vIdx.push_back(vertexCount - 3); vIdx.push_back(vertexCount - 2); vIdx.push_back(vertexCount - 1);	// Second triangle
		
		//face left
		vDef.push_back({ A, {0.0f, 0.0f, -1.0f}, uv });
		vDef.push_back({ B, {0.0f, 0.0f, -1.0f}, uv });
		vDef.push_back({ E, {0.0f, 0.0f, -1.0f}, uv });
		vDef.push_back({ F, {0.0f, 0.0f, -1.0f}, uv });
		
		vertexCount += 4;
		
		vIdx.push_back(vertexCount - 4); vIdx.push_back(vertexCount - 3); vIdx.push_back(vertexCount - 2);	// First triangle
		vIdx.push_back(vertexCount - 3); vIdx.push_back(vertexCount - 2); vIdx.push_back(vertexCount - 1);	// Second triangle
		
		//face back
		vDef.push_back({ A, {-1.0f, 0.0f, 0.0f}, uv });
		vDef.push_back({ D, {-1.0f, 0.0f, 0.0f}, uv });
		vDef.push_back({ E, {-1.0f, 0.0f, 0.0f}, uv });
		vDef.push_back({ H, {-1.0f, 0.0f, 0.0f}, uv });
		
		vertexCount += 4;
		
		vIdx.push_back(vertexCount - 4); vIdx.push_back(vertexCount - 3); vIdx.push_back(vertexCount - 2);	// First triangle
		vIdx.push_back(vertexCount - 3); vIdx.push_back(vertexCount - 2); vIdx.push_back(vertexCount - 1);	// Second triangle
		
		//face right
		vDef.push_back({ C, {0.0f, 0.0f, 1.0f}, uv });
		vDef.push_back({ D, {0.0f, 0.0f, 1.0f}, uv });
		vDef.push_back({ G, {0.0f, 0.0f, 1.0f}, uv });
		vDef.push_back({ H, {0.0f, 0.0f, 1.0f}, uv });
		
		vertexCount += 4;
		
		vIdx.push_back(vertexCount - 4); vIdx.push_back(vertexCount - 3); vIdx.push_back(vertexCount - 2);	// First triangle
		vIdx.push_back(vertexCount - 3); vIdx.push_back(vertexCount - 2); vIdx.push_back(vertexCount - 1);	// Second triangle
	}
	
	void createImageIndexed(std::vector<VertexOverlay> &vDef, std::vector<uint32_t> &vIdx) {
		vDef.push_back({{-1.0f, -1.0f}, {0.0f, 0.0f}});
		vDef.push_back({{-1.0f, 1.0f}, {0.0f, 1.0f}});
		vDef.push_back({{1.0f, -1.0f}, {1.0f, 0.0f}});
		vDef.push_back({{1.0f, 1.0f}, {1.0f, 1.0f}});

		vIdx.push_back(0); vIdx.push_back(1); vIdx.push_back(2); 
		vIdx.push_back(1); vIdx.push_back(2); vIdx.push_back(3); 
	}

	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	void localInit() {

		// Descriptor Layouts [what will be passed to the shaders]
		DSLMesh.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
				});
				
		DSLOverlay.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
				});				
		DSLGubo.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
				});
		DSLNormMap.init(this, {
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
					{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
					{3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
		});

		// Vertex descriptors
		VMesh.init(this, {
				  {0, sizeof(VertexMesh), VK_VERTEX_INPUT_RATE_VERTEX}
				}, {
				  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMesh, pos),
				         sizeof(glm::vec3), POSITION},
				  {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexMesh, norm),
				         sizeof(glm::vec3), NORMAL},
				  {0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexMesh, UV),
				         sizeof(glm::vec2), UV}
				});

		VOverlay.init(this, {
				  {0, sizeof(VertexOverlay), VK_VERTEX_INPUT_RATE_VERTEX}
				}, {
				  {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexOverlay, pos),
				         sizeof(glm::vec2), OTHER},
				  {0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexOverlay, UV),
				         sizeof(glm::vec2), UV}
				});

		VNorm.init(this, {
				  {0, sizeof(VertexNormMap), VK_VERTEX_INPUT_RATE_VERTEX}
				}, {
				  {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexNormMap, pos),
				         sizeof(glm::vec3), POSITION},
				  {0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexNormMap, normal),
				         sizeof(glm::vec3), NORMAL},
				  {0, 2, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(VertexNormMap, tangent),
				         sizeof(glm::vec4), TANGENT},
				  {0, 3, VK_FORMAT_R32G32_SFLOAT, offsetof(VertexNormMap, texCoord),
				         sizeof(glm::vec2), UV}
				});

		PMesh.init(this, &VMesh, "shaders/MeshVert.spv", "shaders/MeshFrag.spv", {&DSLGubo, &DSLMesh});
		PMesh.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, true);
		POverlay.init(this, &VOverlay, "shaders/OverlayVert.spv", "shaders/OverlayFrag.spv", {&DSLOverlay});
		POverlay.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
		PNormMap.init(this, &VNorm, "shaders/NMVert.spv", "shaders/NMBlinnFrag.spv", {&DSLGubo, &DSLNormMap});
		PNormMap.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, false);
		PNayar.init(this, &VMesh, "shaders/MeshVert.spv", "shaders/NayarFrag.spv", {&DSLGubo, &DSLMesh});
		PBoundaries.init(this, &VMesh, "shaders/MeshVert.spv", "shaders/FragTransparent.spv", {&DSLGubo, &DSLMesh});
		PBoundaries.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, true);

		MCharacter.init(this,   &VMesh, "Models/MainCharacter.obj", OBJ);
		
		createBoundariesMesh(MBoundaries.vertices, MBoundaries.indices);
		MBoundaries.initMesh(this, &VMesh);

		MFloor.init(this, &VMesh, "Models/floor.obj", OBJ);
		MSphere.init(this, &VMesh, "Models/Sphere.obj", OBJ);
		
		MSphereGLTF.init(this, &VNorm, "Models/Sphere.gltf", GLTF);

		createImageIndexed(MStartGame.vertices, MStartGame.indices);
		MStartGame.initMesh(this, &VOverlay);

		createImageIndexed(MGameOver.vertices, MGameOver.indices);
		MGameOver.initMesh(this, &VOverlay);

		TCharacter.init(this,   "textures/red_Base_Color.png");
		TFloor.init(this, "textures/floor.jpg");
		TBoundaries.init(this, "textures/boundaries.png");
		TSphere1.init(this, "textures/alien/alien.png");
		TSphere1N.init(this, "textures/alien/alien-panels_normal-ogl.png", VK_FORMAT_R8G8B8A8_UNORM);
		TSphere1M.init(this, "textures/alien/alien_MRAO.png", VK_FORMAT_R8G8B8A8_UNORM);
		TSphere2.init(this, "textures/opal.jpeg");
		TSphere3.init(this, "textures/dirt.jpg");
		TSphere4.init(this, "textures/StylizedWoodPlanks_01/StylizedWoodPlanks_01_basecolor.jpg");
		TSphere4N.init(this, "textures/StylizedWoodPlanks_01/StylizedWoodPlanks_01_normal.jpg", VK_FORMAT_R8G8B8A8_UNORM);
		TSphere4M.init(this, "textures/StylizedWoodPlanks_01/Wood_MRAO.png", VK_FORMAT_R8G8B8A8_UNORM);
		TStartGame.init(this, "textures/StartGame.jpeg");
		TGameOver.init(this, "textures/GameOver.png");
		
		// Init local variables
		CamH = 1.0f;
		CamRadius = 3.0f;
		CamPitch = glm::radians(15.f);
		CamYaw = glm::radians(30.f);
		gameState = 0;
	}
	
	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders
		PMesh.create();
		POverlay.create();
		PNormMap.create();
		PNayar.create();
		PBoundaries.create();
		// Here you define the data set
		DSCharacter.init(this, &DSLMesh, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
					{1, TEXTURE, 0, &TCharacter}
				});

		DSSphere1.init(this, &DSLNormMap, {
			{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
			{1, TEXTURE, 0, &TSphere1},
			{2, TEXTURE, 0, &TSphere1N},
			{3, TEXTURE, 0, &TSphere1M}
		});

		DSSphere2.init(this, &DSLMesh, {
			{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
			{1, TEXTURE, 0, &TSphere2}
		});
		DSSphere3.init(this, &DSLMesh, {
			{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
			{1, TEXTURE, 0, &TSphere3}
		});
		DSSphere4.init(this, &DSLNormMap, {
			{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
			{1, TEXTURE, 0, &TSphere4},
			{2, TEXTURE, 0, &TSphere4N},
			{3, TEXTURE, 0, &TSphere4M}
		});
		DSFloor.init(this, &DSLMesh, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
					{1, TEXTURE, 0, &TFloor}
				});
		DSBoundaries.init(this, &DSLMesh, {
				  {0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
				  {1, TEXTURE, 0, &TBoundaries}
			  });
		DSGubo.init(this, &DSLGubo, {
					{0, UNIFORM, sizeof(GlobalUniformBlock), nullptr}
				});
		DSStartGame.init(this, &DSLMesh, {
					{0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
					{1, TEXTURE, 0, &TStartGame}
				});
		DSGameOver.init(this, &DSLMesh, {
					{0, UNIFORM, sizeof(OverlayUniformBlock), nullptr},
					{1, TEXTURE, 0, &TGameOver}
				});

	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines
		PMesh.cleanup();
		POverlay.cleanup();
		PNormMap.cleanup();
		PNayar.cleanup();
		PBoundaries.cleanup();
		// Cleanup datasets
		DSCharacter.cleanup();
		DSSphere1.cleanup();
		DSSphereS.cleanup();
		DSSphere2.cleanup();
		DSSphere3.cleanup();
		DSSphere4.cleanup();
		DSFloor.cleanup();
		DSBoundaries.cleanup();
		DSBall.cleanup();

		DSGubo.cleanup();
		DSStartGame.cleanup();
		DSGameOver.cleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() {
		// Cleanup textures
		TCharacter.cleanup();
		TFloor.cleanup();
		TBoundaries.cleanup();
		TSphere1.cleanup();
		TSphere1N.cleanup();
		TSphere1M.cleanup();
		TSphere4N.cleanup();
		TSphere4M.cleanup();

		TSphere2.cleanup();
		TSphere3.cleanup();
		TSphere4.cleanup();
		TStartGame.cleanup();
		TGameOver.cleanup();

		
		// Cleanup models
		MCharacter.cleanup();
		MFloor.cleanup();
		MGameOver.cleanup();
		MSplash.cleanup();
		MSphere.cleanup();
		MStartGame.cleanup();
		MGameOver.cleanup();
		MSphereGLTF.cleanup();
		MBoundaries.cleanup();
		
		DSLMesh.cleanup();
		DSLOverlay.cleanup();
		DSLNormMap.cleanup();
		DSLGubo.cleanup();
		
		// Destroies the pipelines
		PBoundaries.destroy();
		PMesh.destroy();		
		POverlay.destroy();
		PNormMap.destroy();
		PNayar.destroy();
	}
	
	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		// sets global uniforms (see below fro parameters explanation)
		DSGubo.bind(commandBuffer, PMesh, 0, currentImage);

		PMesh.bind(commandBuffer);
		MCharacter.bind(commandBuffer);
		DSCharacter.bind(commandBuffer, PMesh, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MCharacter.indices.size()), 1, 0, 0, 0);
		
		PNormMap.bind(commandBuffer);
		MSphereGLTF.bind(commandBuffer);
		DSSphere1.bind(commandBuffer, PNormMap, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MSphereGLTF.indices.size()), WAVE_SIZE, 0, 0, 0);
		DSSphere4.bind(commandBuffer, PNormMap, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MSphereGLTF.indices.size()), WAVE_SIZE, 0, 0, 0);
		
		PMesh.bind(commandBuffer);
		MSphere.bind(commandBuffer);
		DSSphere2.bind(commandBuffer, PMesh, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MSphere.indices.size()), WAVE_SIZE, 0, 0, 0);
		
		PNayar.bind(commandBuffer);
		DSSphere3.bind(commandBuffer, PNayar, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MSphere.indices.size()), WAVE_SIZE, 0, 0, 0);
		
		
		PMesh.bind(commandBuffer);
		MFloor.bind(commandBuffer);
		DSFloor.bind(commandBuffer, PMesh, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MFloor.indices.size()), 1, 0, 0, 0);

		PBoundaries.bind(commandBuffer);
		MBoundaries.bind(commandBuffer);
		DSBoundaries.bind(commandBuffer, PBoundaries, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MBoundaries.indices.size()), 1, 0, 0, 0);

		POverlay.bind(commandBuffer);
		MStartGame.bind(commandBuffer);
		DSStartGame.bind(commandBuffer, POverlay, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MStartGame.indices.size()), 1, 0, 0, 0);
		
		POverlay.bind(commandBuffer);
		MGameOver.bind(commandBuffer);
		DSGameOver.bind(commandBuffer, POverlay, 0, currentImage);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(MGameOver.indices.size()), 1, 0, 0, 0);
		
	}

	void initialiseCounters() {
		meshCounters.alien = 0;
		meshCounters.opal = 0;
		meshCounters.knit = 0;
		meshCounters.shatter = 0;
	}
	void clearUbo(MeshUniformBlock* ubo, int from) {
		for (int i = from; i < WAVE_SIZE; i++) {
			glm::mat4 nullMatrix = glm::mat4(0.0f);
			ubo->mvpMat[i] = nullMatrix;
			ubo->mMat[i] = nullMatrix;
			ubo->nMat[i] = glm::inverse(glm::transpose(nullMatrix));
		}
	}
	void updateUbo(MeshUniformBlock* ubo, int counter, glm::mat4 objectWorldMatrix, glm::mat4 Prj, glm::mat4 View) {
		ubo->amb = 1.0f; ubo->gamma = 180.0f; ubo->sColor = glm::vec3(1.0f);
		ubo->mvpMat[counter] = Prj * View * objectWorldMatrix;
		ubo->mMat[counter] = objectWorldMatrix;
		ubo->nMat[counter] = glm::inverse(glm::transpose(objectWorldMatrix));
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {

		// Standard procedure to quit when the ESC key is pressed
		if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		// Integration with the timers and the controllers
		static bool timeWarp = 0;
		float deltaT;
		float deltaT2;
		float warpTime = 0;
		static bool once = 1;
		glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
		bool fire = false;
		getSixAxis(deltaT, m, r, fire);
		deltaT2 = deltaT;

		if(glfwGetKey(window, GLFW_KEY_Q) && once) {
			timeWarp = 1;
			once = 1;
		}
		
		// getSixAxis() is defined in Starter.hpp in the base class.
		// It fills the float point variable passed in its first parameter with the time
		// since the last call to the procedure.
		// It fills vec3 in the second parameters, with three values in the -1,1 range corresponding
		// to motion (with left stick of the gamepad, or ASWD + RF keys on the keyboard)
		// It fills vec3 in the third parameters, with three values in the -1,1 range corresponding
		// to motion (with right stick of the gamepad, or Arrow keys + QE keys on the keyboard, or mouse)
		// If fills the last boolean variable with true if fire has been pressed:
		//          SPACE on the keyboard, A or B button on the Gamepad, Right mouse button

		static auto start = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		static auto finalTime = start;
		gameTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - start).count(); /*TIMER IN SECONDI*/
		float spawnTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - finalTime).count();

		if (timeWarp) {
			static auto startWarpTimeCD = std::chrono::high_resolution_clock::now();
			warpTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startWarpTimeCD).count();
			deltaT2 = deltaT2 / 10;
			spawnTime = spawnTime / 10;
		}

		if (warpTime > 5.0f){
			timeWarp = 0;
		}

		static glm::vec3 minArea = glm::vec3(-10.0f, 0.0f, -10.0f);
		static glm::vec3 maxArea = glm::vec3(10.0f, 0.0f, 10.0f);

		const glm::vec3 StartingPosition = glm::vec3(0.0, 0.0, 0.0);
		static Wave wave = Wave(15, 1.0f, minArea, maxArea);

		std::list<Ball>::iterator tempBall;
		
		static glm::vec3 Pos = StartingPosition;
		static glm::vec3 newPos;
		static glm::vec3 oldPos = Pos;

		float tolerance = 0.05f; // Tolerance value for comparison

		glm::mat4 WorldMatrix;

		static float yaw = 0.0f;
		static float pitch = glm::radians(-65.0f);
		static float roll = 0.0f;
		static float yaw2 = 0.0f;
		// static variables for current angles
		float constant = gameTime / 600; //angle of spotlight
		// Parameters
		// Camera FOV-y, Near Plane and Far Plane
		const float FOVy = glm::radians(45.0f);
		const float nearPlane = 0.1f;
		const float farPlane = 100.0f;

		const float ROT_SPEED = glm::radians(90.0f);
		const float MOVE_SPEED = 5.0f;

		float correctionAngle = glm::radians(90.0f) + yaw;
		if (m.x == 0 && m.z > 0) yaw2 = glm::radians(90.0f) + correctionAngle;
		if (m.x == 0 && m.z < 0) yaw2 = glm::radians(-90.0f) + correctionAngle;
		if (m.x < 0 && m.z >= 0) yaw2 = atan(m.z / m.x) + glm::radians(180.0f) + correctionAngle;
		if (m.x < 0 && m.z < 0) yaw2 = atan(m.z / m.x) - glm::radians(180.0f) + correctionAngle;
		if (m.x > 0) yaw2 = atan(m.z / m.x) + correctionAngle;

		const float CamH = 2.4;
		const float CamD = 7.5;
		pitch += ROT_SPEED * r.x * deltaT / 4;
		//if (pitch <= minPitch) pitch = minPitch;
		//if (pitch >= maxPitch) pitch = maxPitch;
		//roll += ROT_SPEED * r.z * deltaT;
		glm::vec3 ux = glm::vec3(glm::rotate(glm::mat4(1), yaw, glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1));
		glm::vec3 uy = glm::vec3(0, 1, 0);
		glm::vec3 uz = glm::vec3(glm::rotate(glm::mat4(1), yaw, glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1));

		Pos += ux * MOVE_SPEED * m.x * deltaT;
		Pos += uy * MOVE_SPEED * m.y * deltaT;
		Pos += uz * MOVE_SPEED * m.z * deltaT;

		//constraints check
		if (Pos.y <= 0) Pos.y = 0;
		if (Pos.y > 0 && m.y == 0) Pos.y -= 0.01;
		if (Pos.x > 5.0f) Pos.x = 5.0f;
		if (Pos.x < -5.0f) Pos.x = -5.0f;
		if (Pos.z > 5.0f) Pos.z = 5.0f;
		if (Pos.z < -5.0f) Pos.z = -5.0f;
		
		std::list<Ball>::iterator currentBall;
		static glm::mat4 tempWorldMatrix = glm::mat4(1);
		static glm::vec3 tempPos = glm::vec3(1);
		float lamba = 10.0f;

		switch(gameState) {
			case 0: {
				if(glfwGetKey(window, GLFW_KEY_ENTER)) {
					gameState = 1;
				}

				break;
			}
			case 1: {
				if (spawnTime >= 1.0f || !flaggswag) {
					glm::vec3 positionToTrack = Pos;
					wave.addBall(positionToTrack);
					finalTime = currentTime;
					flaggswag = true;
				}
				newPos = (oldPos * exp(-lamba * deltaT)) + Pos * (1 - exp(-lamba * deltaT));
				oldPos = newPos;

				WorldMatrix = glm::translate(glm::mat4(1.0), Pos) * glm::rotate(glm::mat4(1.0), yaw2, glm::vec3(0, 1, 0)) * glm::rotate(glm::mat4(1.0), roll, glm::vec3(0, 0, 1)) * glm::scale(glm::mat4(1.0), glm::vec3(1, 1, 1));

				glm::mat4 WorldMatrixNew = glm::translate(glm::mat4(1.0), newPos) * glm::rotate(glm::mat4(1.0), yaw, glm::vec3(0, 1, 0)) * glm::rotate(glm::mat4(1.0), pitch, glm::vec3(1, 0, 0)) * glm::scale(glm::mat4(1.0), glm::vec3(1, 1, 1));

				glm::mat4 Prj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
				Prj[1][1] *= -1;

				glm::vec3 camTarget = glm::vec3(WorldMatrixNew * glm::vec4(0, 0, 0, 1)) + glm::vec3(0, CamH, 0);
				glm::vec3 camPos = WorldMatrixNew * glm::vec4(0, CamH + CamD * sin(glm::radians(pitch)), CamD * cos(glm::radians(pitch)), 1);
				glm::mat4 View = glm::lookAt(camPos, camTarget, glm::vec3(0, 1, 0));

				gubo.DlightDir = glm::normalize((Pos + glm::vec3(0, 5, 0)) - Pos);
				gubo.DlightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				if (gameTime > 110 && (int)(gameTime-110)%2 == 0) {
					gubo.DlightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				}
				if (gameTime > 110 && (int)(gameTime-110)%2 == 1) {
					gubo.DlightColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
				}
				gubo.eyePos = camPos;
				gubo.lightPos = Pos + glm::vec3(0,6, 0.0f);
			
				gubo.cosout = 0.75f + constant;
				gubo.cosin = 0.80 + constant;
				
				DSGubo.map(currentImage, &gubo, sizeof(gubo), 0);

				uboCharacter.amb = 1.0f; uboCharacter.gamma = 180.0f; uboCharacter.sColor = glm::vec3(1.0f);
				uboCharacter.mvpMat[0] = Prj * View * WorldMatrix;
				uboCharacter.mMat[0] = WorldMatrix;
				uboCharacter.nMat[0] = glm::inverse(glm::transpose(WorldMatrix));
				DSCharacter.map(currentImage, &uboCharacter, sizeof(uboCharacter), 0);
	
				for(currentBall = wave.balls.begin(); currentBall != wave.balls.end(); currentBall++) {
					currentBall->updatePosition(deltaT2);
					if (glm::distance(currentBall->position - glm::vec3(0.0f, currentBall->size, 0.0f), Pos) <= currentBall->size || gameTime > 120.0f) {
						gameState = 2;
					}
				}
				wave.removeOutOfBoundBalls();
				initialiseCounters();
				if (wave.balls.size() != 0){
					for (currentBall = wave.balls.begin(); currentBall != wave.balls.end(); currentBall++) {
						switch (currentBall->type) {
							case alien: {
								updateUbo(&uboSphere1, meshCounters.alien, currentBall->getWorldMatrix(), Prj, View);
								meshCounters.alien++;
								break;
							}
							case opal: {
								updateUbo(&uboSphere2, meshCounters.opal, currentBall->getWorldMatrix(), Prj, View);
								meshCounters.opal++;
								break;
							}
							case knit: {
								updateUbo(&uboSphere3, meshCounters.knit, currentBall->getWorldMatrix(), Prj, View);
								meshCounters.knit++;
								break;
							}
							case shatter: {
								updateUbo(&uboSphere4, meshCounters.shatter, currentBall->getWorldMatrix(), Prj, View);
								meshCounters.shatter++;
								break;
							}
						}

					}
					clearUbo(&uboSphere1, meshCounters.alien);
					clearUbo(&uboSphere2, meshCounters.opal);
					clearUbo(&uboSphere3, meshCounters.knit);
					clearUbo(&uboSphere4, meshCounters.shatter);
				} else {
					clearUbo(&uboSphere1, 0);
					clearUbo(&uboSphere2, 0);
					clearUbo(&uboSphere3, 0);
					clearUbo(&uboSphere4, 0);
				}

				uboSphereS.amb = 1.0f; uboSphereS.gamma = 180.0f; uboSphereS.sColor = glm::vec3(1.0f);
				tempPos += glm::vec3(0.1f*deltaT,0,0);
				tempWorldMatrix =  glm::translate(glm::mat4(1.0), tempPos);
				uboSphereS.mvpMat[0] = Prj * View * tempWorldMatrix;
				uboSphereS.mMat[0] = tempWorldMatrix;
				uboSphereS.nMat[0] = glm::inverse(glm::transpose(tempWorldMatrix));
				DSSphere1.map(currentImage, &uboSphere1, sizeof(uboSphere1), 0);
				DSSphere2.map(currentImage, &uboSphere2, sizeof(uboSphere2), 0);
				DSSphere3.map(currentImage, &uboSphere3, sizeof(uboSphere3), 0);
				DSSphere4.map(currentImage, &uboSphere4, sizeof(uboSphere4), 0);
				
				glm::mat4 World = glm::mat4(1);
				uboBoundaries.mvpMat[0] = Prj * View * World;
				uboBoundaries.mMat[0] = World;
				uboBoundaries.nMat[0] = glm::inverse(glm::transpose(World));
				DSBoundaries.map(currentImage, &uboBoundaries, sizeof(uboBoundaries), 0);
			
				World = glm::mat4(1);
				World = World * glm::translate(glm::mat4(1), glm::vec3(-20, 0, -20)) * glm::scale(glm::mat4(1), glm::vec3(50, 1, 50));
				uboFloor.amb = 1.0f; uboFloor.gamma = 180.0f; uboFloor.sColor = glm::vec3(1.0f);
				uboFloor.mvpMat[0] = Prj * View * World;
				uboFloor.mMat[0] = World;
				uboFloor.nMat[0] = glm::inverse(glm::transpose(World));
				DSFloor.map(currentImage, &uboFloor, sizeof(uboFloor), 0);
				
				break;
			}
		}

		uboStartGame.visible = (gameState == 0) ? 1.0f : 0.0f;
		DSStartGame.map(currentImage, &uboStartGame, sizeof(uboStartGame), 0);

		uboGameOver.visible = (gameState == 2) ? 1.0f : 0.0f;
		DSGameOver.map(currentImage, &uboGameOver, sizeof(uboGameOver), 0);
	}	
};

// This is the main: probably you do not need to touch this!
int main() {
    SlotMachine app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
