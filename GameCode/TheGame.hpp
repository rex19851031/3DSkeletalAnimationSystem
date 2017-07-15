#pragma once

#ifndef THEGAME_HPP
#define THEGAME_HPP

#define WIN32_LEAN_AND_MEAN
#include <queue>
#include <list>
#include <windows.h>
#include <Xinput.h>					// include the Xinput API
#include <stdio.h>
#pragma comment( lib, "xinput" )	// Link in the xinput.lib static library

#include "Engine\Core\Camera3D.hpp"
#include "Engine\Renderer\Material.hpp"
#include "Engine\Math\GeneralStruct.hpp"
#include "Engine\Input\InputSystem.hpp"
#include "Engine\Parsing\TinyXML\TinyXMLHelper.hpp"
#include "Engine\Core\DeveloperConsole.hpp"
#include "Engine\Renderer\Sprites.hpp"
#include "Engine\Parsing\C23Parser\C23Parser.hpp"

#include "GameCode\SkeletalAnimationSystem.hpp"
#include "GameCode\AnimationActor.hpp"


namespace Henry
{

enum GameState{ INITIALIZE = 0, SETTING, GAMEPLAY, QUITTING };

class TheGame
{
public:
	TheGame(void);
	~TheGame(void);
	void Input();
	void Update( float deltaSeconds );
	void Render();
	GameState GetGameState();
	void SetGameState(GameState state);
	
public:
	bool m_isQuitting;

private:
	void Initialize();
	void LoadDataFiles();
	void SetUpPerspectiveProjection();
	void ApplyCameraTransform();
	void RenderAxes();
	void UpdateCameraFromMouseAndKeyboard();
	void PollKeyStates();
	void DoPostEffect();
	//
	void UpdateCameraFollowPlayer();
	
private:
	DeveloperConsole* m_console;
	Material* m_material;
	Material* m_animMaterial;
	Material* m_fboMaterial;
	Camera3D* m_camera;
	FBO m_fbo;
	GameState m_gameState;
	InputSystem* m_input;
	Matrix4 m_objectToWorldMatrix;
	Vertex_PCT m_quad[4];
	Vertex_PosColor m_floorQuad[4];

	BitmapFont* m_font;
	Texture* m_zombieTexture;
	Texture* m_zombieDebugTexture;
	int m_textureId;
	bool m_drawBones;
	bool m_drawModels;

	SkeletalAnimationSystem m_animationSystem;
	AnimationActor m_aa;
	std::vector<AnimationActor> m_aaList;
	C23Parser m_c23;
	int m_vboID;
	int m_iboID;
};

};

#endif