#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <algorithm>

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/MathUtils.hpp"

const Rgba8 DevConsole::ERRORMSG		= Rgba8::RED;
const Rgba8 DevConsole::WARNINGMSG		= Rgba8::YELLOW;
const Rgba8 DevConsole::INFOMSG_MAJOR	= Rgba8::BLUE;
const Rgba8 DevConsole::INFOMSG_MINOR	= Rgba8::GREEN;

DevConsole* g_devConsole = nullptr;

DevConsole::DevConsole( DevConsoleConfig const& config )
	: m_config( config )
{
	m_config.camera->SetOrthoView( Vec2( 0.f, 0.f ), Vec2( 1600.f, 800.f ) );
	m_historyInput.reserve( m_config.maxCommandHistory );
	m_lines.reserve( m_config.maxLogLines );
	m_cursorTimer = new Timer( 1.f );
}

DevConsole::~DevConsole()
{
}

void DevConsole::Startup()
{
	m_cursorTimer->Start();
	if (g_eventSystem != nullptr)
	{
		g_eventSystem->SubscribeEventCallBackFunc( "help", reinterpret_cast<void(*)()>(PrintAllCommand), 1 );
		g_eventSystem->SubscribeEventCallBackFunc( "clear", reinterpret_cast<void(*)()>(ClearScreen), 1 );
		g_eventSystem->SubscribeEventCallBackFunc( "history", reinterpret_cast<void(*)()>(PrintHistory), 1 );
	}
}

void DevConsole::Shutdown()
{
}

void DevConsole::BeginFrame()
{
}

void DevConsole::EndFrame()
{
}

void DevConsole::RecordInput()
{
	m_historyInput.push_back( m_inputText );
	m_historyIndex = static_cast<int>(m_historyInput.size());
}

void DevConsole::UsePrevInput()
{
	if (m_historyInput.size() == 0)
	{
		m_inputText = "";
		m_cursorPos = 0;
		return;
	}
	if (m_historyIndex - 1 < 0)
	{
		return;
	}

	m_historyIndex--;
	m_inputText = m_historyInput[m_historyIndex];
	m_cursorPos = static_cast<int>(m_historyInput[m_historyIndex].length());
	ResetSelected();
}

void DevConsole::UseNextInput()
{
	if (m_historyIndex + 1 >= m_historyInput.size())
	{
		return;
	}

	m_historyIndex++;
	m_inputText = m_historyInput[m_historyIndex];
	m_cursorPos = static_cast<int>(m_historyInput[m_historyIndex].length());
	ResetSelected();
}

void DevConsole::FillInput()
{

}

void DevConsole::ClearSelected()
{
	if (m_selectStart == m_cursorPos)
		return;

	int leftBound = MIN( m_selectStart, m_cursorPos );
	int rightBound = MAX( m_selectStart, m_cursorPos );

	m_inputText.erase( leftBound, rightBound - leftBound );
	m_cursorPos = leftBound;
	ResetSelected();
}

void DevConsole::ResetSelected()
{
	m_selectStart = m_cursorPos;
}

void DevConsole::CopyToClipBoard()
{
	if (!IsSelecting())
		return;
	if (!IsClipboardFormatAvailable( CF_TEXT ))
		return;
	if (!OpenClipboard( 0 ))
		return;

	int leftBound = MIN( m_selectStart, m_cursorPos );
	int rightBound = MAX( m_selectStart, m_cursorPos );

	std::string output = m_inputText.substr( leftBound, rightBound - leftBound );
	char const* outputCStr = output.c_str();
	size_t const leng = strlen( outputCStr ) + 1;
	HGLOBAL hMem = GlobalAlloc( GMEM_MOVEABLE, leng );
	memcpy( GlobalLock( hMem ), outputCStr, leng );
	GlobalUnlock( hMem );
	OpenClipboard( 0 );
	EmptyClipboard();
	SetClipboardData( CF_TEXT, hMem );
	CloseClipboard();
	ResetSelected();
}

void DevConsole::PasteFromClipBoard()
{
	if (!IsClipboardFormatAvailable( CF_TEXT ))
		return;
	if (!OpenClipboard( 0 ))
		return;

	HGLOBAL hglb = GetClipboardData( CF_TEXT );
	if (hglb != NULL)
	{
		std::string clipboardText = (char*)hglb;
		ClearSelected();
		m_inputText.insert( m_cursorPos, clipboardText );
		m_cursorPos += static_cast<int>(clipboardText.length());
		ResetSelected();
	}
}

