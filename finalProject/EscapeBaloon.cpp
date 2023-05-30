// This has been adapted from the Vulkan tutorial

#include "Starter.hpp"

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
	alignas(16) glm::mat4 mvpMat[20];
	alignas(16) glm::mat4 mMat[20];
	alignas(16) glm::mat4 nMat;
};

struct OverlayUniformBlock {
	alignas(4) float visible;
};

struct GlobalUniformBlock {
	alignas(16) glm::vec3 DlightDir;
	alignas(16) glm::vec3 DlightColor;
	alignas(16) glm::vec3 AmbLightColor;
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



void GameLogic(float deltaT, float Ar, glm::mat4 &ViewPrj, glm::mat4 &World);
// MAIN ! 
class SlotMachine : public BaseProject {
	protected:

	// Current aspect ratio (used by the callback that resized the window
	float Ar;

	// Descriptor Layouts ["classes" of what will be passed to the shaders]
	DescriptorSetLayout DSLGubo, DSLMesh, DSLOverlay;

	// Vertex formats
	VertexDescriptor VMesh;
	VertexDescriptor VOverlay;

	// Pipelines [Shader couples]
	Pipeline PMesh;
	Pipeline POverlay;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex structure
	Model<VertexMesh> MCharacter, MFloor;
	Model<VertexOverlay> MKey, MSplash;
	DescriptorSet DSGubo, DSCharacter, DSFloor;
	Texture TCharacter, TFloor;
	
	// C++ storage for uniform variables
	MeshUniformBlock uboCharacter, uboFloor;
	GlobalUniformBlock gubo;
	OverlayUniformBlock uboKey, uboSplash;

	// Other application parameters
	float CamH, CamRadius, CamPitch, CamYaw;
	int gameState;

	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "Slot Machine";
    	windowResizable = GLFW_TRUE;
		initialBackgroundColor = {0.0f, 0.005f, 0.01f, 1.0f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 8;
		texturesInPool = 7;
		setsInPool = 8;
		
		Ar = (float)windowWidth / (float)windowHeight;
	}
	
