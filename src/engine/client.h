/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_CLIENT_H
#define ENGINE_CLIENT_H

#include "message.h"

class IClient
{
public:

	class CSnapItem
	{
	public:
		int m_Type;
		int m_ID;
		int m_DataSize;
	};

	virtual const char *MapDownloadName() const = 0;
	virtual int MapDownloadAmount() const = 0;
	virtual int MapDownloadTotalsize() const = 0;

	// remote console
	virtual void RconAuth(const char *pUsername, const char *pPassword) = 0;
	virtual bool RconAuthed() const = 0;
	virtual bool UseTempRconCommands() const = 0;
	virtual void Rcon(const char *pLine) = 0;

	// snapshot interface

	enum
	{
		SNAP_CURRENT=0,
		SNAP_PREV=1
	};

	// TODO: Refactor: should redo this a bit i think, too many virtual calls
	virtual int SnapNumItems(int SnapID) const = 0;
	virtual const void *SnapFindItem(int SnapID, int Type, int ID) const = 0;
	virtual const void *SnapGetItem(int SnapID, int Index, CSnapItem *pItem) const = 0;
	virtual void SnapInvalidateItem(int SnapID, int Index) = 0;

	virtual void *SnapNewItem(int Type, int ID, int Size) = 0;

	virtual void SnapSetStaticsize(int ItemType, int Size) = 0;

	virtual int SendMsg(CMsgPacker *pMsg, int Flags) = 0;

	template<class T>
	int SendPackMsg(T *pMsg, int Flags)
	{
		CMsgPacker Packer(pMsg->MsgID(), false);
		if(pMsg->Pack(&Packer))
			return -1;
		return SendMsg(&Packer, Flags);
	}
};

class IGameClient
{
protected:
public:
	virtual void OnRconLine(const char *pLine) = 0;
	virtual void OnNewSnapshot() = 0;
	virtual void OnMessage(int MsgID, CUnpacker *pUnpacker) = 0;

	virtual int OnSnapInput(int *pData) = 0;

	virtual const char *GetItemName(int Type) const = 0;
	virtual const char *Version() const = 0;
	virtual const char *NetVersion() const = 0;
	virtual const char *NetVersionHashUsed() const = 0;
	virtual const char *NetVersionHashReal() const = 0;
	virtual int ClientVersion() const = 0;

};

extern IGameClient *CreateGameClient();
#endif