void DevConsole::Execute( std::string const& consoleCommandText )
{
	Strings commands = Split( consoleCommandText, '\n', true );
	for (std::string commandIndex : commands)
	{
		AddLine( INFOMSG_MAJOR, commandIndex );
		std::replace( commandIndex.begin(), commandIndex.end(), '=', ' ' );
		Strings contents = Split( commandIndex, ' ', true );
		Strings contents2;
		if (commandIndex[0] != '"')
			contents2 = Split( commandIndex, ' ', true );
		else
			contents2.push_back( commandIndex );
		if (contents.size() > 0 && !g_eventSystem->IsEventSubscribed( contents2[0] ))
		{
			AddLine( ERRORMSG, "Invalid Command" );
			return;
 		}

// 		if (contents.size() > 1)
// 		{
// 			contents[1].erase( std::remove_if( contents[1].begin(), contents[1].end(), []( unsigned char x ) { return std::isspace( x ); } ), contents[1].end() );
// 		}
		int totalSubs = 0;
		totalSubs = g_eventSystem->FireEvent( contents2 );
// 		if (totalSubs == 0)
// 			AddLine( ERRORMSG, "None responded" );
// 		else
// 			AddLine( DevConsole::INFOMSG_MAJOR, std::to_string( totalSubs ) + " event(s) fired successfully" );
	}
}

void DevConsole::AddLine( Rgba8 const& color, std::string const& text )
{
	m_lines.push_back( DevConsoleLine{ color, text } );
}

void DevConsole::ResetLog()
{
	m_lines.clear();
}

void DevConsole::Render( AABB2 const& bounds, Renderer* rendererOverride ) const
{
	if (m_mode == DevConsoleMode::HIDDEN)
		return;
	Renderer* renderer = (rendererOverride == nullptr) ? m_config.renderer : rendererOverride;
	renderer->BeginCamera( *m_config.camera );
 	Render_OpenFull( bounds, *renderer, *m_config.font, m_config.fontAspect );
	renderer->EndCamera( *m_config.camera );
}

Strings DevConsole::GetHistoryInput()
{
	return m_historyInput;
}

DevConsoleMode DevConsole::GetMode() const
{
	return m_mode;
}

void DevConsole::SetMode( DevConsoleMode mode )
{
	if (m_mode != mode)
	{
		m_cursorTimer->Start();
	}
	m_mode = mode;
}

void DevConsole::ToggleMode()
{
	if (m_mode == DevConsoleMode::DISPLAY)
		m_mode = DevConsoleMode::HIDDEN;
	else
		m_mode = DevConsoleMode::DISPLAY;
	m_cursorTimer->Start();

}