	// What to do when the window changes size
	void onWindowResize(int w, int h) {
		Ar = (float)w / (float)h;
	}
	
	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	void localInit() {
		// Descriptor Layouts [what will be passed to the shaders]
		DSLMesh.init(this, {
					// this array contains the bindings:
					// first  element : the binding number
					// second element : the type of element (buffer or texture)
					//                  using the corresponding Vulkan constant
					// third  element : the pipeline stage where it will be used
					//                  using the corresponding Vulkan constant
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

		// Vertex descriptors
		VMesh.init(this, {
				  // this array contains the bindings
				  // first  element : the binding number
				  // second element : the stride of this binging
				  // third  element : whether this parameter change per vertex or per instance
				  //                  using the corresponding Vulkan constant
				  {0, sizeof(VertexMesh), VK_VERTEX_INPUT_RATE_VERTEX}
				}, {
				  // this array contains the location
				  // first  element : the binding number
				  // second element : the location number
				  // third  element : the offset of this element in the memory record
				  // fourth element : the data type of the element
				  //                  using the corresponding Vulkan constant
				  // fifth  elmenet : the size in byte of the element
				  // sixth  element : a constant defining the element usage
				  //                   POSITION - a vec3 with the position
				  //                   NORMAL   - a vec3 with the normal vector
				  //                   UV       - a vec2 with a UV coordinate
				  //                   COLOR    - a vec4 with a RGBA color
				  //                   TANGENT  - a vec4 with the tangent vector
				  //                   OTHER    - anything else
				  //
				  // ***************** DOUBLE CHECK ********************
				  //    That the Vertex data structure you use in the "offsetoff" and
				  //	in the "sizeof" in the previous array, refers to the correct one,
				  //	if you have more than one vertex format!
				  // ***************************************************
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

		// Pipelines [Shader couples]
		// The second parameter is the pointer to the vertex definition
		// Third and fourth parameters are respectively the vertex and fragment shaders
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		PMesh.init(this, &VMesh, "shaders/MeshVert.spv", "shaders/MeshFrag.spv", {&DSLGubo, &DSLMesh});
		POverlay.init(this, &VOverlay, "shaders/OverlayVert.spv", "shaders/OverlayFrag.spv", {&DSLOverlay});
		POverlay.setAdvancedFeatures(VK_COMPARE_OP_LESS_OR_EQUAL, VK_POLYGON_MODE_FILL,
 								    VK_CULL_MODE_NONE, false);

		// Models, textures and Descriptors (values assigned to the uniforms)

		// Create models
		// The second parameter is the pointer to the vertex definition for this model
		// The third parameter is the file name
		// The last is a constant specifying the file type: currently only OBJ or GLTF
		MCharacter.init(this,   &VMesh, "Models/MainCharacter.obj", OBJ);

		// Creates a mesh with direct enumeration of vertices and indices
		MFloor.init(this, &VMesh, "Models/floor.obj", OBJ);
		
		// Create the textures
		// The second parameter is the file name
		TCharacter.init(this,   "textures/red_Base_Color.png");
		TFloor.init(this, "textures/floor.jpg");
		
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
		
		// Here you define the data set
		DSCharacter.init(this, &DSLMesh, {
		// the second parameter, is a pointer to the Uniform Set Layout of this set
		// the last parameter is an array, with one element per binding of the set.
		// first  elmenet : the binding number
		// second element : UNIFORM or TEXTURE (an enum) depending on the type
		// third  element : only for UNIFORMs, the size of the corresponding C++ object. For texture, just put 0
		// fourth element : only for TEXTUREs, the pointer to the corresponding texture object. For uniforms, use nullptr
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
					{1, TEXTURE, 0, &TCharacter}
				});
		DSFloor.init(this, &DSLMesh, {
					{0, UNIFORM, sizeof(MeshUniformBlock), nullptr},
					{1, TEXTURE, 0, &TFloor}
				});
		DSGubo.init(this, &DSLGubo, {
					{0, UNIFORM, sizeof(GlobalUniformBlock), nullptr}
				});
	}

	// Here you destroy your pipelines and Descriptor Sets!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	void pipelinesAndDescriptorSetsCleanup() {
		// Cleanup pipelines
		PMesh.cleanup();
		POverlay.cleanup();

		// Cleanup datasets
		DSCharacter.cleanup();
		DSFloor.cleanup();

		DSGubo.cleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// All the object classes defined in Starter.hpp have a method .cleanup() for this purpose
	// You also have to destroy the pipelines: since they need to be rebuilt, they have two different
	// methods: .cleanup() recreates them, while .destroy() delete them completely
	void localCleanup() {
		// Cleanup textures
		TCharacter.cleanup();
		TFloor.cleanup();

		
		// Cleanup models
		MCharacter.cleanup();
		MFloor.cleanup();
		MKey.cleanup();
		MSplash.cleanup();
		
		// Cleanup descriptor set layouts
		DSLMesh.cleanup();
		DSLOverlay.cleanup();

		DSLGubo.cleanup();
		
		// Destroies the pipelines
		PMesh.destroy();		
		POverlay.destroy();
	}
	
	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
		// sets global uniforms (see below fro parameters explanation)
		DSGubo.bind(commandBuffer, PMesh, 0, currentImage);

		// binds the pipeline
		PMesh.bind(commandBuffer);
		// For a pipeline object, this command binds the corresponing pipeline to the command buffer passed in its parameter

		// binds the model
		MCharacter.bind(commandBuffer);
		// For a Model object, this command binds the corresponing index and vertex buffer
		// to the command buffer passed in its parameter
		
		// binds the data set
		DSCharacter.bind(commandBuffer, PMesh, 1, currentImage);
		// For a Dataset object, this command binds the corresponing dataset
		// to the command buffer and pipeline passed in its first and second parameters.
		// The third parameter is the number of the set being bound
		// As described in the Vulkan tutorial, a different dataset is required for each image in the swap chain.
		// This is done automatically in file Starter.hpp, however the command here needs also the index
		// of the current image in the swap chain, passed in its last parameter
					
		// record the drawing command in the command buffer
		vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MCharacter.indices.size()), 1, 0, 0, 0);
		// the second parameter is the number of indexes to be drawn. For a Model object,
		// this can be retrieved with the .indices.size() method.

		MFloor.bind(commandBuffer);
		DSFloor.bind(commandBuffer, PMesh, 1, currentImage);
		vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(MFloor.indices.size()), 1, 0, 0, 0);

	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		// Standard procedure to quit when the ESC key is pressed
		if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		// Integration with the timers and the controllers
		float deltaT;
		glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
		bool fire = false;
		getSixAxis(deltaT, m, r, fire);
		// getSixAxis() is defined in Starter.hpp in the base class.
		// It fills the float point variable passed in its first parameter with the time
		// since the last call to the procedure.
		// It fills vec3 in the second parameters, with three values in the -1,1 range corresponding
		// to motion (with left stick of the gamepad, or ASWD + RF keys on the keyboard)
		// It fills vec3 in the third parameters, with three values in the -1,1 range corresponding
		// to motion (with right stick of the gamepad, or Arrow keys + QE keys on the keyboard, or mouse)
		// If fills the last boolean variable with true if fire has been pressed:
		//          SPACE on the keyboard, A or B button on the Gamepad, Right mouse button

		// To debounce the pressing of the fire button, and start the event when the key is released
		const glm::vec3 StartingPosition = glm::vec3(0.0, 0.0, 0.0);
		// Parameters: wheels and handle speed and range
		static glm::vec3 Pos = StartingPosition;
		static glm::vec3 newPos;
		static glm::vec3 oldPos = Pos;

		glm::mat4 WorldMatrix;

		static float yaw = 0.0f;
		static float pitch = glm::radians(-45.0f);
		static float roll = 0.0f;
		static float yaw2 = 0.0f;
		// static variables for current angles
		

		
		
		// Parameters
		// Camera FOV-y, Near Plane and Far Plane
		const float FOVy = glm::radians(45.0f);
		const float nearPlane = 0.1f;
		const float farPlane = 100.0f;

		const float ROT_SPEED = glm::radians(90.0f);
		const float MOVE_SPEED = 5.0f;

		float correctionAngle = glm::radians(90.0f) +yaw;
		if (m.x == 0 && m.z > 0) yaw2 = glm::radians(90.0f) + correctionAngle;
		if (m.x == 0 && m.z < 0) yaw2 = glm::radians(-90.0f) + correctionAngle;
		if (m.x < 0 && m.z >= 0) yaw2 = atan(m.z/m.x)+glm::radians(180.0f) + correctionAngle;
		if (m.x < 0 && m.z < 0) yaw2 = atan(m.z/m.x)-glm::radians(180.0f) + correctionAngle;
		if (m.x > 0) yaw2 = atan(m.z / m.x) + correctionAngle;
		
		const float CamH = 2.4;
		const float CamD = 7.5;
		pitch += ROT_SPEED * r.x * deltaT/4;
		//if (pitch <= minPitch) pitch = minPitch;
		//if (pitch >= maxPitch) pitch = maxPitch;
		//roll += ROT_SPEED * r.z * deltaT;
		glm::vec3 ux = glm::vec3(glm::rotate(glm::mat4(1), yaw,
		glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1));
		glm::vec3 uy = glm::vec3(0,1,0);
		glm::vec3 uz = glm::vec3(glm::rotate(glm::mat4(1), yaw,
			glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1));

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

		float lamba = 10.0f;
		newPos = (oldPos * exp(-lamba * deltaT)) + Pos * (1 - exp(-lamba * deltaT));
		oldPos = newPos;

		WorldMatrix = glm::translate(glm::mat4(1.0), Pos) * glm::rotate(glm::mat4(1.0), yaw2, glm::vec3(0,1,0)) * glm::rotate(glm::mat4(1.0), roll, glm::vec3(0,0,1))* glm::scale(glm::mat4(1.0), glm::vec3(1,1,1));
		
		glm::mat4 WorldMatrixNew= glm::translate(glm::mat4(1.0), newPos) * glm::rotate(glm::mat4(1.0), yaw, glm::vec3(0,1,0))* glm::rotate(glm::mat4(1.0), pitch, glm::vec3(1,0,0)) * glm::scale(glm::mat4(1.0), glm::vec3(1,1,1));
		
		glm::mat4 Prj = glm::perspective(FOVy, Ar, nearPlane, farPlane);
		Prj[1][1] *= -1;

		glm::vec3 camTarget = glm::vec3( WorldMatrixNew * glm::vec4(0, 0, 0, 1)) + glm::vec3(0, CamH, 0);
		glm::vec3 camPos    = WorldMatrixNew * glm::vec4(0, CamH + CamD * sin(glm::radians(pitch)), CamD * cos(glm::radians(pitch)), 1);
		glm::mat4 View = glm::lookAt(camPos, camTarget, glm::vec3(0,1,0));

		


		gubo.DlightDir = glm::normalize(glm::vec3(1, 2, 3));
		gubo.DlightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		gubo.AmbLightColor = glm::vec3(0.1f);
		gubo.eyePos = camPos;

		// Writes value to the GPU
		DSGubo.map(currentImage, &gubo, sizeof(gubo), 0);
		// the .map() method of a DataSet object, requires the current image of the swap chain as first parameter
		// the second parameter is the pointer to the C++ data structure to transfer to the GPU
		// the third parameter is its size
		// the fourth parameter is the location inside the descriptor set of this uniform block

		uboCharacter.amb = 1.0f; uboCharacter.gamma = 180.0f; uboCharacter.sColor = glm::vec3(1.0f);
		uboCharacter.mvpMat[0] = Prj * View * WorldMatrix;
		uboCharacter.mMat[0] = WorldMatrix;
		uboCharacter.nMat = glm::inverse(glm::transpose(WorldMatrix));
		DSCharacter.map(currentImage, &uboCharacter, sizeof(uboCharacter), 0);

		glm::mat4 World = glm::mat4(1);
		World = World * glm::translate(glm::mat4(1), glm::vec3(-20,0,-20))*glm::scale(glm::mat4(1), glm::vec3(50, 1, 50));
		uboFloor.amb = 1.0f; uboFloor.gamma = 180.0f; uboFloor.sColor = glm::vec3(1.0f);
		uboFloor.mvpMat[0] = Prj * View * World;
		uboFloor.mMat[0] = World;
		
		DSFloor.map(currentImage, &uboFloor, sizeof(uboFloor), 0);
	
	

		//uboSplash.visible = (gameState == 0) ? 1.0f : 0.0f;
		//DSSplash.map(currentImage, &uboSplash, sizeof(uboSplash), 0);
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