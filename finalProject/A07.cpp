// This has been adapted from the Vulkan tutorial

#include "Starter.hpp"

//xcode

// The uniform buffer object used in this example
struct UniformBufferObject {
	alignas(16) glm::mat4 mvpMat;
	alignas(16) glm::mat4 mMat;
	alignas(16) glm::mat4 nMat;
};

struct GlobalUniformBufferObject {
	alignas(16) glm::vec3 lightDir;
	alignas(16) glm::vec4 lightColor;
	alignas(16) glm::vec3 eyePos;
};

class Assignment07;
void GameLogic(Assignment07 *A, float Ar, glm::mat4 &ViewPrj, glm::mat4 &World);

// MAIN ! 
class Assignment07 : public BaseProject {
	protected:
	// Here you list all the Vulkan objects you need:
	
	// Descriptor Layouts [what will be passed to the shaders]
	DescriptorSetLayout DSL1;

	// Pipelines [Shader couples]
	Pipeline P1;

	// Models, textures and Descriptors (values assigned to the uniforms)
	Model M1, MG;
	Texture T1, TG[4];
	DescriptorSet DS1, DSG[4];
	
	// Other application parameters
	float Ar;
	glm::mat4 GWM[4];

	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "Escape Baloons";
    	windowResizable = GLFW_TRUE;
		initialBackgroundColor = {0.0f, 0.6f, 0.8f, 1.0f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 13;
		texturesInPool = 7;
		setsInPool = 7;
		
		Ar = 4.0f / 3.0f;
	}
	
	// What to do when the window changes size
	void onWindowResize(int w, int h) {
		std::cout << "Window resized to: " << w << " x " << h << "\n";
		Ar = (float)w / (float)h;
	}
	
	// Here you load and setup all your Vulkan Models and Texutures.
	// Here you also create your Descriptor set layouts and load the shaders for the pipelines
	void localInit() {
		// Descriptor Layouts [what will be passed to the shaders]
		DSL1.init(this, {
					// this array contains the binding:
					// first  element : the binding number
					// second element : the type of element (buffer or texture)
					// third  element : the pipeline stage where it will be used
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT},
					{2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
				  });

		// Pipelines [Shader couples]
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		P1.init(this, "shaders/PhongVert.spv", "shaders/PhongFrag.spv", {&DSL1});
		P1.setAdvancedFeatures(VK_COMPARE_OP_LESS, VK_POLYGON_MODE_FILL,
 								    VK_CULL_MODE_NONE, false);

		// Models, textures and Descriptors (values assigned to the uniforms)
		M1.init(this, "models/Character.obj");
		MG.init(this, "models/floor.obj");
		
		T1.init(this, "textures/Colors2.png");
		TG[0].init(this, "textures/u5chunkedMap-0.png");
		TG[1].init(this, "textures/u5chunkedMap-1.png");
		TG[2].init(this, "textures/u5chunkedMap-2.png");
		TG[3].init(this, "textures/u5chunkedMap-3.png");
		

		GWM[0] = glm::translate(glm::scale(glm::mat4(1),glm::vec3(128)),glm::vec3(-1,0,-1));	
		GWM[1] = glm::translate(glm::scale(glm::mat4(1),glm::vec3(128)),glm::vec3(0,0,-1));	
		GWM[2] = glm::translate(glm::scale(glm::mat4(1),glm::vec3(128)),glm::vec3(-1,0,0));	
		GWM[3] = glm::translate(glm::scale(glm::mat4(1),glm::vec3(128)),glm::vec3(0,0,0));	
	}
	
	// Here you create your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsInit() {
		// This creates a new pipeline (with the current surface), using its shaders
		P1.create();

		DS1.init(this, &DSL1, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &T1},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
				});
		for(int i = 0; i < 4; i++) {
			DSG[i].init(this, &DSL1, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &TG[i]},
					{2, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
				});
		}
	}

	// Here you destroy your pipelines and Descriptor Sets!
	void pipelinesAndDescriptorSetsCleanup() {
		P1.cleanup();
		
		DS1.cleanup();
		DSG[0].cleanup();
		DSG[1].cleanup();
		DSG[2].cleanup();
		DSG[3].cleanup();
	}

	// Here you destroy all the Models, Texture and Desc. Set Layouts you created!
	// You also have to destroy the pipelines
	void localCleanup() {
		T1.cleanup();
		M1.cleanup();
		TG[0].cleanup();
		TG[1].cleanup();
		TG[2].cleanup();
		TG[3].cleanup();
		MG.cleanup();

		DSL1.cleanup();
		
		P1.destroy();		
	}
	
	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {

		P1.bind(commandBuffer);
		M1.bind(commandBuffer);
		DS1.bind(commandBuffer, P1, currentImage);
					
		vkCmdDrawIndexed(commandBuffer,
				static_cast<uint32_t>(M1.indices.size()), 1, 0, 0, 0);
		
		MG.bind(commandBuffer);
		for(int i = 0; i < 4; i++) {
			DSG[i].bind(commandBuffer, P1, currentImage);
						
			vkCmdDrawIndexed(commandBuffer,
					static_cast<uint32_t>(MG.indices.size()), 1, 0, 0, 0);
		}
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		glm::mat4 ViewPrj;
		glm::mat4 WM;
		
		GameLogic(this, Ar, ViewPrj, WM);
		

		UniformBufferObject ubo{};								
		// Here is where you actually update your uniforms

		// updates global uniforms
		GlobalUniformBufferObject gubo{};
		gubo.lightDir = glm::vec3(cos(glm::radians(135.0f)), sin(glm::radians(135.0f)), 0.0f);
		gubo.lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		gubo.eyePos = glm::vec3(100.0, 100.0, 100.0);

		ubo.mMat = WM * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0,1,0));
		ubo.mvpMat = ViewPrj * ubo.mMat;
		ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
		DS1.map(currentImage, &ubo, sizeof(ubo), 0);
		DS1.map(currentImage, &gubo, sizeof(gubo), 2);

		for(int i = 0; i < 4; i++) {
			ubo.mMat = GWM[i];
			ubo.mvpMat = ViewPrj * ubo.mMat;
			ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
			DSG[i].map(currentImage, &ubo, sizeof(ubo), 0);
			DSG[i].map(currentImage, &gubo, sizeof(gubo), 2);
		}
	}
	
};

#include "Logic.hpp"

// This is the main: probably you do not need to touch this!
int main() {
    Assignment07 app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