void DevConsole::Render_OpenFull( AABB2 const& bounds, Renderer& renderer, BitmapFont& font, float fontAspect ) const
{
	std::vector<Vertex_PCU>panel;
	std::vector<Vertex_PCU>textMap;

	int expectLineNum = static_cast<int>(m_config.linesToRender);
	textMap.reserve( expectLineNum * 6 * 100 );
	float cellHeight = bounds.GetDimensions().y / m_config.linesToRender + 1;
	float cellWidth = cellHeight * fontAspect;
	AddVertsForAABB2D( panel, AABB2( Vec2( 0.f, 0.f ), Vec2( bounds.GetDimensions().x, cellHeight ) ), Rgba8( 0, 0, 0, 150 ) );
	AddVertsForAABB2D( panel, AABB2( Vec2( 0.f, cellHeight ), bounds.GetDimensions() ), Rgba8( 255, 255, 255, 50 ) );
	
	renderer.SetModelConstants();
	renderer.SetDepthMode( DepthMode::ENABLED );
	renderer.SetBlendMode( BlendMode::ALPHA );
 	renderer.BindTexture( nullptr );
 	renderer.DrawVertexArray( panel );

	for (int i = static_cast<int>(m_lines.size()) - 1; i >= static_cast<int>(m_lines.size()) - expectLineNum; --i)
	{
		if (i < 0)
			break;

		int index = static_cast<int>(m_lines.size()) - 1 - i + 1;

		AABB2 boundOnIndex = AABB2(
			Vec2(
				bounds.m_mins.x,
				bounds.m_mins.y + cellHeight * index
			),
			Vec2(
				bounds.m_maxs.x,
				bounds.m_mins.y + cellHeight * (index + 1)
			)
		);
		font.AddVertsForTextInBox2D( textMap, boundOnIndex, cellHeight * 0.6f, m_lines[i].text, m_lines[i].color, fontAspect, Vec2( 0.f, 0.5f ) );
	}

	font.AddVertsForTextInBox2D( textMap, AABB2( Vec2( 0.f, 0.f ), Vec2( bounds.GetDimensions().x, cellHeight ) ), cellHeight * 0.6f, m_inputText, Rgba8::WHITE, fontAspect, Vec2( 0.f, 0.3f ), OVERRUN );
	renderer.SetModelConstants();
	renderer.SetDepthMode( DepthMode::ENABLED );
	renderer.SetBlendMode( BlendMode::ALPHA );
	renderer.BindTexture( &font.GetTexture() );
 	renderer.DrawVertexArray( textMap );

 	if (IsSelecting())
 	{
 		std::vector<Vertex_PCU>selectedBox;
 		std::vector<Vertex_PCU>selectedInput;
 
 		int leftBound = MIN( m_selectStart, m_cursorPos );
 		int rightBound = MAX( m_selectStart, m_cursorPos );
 
 		selectedBox.reserve( 4 );
 		selectedInput.reserve( (rightBound - leftBound) * 6 * 100 );
 
 		AddVertsForAABB2D( selectedBox, AABB2( Vec2( leftBound * cellWidth * 0.6f, 0.f ), Vec2( rightBound * cellWidth * 0.6f, cellHeight ) ), Rgba8::WHITE );
 		font.AddVertsForTextInBox2D( selectedInput, AABB2( Vec2( leftBound * cellWidth * 0.6f, 0.f ), Vec2( rightBound * cellWidth * 0.6f, cellHeight ) ), cellHeight * 0.6f, m_inputText.substr( leftBound, rightBound - leftBound ), Rgba8::BLACK, fontAspect, Vec2( 0.f, 0.3f ), OVERRUN );
 		renderer.SetModelConstants();
		renderer.SetDepthMode( DepthMode::ENABLED );
 		renderer.SetBlendMode( BlendMode::OPAQUE );
 		renderer.BindTexture( nullptr );
 		renderer.DrawVertexArray( selectedBox );
 
 		renderer.SetModelConstants();
		renderer.SetDepthMode( DepthMode::ENABLED );
 		renderer.SetBlendMode( BlendMode::ALPHA );
 		renderer.BindTexture( &font.GetTexture() );
 		renderer.DrawVertexArray( selectedInput );
 	}

	std::vector<Vertex_PCU>insertion;
	if (m_cursorTimer->GetElapsedTime() <= 0.5f)
	{
		AddVertsForAABB2D( insertion,
			AABB2( Vec2( cellWidth * 0.6f * m_cursorPos, cellHeight * 0.12f ), Vec2( cellWidth * 0.6f * m_cursorPos + cellWidth * 0.6f, cellHeight * 0.17f ) ),
			(m_cursorPos < m_selectStart) ? Rgba8::BLACK : Rgba8::WHITE
		);
	}
	m_cursorTimer->DecrementPeriodIfElapsed();
	renderer.SetModelConstants();
	renderer.SetDepthMode( DepthMode::ENABLED );
	renderer.SetBlendMode( BlendMode::ALPHA );
	renderer.BindTexture( nullptr );
	renderer.DrawVertexArray( insertion );
}

bool DevConsole::IsPosSelected( int pos ) const
{
	if (!IsSelecting())
		return false;

	int leftBound = MIN( m_selectStart, m_cursorPos );
	int rightBound = MAX( m_selectStart, m_cursorPos );
	return pos >= leftBound && pos < rightBound;
}

bool PrintAllCommand()
{
	Strings commands = g_eventSystem->GetAllSubscribedName();
	for (std::string command : commands)
	{
		g_devConsole->AddLine( DevConsole::INFOMSG_MINOR, command );
	}
	return false;
}

bool ClearScreen()
{
	g_devConsole->ResetLog();
	return false;
}

bool PrintHistory()
{
	Strings inputs = g_devConsole->GetHistoryInput();
	for (std::string historyInput : inputs)
	{
		g_devConsole->AddLine( DevConsole::INFOMSG_MINOR, historyInput );
	}
	return false;
}

bool DevConsoleLiteralKey( unsigned char param )
{
	if (g_devConsole == nullptr)
	{
		return false;
	}
	if (!g_devConsole->IsOn())
	{
		return false;
	}

	if (param >= 32 && param <= 126 && param != 96 && param != 13)
	{
		g_devConsole->ClearSelected();
		g_devConsole->m_inputText.insert( g_devConsole->m_inputText.begin() + g_devConsole->m_cursorPos, param );
		g_devConsole->m_cursorPos++;
		g_devConsole->ResetSelected();
		g_devConsole->m_cursorTimer->Start();
	}
	else if (param == 1) // Ctrl A
	{
		g_devConsole->m_cursorPos = (int)g_devConsole->m_inputText.length();
		g_devConsole->m_selectStart = 0;
	}
	else if (param == 3) // Ctrl C
	{
		g_devConsole->CopyToClipBoard();
	}
	else if (param == 22) // Ctrl V
	{
		g_devConsole->PasteFromClipBoard();
	}
	else if (param == 24) // Ctrl X
	{
		g_devConsole->CopyToClipBoard();
		g_devConsole->ClearSelected();
	}
	return true;
}

