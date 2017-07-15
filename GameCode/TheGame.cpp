#include "TheGame.hpp"
#include "TheApplication.hpp"

#include "Engine\Renderer\OpenGLRenderer.hpp"
#include "Engine\Core\Time.hpp"
#include "Engine\Core\HenryFunctions.hpp"

#include <fstream>
#include <sstream>

#define UNUSED(x) (void)(x);


namespace Henry
{

extern TheApplication* g_theApplication;

TheGame::TheGame(void)
{
	m_isQuitting = false;
	SetGameState(INITIALIZE);
	Initialize();
}


TheGame::~TheGame(void)
{
	if(m_material)
	{
		delete m_material;
		m_material		= nullptr;
	}

	if(m_fboMaterial)
	{
		delete m_fboMaterial;
		m_fboMaterial	= nullptr;
	}
}


void TheGame::Input()
{
	if(m_input->PressedOnce(Inputs::INPUT_ESCAPE))
		SetGameState(QUITTING);
	
	UpdateCameraFromMouseAndKeyboard();

	switch(m_gameState)
	{
	case INITIALIZE:
		break;
	case SETTING:
		break;
	case GAMEPLAY:
	{
		if (m_input->m_keyState['1'].isPressedFrame)
		{
			m_aa.PlayAnimation("walk", 2.0);
		}

		if (m_input->m_keyState['2'].isPressedFrame)
		{
			m_aa.BlendToAnimation("attack", 3.0, 2.0, true, 1.0, false);
			m_aa.AddFilter("attack", "Bip01 Spine1");
		}

		if (m_input->m_keyState['3'].isPressedFrame)
		{
			m_aa.StopAnimation("walk");
		}

		if (m_input->m_keyState['4'].isPressedFrame)
		{
			m_aa.StopAnimation("attack", 1.0f);
		}

		if (m_input->m_keyState['5'].isPressedFrame)
		{
			m_aa.SimulateRagdoll();
		}

		if (m_input->m_keyState[Inputs::INPUT_TAB].isPressedFrame)
		{
			m_textureId = m_textureId == m_zombieTexture->m_textureID ? m_zombieDebugTexture->m_textureID : m_zombieTexture->m_textureID;
		}

		if (m_input->m_keyState['Q'].isPressedFrame)
		{
			m_drawBones = !m_drawBones;
		}

		if (m_input->m_keyState['R'].isPressedFrame)
		{
			m_drawModels = !m_drawModels;
		}

		if (m_input->m_keyState[Inputs::INPUT_ADD].isPressedFrame || m_input->m_keyState['E'].isHold)
		{
			AnimationActor aa;
			aa.AttachToSkeletalAnimationObject(m_animationSystem.GetSkeletalAnimationObject("zombie"));
			aa.PlayAnimation("walk", random(1.0f, 5.0f), true);
			if (random(0, 2))
			{
				aa.BlendToAnimation("attack", random(1.0f, 5.0f), random(1.0f, 3.0f), true, 1.0f, false);
				aa.AddFilter("attack", "Bip01 Spine1");
			}

			m_aaList.push_back(aa);
		}
	}
		break;
	};
}


void TheGame::Render()
{
 	SetUpPerspectiveProjection();
 	ApplyCameraTransform();
 
 	OpenGLRenderer::BindFrameBuffer(m_fbo.ID);
 	OpenGLRenderer::ClearScreen(0.0f,0.0f,0.0f);

 	switch(m_gameState)
 	{
 	case SETTING:
 		break;
 	case GAMEPLAY:
 	{
		Matrix4 objectToWorldMatrix;
		m_material->Activate();
		m_material->SendUniform("u_objectToWorldMatrix", objectToWorldMatrix);
 		RenderAxes();
 		OpenGLRenderer::DrawVertexWithVertexArray(m_floorQuad, 4, OpenGLRenderer::SHAPE_QUADS);
 
 		objectToWorldMatrix.ApplyScale(0.1f, 0.1f, 0.1f);
 		m_material->SendUniform("u_objectToWorldMatrix", objectToWorldMatrix);
		if (m_drawBones)
			m_aa.Render();

 		m_animMaterial->Activate();
 		OpenGLRenderer::BindTexture(m_textureId);
 		m_animMaterial->SendUniform("u_boneMatrixs", m_aa.m_boneToObjectMatrixs);
 		m_animMaterial->SendUniform("u_objectToWorldMatrix", objectToWorldMatrix);
 		if (m_drawModels)
 			OpenGLRenderer::DrawVertexWithIndexedVertexBufferObject(m_vboID, m_iboID, m_c23.m_modelVertices.size(), m_c23.m_modelIndices.size(), OpenGLRenderer::SHAPE_TRIANGLES, Vertex_PCTNB());
 
 		int spaceBetweenZombies = 15;
 		int numOfZombieInARow = 5;
 		for (size_t index = 0; index < m_aaList.size(); ++index)
 		{
 			objectToWorldMatrix.LoadIdentity();
 			objectToWorldMatrix.ApplyTransform(((int)index % numOfZombieInARow + 1) * spaceBetweenZombies, ((int)index / numOfZombieInARow) * spaceBetweenZombies, 0);
 			objectToWorldMatrix.ApplyScale(0.1f, 0.1f, 0.1f);
 
 			m_material->Activate();
 			m_material->SendUniform("u_objectToWorldMatrix", objectToWorldMatrix);
 			if (m_drawBones)
 				m_aaList[index].Render();
 
 			m_animMaterial->Activate();
 			m_animMaterial->SendUniform("u_boneMatrixs", m_aaList[index].m_boneToObjectMatrixs);
 			m_animMaterial->SendUniform("u_objectToWorldMatrix", objectToWorldMatrix);
 			if (m_drawModels)
 				OpenGLRenderer::DrawVertexWithIndexedVertexBufferObject(m_vboID, m_iboID, m_c23.m_modelVertices.size(), m_c23.m_modelIndices.size(), OpenGLRenderer::SHAPE_TRIANGLES, Vertex_PCTNB());
 		}

 		OpenGLRenderer::SetOrtho();
 		m_fboMaterial->Activate();
 		m_font->Draw("Zombie Counts:%d", Vec2f(20, 800), 60.0f, RGBA(), Vec2i(1600, 900), m_aaList.size() + 1);
 	}
 		break;
 	};

 	DoPostEffect();
}


void TheGame::DoPostEffect()
{	
	OpenGLRenderer::BindFrameBuffer(0);
	OpenGLRenderer::ClearScreen(0.0f, 0.0f, 0.0f);
	OpenGLRenderer::SendBasicTwoFBOTextures( m_fbo.colorTextureID , m_fbo.depthTextureID );
	OpenGLRenderer::SetOrtho();
	m_fboMaterial->Activate();
	OpenGLRenderer::DrawVertexWithVertexArray2D(m_quad, 4, OpenGLRenderer::SHAPE_QUADS);
}


void TheGame::Update( float deltaSeconds )
{
	switch(m_gameState)
	{
	case INITIALIZE:
		break;
	case SETTING:
		break;
	case GAMEPLAY:
		break;
	case QUITTING:
		m_isQuitting = true;
		break;
	};

	m_aa.Update(deltaSeconds);
	for (size_t index = 0; index < m_aaList.size(); ++index)
		m_aaList[index].Update(deltaSeconds);

	m_camera->Update(deltaSeconds);
	ApplyCameraTransform();
}


void TheGame::RenderAxes()
{
	OpenGLRenderer::LineWidth(2.0f);
	float axisLength = 10.0f;
	float axisStart = 0.0f;
	Vertex_PosColor vertices[8];
	vertices[0].color = RGBA(1.0f,0.0f,0.0f,1.0f);
	vertices[0].position = Vec3f(axisStart,0.0f,0.0f);
	vertices[1].color = RGBA(1.0f,0.0f,0.0f,1.0f);
	vertices[1].position = Vec3f(axisLength,0.0f,0.0f);

	vertices[2].color = RGBA(0.0f,1.0f,0.0f,1.0f);
	vertices[2].position = Vec3f(0.0f,axisStart,0.0f);
	vertices[3].color = RGBA(0.0f,1.0f,0.0f,1.0f);
	vertices[3].position = Vec3f(0.0f,axisLength,0.0f);

	vertices[4].color = RGBA(0.0f,0.0f,1.0f,1.0f);
	vertices[4].position = Vec3f(0.0f,0.0f,axisStart);
	vertices[5].color = RGBA(0.0f,0.0f,1.0f,1.0f);
	vertices[5].position = Vec3f(0.0f,0.0f,axisLength);

	OpenGLRenderer::DrawVertexWithVertexArray(vertices,6,OpenGLRenderer::SHAPE_LINES);
}


void TheGame::SetUpPerspectiveProjection()
{
	float aspect = (WINDOWS_WIDTH / (float)WINDOWS_HEIGHT); // VIRTUAL_SCREEN_WIDTH / VIRTUAL_SCREEN_HEIGHT;
	float fovX = 70.f;
	float fovY = (fovX / aspect);
	float zNear = 0.1f;
	float zFar = 1200.f;

	OpenGLRenderer::LoadIdentity();
	OpenGLRenderer::Perspective( fovY, aspect, zNear, zFar ); // Note: Absent in OpenGL ES 2.0*/
}


void TheGame::ApplyCameraTransform()
{
	// Put us in our preferred coordinate system: +X is east (forward), +Y is north, +Z is up
	OpenGLRenderer::Rotatef( -90.f, 1.f, 0.f, 0.f ); // lean "forward" 90 degrees, to put +Z up (was +Y)
	OpenGLRenderer::Rotatef( 90.f, 0.f, 0.f, 1.f ); // spin "left" 90 degrees, to put +X forward (was +Y)
	// Orient the view per the camera's orientation
	OpenGLRenderer::Rotatef( -m_camera->m_orientation.rollDegreesAboutX	, 1.f, 0.f, 0.f );
	OpenGLRenderer::Rotatef( -m_camera->m_orientation.pitchDegreesAboutY, 0.f, 1.f, 0.f);
	OpenGLRenderer::Rotatef( -m_camera->m_orientation.yawDegreesAboutZ	, 0.f, 0.f, 1.f);
	// Position the view per the camera's position
	OpenGLRenderer::Translatef( -m_camera->m_position.x, -m_camera->m_position.y, -m_camera->m_position.z );
}


void TheGame::UpdateCameraFromMouseAndKeyboard()
{
	// Update camera orientation (yaw and pitch only) from mouse x,y movement
	const float degreesPerMouseDelta = 0.04f;
	Vec2i mouseDeltas = m_input->GetMouseMovementFromLastFrame();
	m_camera->m_orientation.yawDegreesAboutZ	-= (degreesPerMouseDelta * mouseDeltas.x);
	m_camera->m_orientation.pitchDegreesAboutY	+= (degreesPerMouseDelta * mouseDeltas.y);
	float moveSpeed = 8.0f;

	// Update camera position based on which (if any) movement keys are down
	float cameraYawRadians = degree2radians(m_camera->m_orientation.yawDegreesAboutZ);
	float cameraPitchRadians = degree2radians(m_camera->m_orientation.pitchDegreesAboutY);
	Vec3f cameraForwardVector = Vec3f( cos(cameraYawRadians) * cos(cameraPitchRadians) , sin(cameraYawRadians) * cos(cameraPitchRadians) , -sin(cameraPitchRadians));
	m_camera->m_forwardVector = cameraForwardVector;
	m_camera->m_rightVector = m_camera->m_forwardVector.crossProductWith(Vec3f(0, 0, 1));
	m_camera->m_upVector = m_camera->m_forwardVector.crossProductWith(m_camera->m_rightVector);

	if(m_input->m_keyState[Inputs::INPUT_SHIFT].isHold)
		moveSpeed = 100.0f;

	//Vec3f movementVector( 0.f, 0.f, 0.f );
	//m_camera.m_movementVector = Vec3f( 0.f, 0.f, 0.f );
	if( m_input->m_keyState[ 'W' ].isHold )
		m_camera->m_movementVector += m_camera->m_forwardVector * moveSpeed;
	if( m_input->m_keyState[ 'S' ].isHold )
		m_camera->m_movementVector -= m_camera->m_forwardVector * moveSpeed;
	if( m_input->m_keyState[ 'A' ].isHold )
		m_camera->m_movementVector -= m_camera->m_rightVector * moveSpeed;
	if( m_input->m_keyState[ 'D' ].isHold )
		m_camera->m_movementVector += m_camera->m_rightVector * moveSpeed;

 	if( m_input->m_keyState[Inputs::INPUT_SPACE].isHold )
 		m_camera->m_movementVector = Vec3f(0.f, 0.f, 0.25f) * moveSpeed;

	if( m_input->m_keyState[ 'Z' ].isHold )
		m_camera->m_movementVector = Vec3f(0.f, 0.f, -0.25f) * moveSpeed;
	//...and so on, for A,S,D moving left, back, right - and for E,C moving up, down.
}


GameState TheGame::GetGameState()
{
	return m_gameState;
}


void TheGame::SetGameState(GameState state)
{
	m_gameState = state;
}


void TheGame::Initialize()
{
	ShowCursor(false);
	m_isQuitting = false;
	m_material = new Material(ShaderProgram(OpenGLRenderer::CreateProgram(), new OpenGLShader("./Data/Shaders/doNothing_fragment.glsl", OpenGLRenderer::FRAGMENT_SHADER), new OpenGLShader("./Data/Shaders/doNothing_vertex.glsl", OpenGLRenderer::VERTEX_SHADER)));
	m_fboMaterial = new Material(ShaderProgram(OpenGLRenderer::CreateProgram(), new OpenGLShader("./Data/Shaders/frameBuffer_fragment.glsl", OpenGLRenderer::FRAGMENT_SHADER), new OpenGLShader("./Data/Shaders/frameBuffer_vertex.glsl", OpenGLRenderer::VERTEX_SHADER)));
	m_animMaterial = new Material(ShaderProgram(OpenGLRenderer::CreateProgram(), new OpenGLShader("./Data/Shaders/skeletalAnim_fragment.glsl", OpenGLRenderer::FRAGMENT_SHADER), new OpenGLShader("./Data/Shaders/skeletalAnim_vertex.glsl", OpenGLRenderer::VERTEX_SHADER)));
	OpenGLRenderer::InitializeFrameBuffer(m_fbo);
	m_input = g_theApplication->m_input;
	m_console = g_theApplication->m_console;

	m_quad[0].position = Vec3f(0.0f, 0.0f, 0.0f);
	m_quad[1].position = Vec3f((float)g_theApplication->m_windowSize.x, 0.0f, 0.0f);
	m_quad[2].position = Vec3f((float)g_theApplication->m_windowSize.x, (float)g_theApplication->m_windowSize.y, 0.0f);
	m_quad[3].position = Vec3f(0.0f, (float)g_theApplication->m_windowSize.y, 0.0f);

	m_quad[0].texCoords = Vec2f(0.0f, 0.0f);
	m_quad[1].texCoords = Vec2f(1.0f, 0.0f);
	m_quad[2].texCoords = Vec2f(1.0f, 1.0f);
	m_quad[3].texCoords = Vec2f(0.0f, 1.0f);

	m_quad[0].color = RGBA(1.0f, 1.0f, 1.0f, 1.0f);
	m_quad[1].color = RGBA(1.0f, 1.0f, 1.0f, 1.0f);
	m_quad[2].color = RGBA(1.0f, 1.0f, 1.0f, 1.0f);
	m_quad[3].color = RGBA(1.0f, 1.0f, 1.0f, 1.0f);

	m_console->m_showErrorMsg = false;

	m_camera = new Camera3D(m_input);

	m_floorQuad[0].position = Vec3f(-5000, -5000, 0);
	m_floorQuad[1].position = Vec3f(5000, -5000, 0);
	m_floorQuad[2].position = Vec3f(5000, 5000, 0);
	m_floorQuad[3].position = Vec3f(-5000, 5000, 0);

	m_floorQuad[0].color = RGBA(0.8f, 0.8f, 0.8f, 0.6f);
	m_floorQuad[1].color = RGBA(0.8f, 0.8f, 0.8f, 0.6f);
	m_floorQuad[2].color = RGBA(0.8f, 0.8f, 0.8f, 0.6f);
	m_floorQuad[3].color = RGBA(0.8f, 0.8f, 0.8f, 0.6f);

	m_animationSystem.LoadAnimationFile("./Data/zombie_walk.anim.C23", "zombie", "walk");
	m_animationSystem.LoadAnimationFile("./Data/zombie_attack.anim.C23", "zombie", "attack");
	m_aa.AttachToSkeletalAnimationObject(m_animationSystem.GetSkeletalAnimationObject("zombie"));
	m_c23.LoadModel("./Data/zombie.model.C23");

	m_font = new BitmapFont("./Data/BMPFonts/buxton.fnt");
	m_zombieTexture = new Texture("./Data/zombie_df.png");
	m_zombieDebugTexture = new Texture("./Data/zombie_df.tga");
	m_textureId = m_zombieTexture->m_textureID;
	m_drawBones = true;
	m_drawModels = true;

	OpenGLRenderer::GenerateBuffers(&m_vboID);
	OpenGLRenderer::GenerateBuffers(&m_iboID);
	OpenGLRenderer::BufferData(m_c23.m_modelVertices, m_vboID);
	OpenGLRenderer::BufferData(m_c23.m_modelIndices, m_iboID);

	SetGameState(GAMEPLAY);
}


};