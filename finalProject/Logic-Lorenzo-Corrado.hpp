

void GameLogic(Assignment07 *A, float Ar, glm::mat4 &ViewPrj, glm::mat4 &World) {
	// The procedure must implement the game logic  to move the character in third person.
	// Input:
	// <Assignment07 *A> Pointer to the current assignment code. Required to read the input from the user
	// <float Ar> Aspect ratio of the current window (for the Projection Matrix)
	// Output:
	// <glm::mat4 &ViewPrj> the view-projection matrix of the camera
	// <glm::mat4 &World> the world matrix of the object
	// Parameters
	// Camera FOV-y, Near Plane and Far Plane
	const float FOVy = glm::radians(45.0f);
	const float nearPlane = 0.1f;
	const float farPlane = 100.0f;
	
	// Player starting point
	const glm::vec3 StartingPosition = glm::vec3(-41.50, 0.0, -19.5);
	
	// Camera target height and distance
	const float camHeight = 0.4;
	const float camDist = 1.5;
	// Camera Pitch limits
	const float minPitch = glm::radians(-8.75f);

	const float maxPitch = glm::radians(10.0f);
	// Rotation and motion speed
	const float ROT_SPEED = glm::radians(120.0f);
	const float MOVE_SPEED = 2.0f;

	// Integration with the timers and the controllers
	// returns:
	// <float deltaT> the time passed since the last frame
	// <glm::vec3 m> the state of the motion axes of the controllers (-1 <= m.x, m.y, m.z <= 1)
	// <glm::vec3 r> the state of the rotation axes of the controllers (-1 <= r.x, r.y, r.z <= 1)
	// <bool fire> if the user has pressed a fire button (not required in this assginment)
	float deltaT;
	glm::vec3 m = glm::vec3(0.0f), r = glm::vec3(0.0f);
	bool fire = false;
	A->getSixAxis(deltaT, m, r, fire);
		
	// Game Logic implementation
	// Current Player Position - statc variables make sure thattheri value remain unchanged in subsequent calls to the procedure
	static glm::vec3 Pos = StartingPosition;
	static glm::vec3 newPos;
	static glm::vec3 oldPos = Pos;
	// To be done in the assignment
	glm::mat4 Mp = glm::perspective(FOVy, Ar, nearPlane, farPlane);
	Mp[1][1] *= -1;
	// The local coordinates model update proc. With quaternions
	glm::mat4 WorldMatrix;
	static float yaw = 0.0f;
	static float pitch = 0.0f;
	static float roll = 0.0f;
	static float yaw2 = 0.0f;

	glm::vec3 ux = glm::vec3(glm::rotate(glm::mat4(1), yaw,
		glm::vec3(0, 1, 0)) * glm::vec4(1, 0, 0, 1));
	glm::vec3 uy = glm::vec3(0,1,0);
	glm::vec3 uz = glm::vec3(glm::rotate(glm::mat4(1), yaw,
		glm::vec3(0, 1, 0)) * glm::vec4(0, 0, -1, 1));
	yaw += ROT_SPEED * r.y * deltaT;
	//rotation yaw world matrix 
	float correctionAngle = glm::radians(270.0f) + yaw;
	if (m.x == 0 && m.z > 0) yaw2 = glm::radians(90.0f) + correctionAngle;
	if (m.x == 0 && m.z < 0) yaw2 = glm::radians(-90.0f) + correctionAngle;
	if (m.x < 0 && m.z >= 0) yaw2 = atan(m.z/m.x)+glm::radians(180.0f) + correctionAngle;
	if (m.x < 0 && m.z < 0) yaw2 = atan(m.z/m.x)-glm::radians(180.0f) + correctionAngle;
	if (m.x > 0) yaw2 = atan(m.z / m.x) + correctionAngle;



	
	pitch += ROT_SPEED * r.x * deltaT;
	if (pitch <= minPitch) pitch = minPitch;
	if (pitch >= maxPitch) pitch = maxPitch;
	roll += ROT_SPEED * r.z * deltaT;

	Pos += ux * MOVE_SPEED * m.x * deltaT;

	Pos += uy * MOVE_SPEED * m.y * deltaT;
	if (Pos.y <= 0) Pos.y = 0;
	if (Pos.y > 0 && m.y == 0) Pos.y -= 0.01;

	Pos += uz * MOVE_SPEED * m.z * deltaT;

	float lamba = 10.0f;
	newPos = (oldPos * exp(-lamba * deltaT)) + Pos * (1 - exp(-lamba * deltaT));
	oldPos = newPos;

	WorldMatrix = glm::translate(glm::mat4(1.0), Pos) *
		glm::rotate(glm::mat4(1.0), yaw2, glm::vec3(0,1,0)) * 
		glm::rotate(glm::mat4(1.0), roll, glm::vec3(0,0,1))*
		glm::scale(glm::mat4(1.0), glm::vec3(1,1,1));

	
	glm::mat4 WorldMatrixNew= glm::translate(glm::mat4(1.0), newPos) *
		glm::rotate(glm::mat4(1.0), yaw, glm::vec3(0,1,0))* glm::rotate(glm::mat4(1.0), pitch, glm::vec3(1,0,0)) * 
		glm::scale(glm::mat4(1.0), glm::vec3(1,1,1));
	
	glm::vec3 u = glm::vec3(0, 1, 0);

	
	glm::vec3 c = WorldMatrixNew * glm::vec4(0, camHeight + camDist * sin(glm::radians(pitch)),
		camDist * cos(glm::radians(pitch)), 1);
	

	glm::vec3 a = glm::vec3( WorldMatrixNew * glm::vec4(0, 0, 0, 1)) + glm::vec3(0, camHeight, 0);
	
	glm::mat4 Mv =  glm::lookAt(c, a, u);

	ViewPrj = Mp*Mv;
	World = WorldMatrix;	
}