bool DevConsoleFunctionKey( unsigned char param )
{
	if (g_devConsole == nullptr)
	{
		return false;
	}
	if (!g_devConsole->IsOn())
	{
		if (param == KEYCODE_TILDE)
		{
			g_devConsole->SetMode( DISPLAY );
			return true;
		}
		return false;
	}

	int inputLength = static_cast<int>(g_devConsole->m_inputText.length());

	if (param == KEYCODE_BACKSPACE) // Backspace
	{
		if (g_devConsole->IsSelecting())
		{
			g_devConsole->ClearSelected();
		}
		else if (inputLength > 0 &&
			g_devConsole->m_cursorPos > 0)
		{
			g_devConsole->m_inputText.erase( g_devConsole->m_inputText.begin() + g_devConsole->m_cursorPos - 1 );
			g_devConsole->m_cursorPos = ClampInt( g_devConsole->m_cursorPos - 1, 0, inputLength );
			g_devConsole->ResetSelected();
			g_devConsole->m_cursorTimer->Start();
		}
	}

	if (param == KEYCODE_DELETE) // Delete
	{
		if (g_devConsole->IsSelecting())
		{
			g_devConsole->ClearSelected();
		}
		else if (inputLength > 0 &&
			g_devConsole->m_cursorPos != inputLength)
		{
			g_devConsole->m_inputText.erase( g_devConsole->m_inputText.begin() + g_devConsole->m_cursorPos );
			g_devConsole->m_cursorPos = ClampInt( g_devConsole->m_cursorPos , 0, inputLength );
			g_devConsole->ResetSelected();
			g_devConsole->m_cursorTimer->Start();
		}
	}

	if (param == KEYCODE_ESC) // ESC
	{
		if (g_devConsole->m_inputText.empty())
		{
			g_devConsole->SetMode( HIDDEN );
		}
		else
		{
			g_devConsole->m_inputText.clear();
			g_devConsole->m_cursorPos = 0;
			g_devConsole->ResetSelected();
			g_devConsole->m_cursorTimer->Start();
		}
	}

	if (param == KEYCODE_TILDE) // Tilde
	{
		g_devConsole->SetMode( HIDDEN );
	}

	if (param == KEYCODE_ENTER) // Enter
	{
		if (g_devConsole->m_inputText.empty())
		{
			g_devConsole->SetMode( HIDDEN );
		}
		else
		{
			g_devConsole->RecordInput();
			std::string inputCache = g_devConsole->m_inputText;
			g_devConsole->m_inputText.clear();
			g_devConsole->m_cursorPos = 0;
			g_devConsole->ResetSelected();
			g_devConsole->m_cursorTimer->Start();
			g_devConsole->Execute( inputCache );
		}
	}
	if (param == KEYCODE_LEFT) // LEFT
	{
		SHORT state = GetKeyState( VK_SHIFT );
		bool pressed = (state & 0x8000) ? true : false;

		g_devConsole->m_cursorPos = ClampInt( g_devConsole->m_cursorPos - 1, 0, inputLength );
		g_devConsole->m_cursorTimer->Start();
		if (!pressed)
		{
			g_devConsole->m_selectStart = g_devConsole->m_cursorPos;
		}
	}
	else if (param == KEYCODE_UP) // UP
	{
		g_devConsole->UsePrevInput();
		g_devConsole->m_cursorTimer->Start();
	}
	else if (param == KEYCODE_RIGHT) // RIGHT
	{
		SHORT state = GetKeyState( VK_SHIFT );
		bool pressed = (state & 0x8000) ? true : false;

		g_devConsole->m_cursorPos = ClampInt( g_devConsole->m_cursorPos + 1, 0, inputLength );
		g_devConsole->m_cursorTimer->Start();
		if (!pressed)
		{
			g_devConsole->m_selectStart = g_devConsole->m_cursorPos;
		}
	}
	else if (param == KEYCODE_DOWN) // DOWN
	{
		g_devConsole->UseNextInput();
		g_devConsole->m_cursorTimer->Start();
	}
	return true;
}
