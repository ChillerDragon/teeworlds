
#include <base/tl/threading.h>

#include "graphics_threaded.h"
#include "backend_sdl.h"

// ------------ CGraphicsBackend_Threaded

void CGraphicsBackend_Threaded::ThreadFunc(void *pUser)
{
}

CGraphicsBackend_Threaded::CGraphicsBackend_Threaded()
{
	m_pBuffer = 0x0;
	m_pProcessor = 0x0;
	m_pThread = 0x0;
}

void CGraphicsBackend_Threaded::StartProcessor(ICommandProcessor *pProcessor)
{
	m_Shutdown = false;
	m_pProcessor = pProcessor;
	m_pThread = thread_init(ThreadFunc, this);
	m_BufferDone.signal();
}

void CGraphicsBackend_Threaded::StopProcessor()
{
	m_Shutdown = true;
	m_Activity.signal();
	thread_wait(m_pThread);
	thread_destroy(m_pThread);
}

void CGraphicsBackend_Threaded::RunBuffer(CCommandBuffer *pBuffer)
{
	WaitForIdle();
	m_pBuffer = pBuffer;
	m_Activity.signal();
}

bool CGraphicsBackend_Threaded::IsIdle() const
{
	return m_pBuffer == 0x0;
}

void CGraphicsBackend_Threaded::WaitForIdle()
{
	while(m_pBuffer != 0x0)
		m_BufferDone.wait();
}


// ------------ CCommandProcessorFragment_General

void CCommandProcessorFragment_General::Cmd_Signal(const CCommandBuffer::SCommand_Signal *pCommand)
{
}

bool CCommandProcessorFragment_General::RunCommand(const CCommandBuffer::SCommand * pBaseCommand)
{
	return true;
}

// ------------ CCommandProcessorFragment_OpenGL

int CCommandProcessorFragment_OpenGL::TexFormatToOpenGLFormat(int TexFormat)
{
	if(TexFormat == CCommandBuffer::TEXFORMAT_RGB) return GL_RGB;
	if(TexFormat == CCommandBuffer::TEXFORMAT_ALPHA) return GL_ALPHA;
	if(TexFormat == CCommandBuffer::TEXFORMAT_RGBA) return GL_RGBA;
	return GL_RGBA;
}

unsigned char CCommandProcessorFragment_OpenGL::Sample(int w, int h, const unsigned char *pData, int u, int v, int Offset, int ScaleW, int ScaleH, int Bpp)
{
	int Value = 0;
	for(int x = 0; x < ScaleW; x++)
		for(int y = 0; y < ScaleH; y++)
			Value += pData[((v+y)*w+(u+x))*Bpp+Offset];
	return Value/(ScaleW*ScaleH);
}

void *CCommandProcessorFragment_OpenGL::Rescale(int Width, int Height, int NewWidth, int NewHeight, int Format, const unsigned char *pData)
{
	return nullptr;
}

void CCommandProcessorFragment_OpenGL::SetState(const CCommandBuffer::SState &State)
{
}

void CCommandProcessorFragment_OpenGL::Cmd_Init(const SCommand_Init *pCommand)
{
}

void CCommandProcessorFragment_OpenGL::Cmd_Texture_Update(const CCommandBuffer::SCommand_Texture_Update *pCommand)
{
}

void CCommandProcessorFragment_OpenGL::Cmd_Texture_Destroy(const CCommandBuffer::SCommand_Texture_Destroy *pCommand)
{
}

void CCommandProcessorFragment_OpenGL::Cmd_Texture_Create(const CCommandBuffer::SCommand_Texture_Create *pCommand)
{
}

void CCommandProcessorFragment_OpenGL::Cmd_Clear(const CCommandBuffer::SCommand_Clear *pCommand)
{
}

void CCommandProcessorFragment_OpenGL::Cmd_Render(const CCommandBuffer::SCommand_Render *pCommand)
{
}

void CCommandProcessorFragment_OpenGL::Cmd_Screenshot(const CCommandBuffer::SCommand_Screenshot *pCommand)
{
}

CCommandProcessorFragment_OpenGL::CCommandProcessorFragment_OpenGL()
{
}

bool CCommandProcessorFragment_OpenGL::RunCommand(const CCommandBuffer::SCommand * pBaseCommand)
{
	return true;
}


// ------------ CCommandProcessorFragment_SDL

void CCommandProcessorFragment_SDL::Cmd_Init(const SCommand_Init *pCommand)
{
}

void CCommandProcessorFragment_SDL::Cmd_Shutdown(const SCommand_Shutdown *pCommand)
{
}

void CCommandProcessorFragment_SDL::Cmd_Swap(const CCommandBuffer::SCommand_Swap *pCommand)
{
}

void CCommandProcessorFragment_SDL::Cmd_VideoModes(const CCommandBuffer::SCommand_VideoModes *pCommand)
{
}

CCommandProcessorFragment_SDL::CCommandProcessorFragment_SDL()
{
}

bool CCommandProcessorFragment_SDL::RunCommand(const CCommandBuffer::SCommand *pBaseCommand)
{
	return true;
}

// ------------ CCommandProcessor_SDL_OpenGL

void CCommandProcessor_SDL_OpenGL::RunBuffer(CCommandBuffer *pBuffer)
{
}

// ------------ CGraphicsBackend_SDL_OpenGL

int CGraphicsBackend_SDL_OpenGL::Init(const char *pName, int *Width, int *Height, int FsaaSamples, int Flags)
{
	return 0;
}

int CGraphicsBackend_SDL_OpenGL::Shutdown()
{
	return 0;
}

int CGraphicsBackend_SDL_OpenGL::MemoryUsage() const
{
	return m_TextureMemoryUsage;
}

void CGraphicsBackend_SDL_OpenGL::Minimize()
{
}

void CGraphicsBackend_SDL_OpenGL::Maximize()
{
	// TODO: SDL
}

int CGraphicsBackend_SDL_OpenGL::WindowActive()
{
	return 0;
}

int CGraphicsBackend_SDL_OpenGL::WindowOpen()
{
	return 0;

}


IGraphicsBackend *CreateGraphicsBackend() { return new CGraphicsBackend_SDL_OpenGL; }
