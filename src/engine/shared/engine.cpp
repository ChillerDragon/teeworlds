/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <stdlib.h> // srand

#include <base/system.h>

#include <engine/engine.h>
#include <engine/shared/network.h>


class CEngine : public IEngine
{
public:
	bool m_Logging;
	IOHANDLE m_DataLogSent;
	IOHANDLE m_DataLogRecv;
	const char *m_pAppname;

	CEngine(const char *pAppname)
	{
		srand(time_get());
		dbg_logger_stdout();
		dbg_logger_debugger();

		//
		dbg_msg("engine", "running on %s-%s-%s", CONF_FAMILY_STRING, CONF_PLATFORM_STRING, CONF_ARCH_STRING);
	#ifdef CONF_ARCH_ENDIAN_LITTLE
		dbg_msg("engine", "arch is little endian");
	#elif defined(CONF_ARCH_ENDIAN_BIG)
		dbg_msg("engine", "arch is big endian");
	#else
		dbg_msg("engine", "unknown endian");
	#endif

		m_DataLogSent = 0;
		m_DataLogRecv = 0;
		m_Logging = false;
		m_pAppname = pAppname;
	}

	~CEngine()
	{
		StopLogging();
	}

	void Init()
	{
	}

	void QueryNetLogHandles(IOHANDLE *pHDLSend, IOHANDLE *pHDLRecv)
	{
		*pHDLSend = m_DataLogSent;
		*pHDLRecv = m_DataLogRecv;
	}

	void StartLogging(IOHANDLE DataLogSent, IOHANDLE DataLogRecv)
	{
		if(DataLogSent)
		{
			m_DataLogSent = DataLogSent;
			dbg_msg("engine", "logging network sent packages");
		}
		else
			dbg_msg("engine", "failed to start logging network sent packages");

		if(DataLogRecv)
		{
			m_DataLogRecv = DataLogRecv;
			dbg_msg("engine", "logging network recv packages");
		}
		else
			dbg_msg("engine", "failed to start logging network recv packages");

		m_Logging = true;
	}

	void StopLogging()
	{
		if(m_DataLogSent)
		{
			dbg_msg("engine", "stopped logging network sent packages");
			io_close(m_DataLogSent);
			m_DataLogSent = 0;
		}
		if(m_DataLogRecv)
		{
			dbg_msg("engine", "stopped logging network recv packages");
			io_close(m_DataLogRecv);
			m_DataLogRecv = 0;
		}
		m_Logging = false;
	}
};

IEngine *CreateEngine(const char *pAppname) { return new CEngine(pAppname); }
