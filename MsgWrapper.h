
#ifndef _MSGWRAPPER_H_
#define _MSGWRAPPER_H_

#include "common.h"
#include "CMemoryPool.h"

#define MSG_HEADER_LENGHTH        (4 + 4 + 4 + 4 + 4)
#define DEFAULT_EXPAND_MSG_SIZE   (1024 * 16 - MSG_HEADER_LENGHTH)

class CMsgBuf {
public:
	CMsgBuf();

	CMsgBuf(uint32_t msg,
		     uint32_t msgType,
		     uint32_t srcId,
		     uint32_t dstId,
		     uint32_t exMsgSize,
		     uint8_t *exMsgBuf);

	CMsgBuf(const CMsgBuf& msgBuf);

	CMsgBuf& operator=(const CMsgBuf& msgBuf);

	~CMsgBuf();

	void SetVersion(uint16_t version);

	void SetMsg(uint32_t msg);

	void SetMsgType(uint32_t msgType);

	void SetSrcId(uint32_t srcId);

	void SetDstId(uint32_t dstId);

	void SetRemoteIP(PString remoteIP);

	void SetExMsgSize(uint32_t exMsgSize);

	void CopyExMsgBuf(uint8_t *buf, uint32_t exMsgSize = 0);

	uint16_t GetMsgVersion() const;

	uint32_t GetMsg() const;

	uint32_t GetMsgType() const;

	uint32_t GetSrcId() const;

	uint32_t GetDstId() const;

	uint32_t GetExMsgSize() const;

	uint8_t* GetExMsgBuf() const;

	static uint32_t GetMsgHeaderSize();

	uint32_t GetMsgSize() const;

	PString GetRemoteIP() const;

	bool IsAvailableMsg();

	bool MakeMsg(uint8_t *rawData, uint32_t len);

	bool MakeMsg(uint32_t msg,
		     uint32_t msgType,
		     uint32_t srcId,
		     uint32_t dstId,
		     uint32_t exMsgSize,
		     uint8_t *exMsgBuf);

	uint32_t MakeRawData(uint8_t *rawData, uint32_t len);

private:
	void Init(const CMsgBuf *org);

	void MakeExMsgEndFlag();

private:
	uint16_t m_version;
        PString m_remoteIP;
	uint32_t m_msg;
	uint32_t m_msgType;
	uint32_t m_srcId;
	uint32_t m_dstId;
	uint32_t m_exMsgSize;  // expand message size in msgBuf
	uint8_t  m_fixBuf[DEFAULT_EXPAND_MSG_SIZE];
	uint8_t *m_exMsgBuf;
};

#define CHECK_R(bytes) \
{ \
	if (m_ptr + bytes > m_orgPtr + m_bufLen) \
{ \
	assert(0); \
	return 0;  \
} \
}

#define CHECK_W(bytes) \
{ \
	if (m_ptr + bytes > m_orgPtr + m_bufLen) \
{ \
	assert(0); \
	return; \
} \
}

class CStreamOperate {
public:
	CStreamOperate(uint8_t *buf, uint32_t len)
	{
		m_orgPtr = m_ptr = buf;
		m_bufLen = len;
	}

	CStreamOperate(CMsgBuf& msgBuf)
	{
		m_orgPtr = m_ptr = msgBuf.GetExMsgBuf();
		m_bufLen = msgBuf.GetExMsgSize();
	}

	~CStreamOperate() { }

	uint32_t GetIOSize()
	{
		return m_ptr - m_orgPtr;
	}

	inline void Check(int bytes) { }

	inline uint8_t ReadU8()
	{
		CHECK_R(1);
		return *m_ptr++;
	}

	inline uint16_t ReadU16()
	{
		CHECK_R(2);
		int b0 = *m_ptr++;
		int b1 = *m_ptr++;
		return b0 << 8 | b1;
	}

	inline uint32_t ReadU32()
	{
		CHECK_R(4);
		int b0 = *m_ptr++;
		int b1 = *m_ptr++;
		int b2 = *m_ptr++;
		int b3 = *m_ptr++;
		return b0 << 24 | b1 << 16 | b2 << 8 | b3;
	}

	inline int8_t  ReadS8() { return (int8_t)ReadU8(); }

	inline int16_t ReadS16() { return (int16_t)ReadU16(); }

	inline int32_t ReadS32() { return (int32_t)ReadU32(); }

	inline char* ReadString()
	{
		uint32_t len = ReadU32();
		assert(len < 65535);
		char *str = new char[len + 1];
		str[0] = 0;
		ReadBytes(str, len);
		str[len] = 0;
		return str;
	}

	inline int ReadBytes(void *data, int length)
	{
		CHECK_R(length);
		memcpy(data, m_ptr, length);
		m_ptr += length;
		return length;
	}

	inline void WriteU8(uint8_t u)
	{
		CHECK_W(1);
		*m_ptr++ = u;
	}

	inline void WriteU16(uint16_t u)
	{
		CHECK_W(2);
		*m_ptr++ = u >> 8;
		*m_ptr++ = (uint8_t)u;
	}

	inline void WriteU32(uint32_t u)
	{
		CHECK_W(4);
		*m_ptr++ = u >> 24;
		*m_ptr++ = u >> 16;
		*m_ptr++ = u >> 8;
		*m_ptr++ = u;
	}

	inline void WriteS8(int8_t s) { WriteU8((uint8_t)s); }

	inline void WriteS16(int16_t s) { WriteU16((uint16_t)s); }

	inline void WriteS32(int32_t s) { WriteU32((uint32_t)s); }

	inline void WriteString(const char *str)
	{
		uint32_t len = strlen(str);
		CHECK_W(len + 1);
		WriteU32(len);
		WriteBytes(str, len);
	}

	inline void WriteBytes(const void *data, int length)
	{
		CHECK_W(length);
		memcpy(m_ptr, data, length);
		m_ptr += length;
	}

private:
	uint8_t *m_orgPtr;
	uint8_t *m_ptr;
	uint32_t m_bufLen;
};

#endif//_MSGWRAPPER_H_

