#include "TheApplication.hpp"
#include "Engine\Core\Time.hpp"

extern HDC g_displayDeviceContext;

namespace Henry
{

TheApplication* g_theApplication = nullptr;

TheApplication::TheApplication(void* platformHandle) : m_platformHandle(platformHandle)
{
	g_theApplication = this;
	
	Initialize();
}


TheApplication::~TheApplication(void)
{
	delete m_console;
	delete m_theGame;
	delete m_input;
}


void TheApplication::Initialize()
{
	RECT rect;
	if (GetClientRect((HWND)m_platformHandle, &rect))
	{
		m_windowSize.x = rect.right - rect.left;
		m_windowSize.y = rect.bottom - rect.top;
		m_windowCenter = Vec2i(m_windowSize.x >> 1 , m_windowSize.y >> 1);
	}
	
	OpenGLRenderer::Initialize(m_windowSize);

	//m_renderer = new OpenGLRenderer();
	m_console = new DeveloperConsole();
	m_input = new InputSystem(m_platformHandle);
	m_theGame = new TheGame();

	m_isQuitting = false;
	m_showConsole = false;
}


void TheApplication::Run()
{
	while(!m_isQuitting)
	{
		Input();
		Update();
		Render();
	}
}


void TheApplication::Input()
{ 
	m_input->ToggleCursorDisplay(m_showConsole);

	if(m_showConsole)
	{
		for(size_t index = 0; index < m_input->m_chars.size(); index++)
		{
			char currentChar = m_input->m_chars[index];
			m_showConsole = m_console->GetSpecialInput(currentChar);
			m_console->PassCharToLog(currentChar);
		}
	}
	else
	{
		m_theGame->Input();
		for(size_t index = 0; index < m_input->m_chars.size(); index++)
		{
			if(m_input->m_chars[index] == '`')
				m_showConsole = true;
		}
	}

	m_isQuitting = m_theGame->m_isQuitting;
}


void TheApplication::Update()
{
	double time_now = GetCurrentTimeSeconds();
	static double previous_frame_time = GetCurrentTimeSeconds();
	double deltaSeconds = time_now - previous_frame_time;

	m_console->Update((float)deltaSeconds);
	m_input->Update();
	m_theGame->Update((float)deltaSeconds);

	previous_frame_time = time_now;
}


void TheApplication::Render()
{
	m_theGame->Render();

	if(m_showConsole)
		m_console->Draw();

	float frameSecond = (1.0f/60.0f);
	double time_now = GetCurrentTimeSeconds();
	static double previous_frame_time = GetCurrentTimeSeconds();
	while(time_now-previous_frame_time < frameSecond)
	{
		time_now = GetCurrentTimeSeconds();
	}

	previous_frame_time = time_now;
	SwapBuffers( g_displayDeviceContext );
}


};