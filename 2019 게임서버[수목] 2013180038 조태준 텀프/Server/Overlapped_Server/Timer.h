#pragma once

#ifndef __TIMER_H__
#define __TIMER_H__

#include "Common.h"
#include "Client.h"

typedef struct stTimerEvent {
	ULONGLONG		s_time;
	e_EventType		type;
	CChessClient*   client;
	DWORD			targetID;
} TimerEvent, TIMEREVENT;

struct TimeComparison
{
	bool operator() (const TimerEvent& lhs, const TimerEvent& rhs) const
	{
		return (lhs.s_time > rhs.s_time);
	}
};

typedef priority_queue <TimerEvent, vector<TimerEvent>, TimeComparison> TimerQueue;

class CTimer
{
public:
	CTimer() {};
	~CTimer() {};
private:
	HANDLE			m_hIocp;
	TimerQueue		m_TimerQueue;
	mutex			m_Mutex;
public:
	const bool		Initialize(HANDLE hiocp);
	const bool		AddTimer(ULONGLONG s_time, e_EventType etimerType, CChessClient* pclient, DWORD wtargetID = 0);
	void			ProcessTimer();
};

#endif // !__TIMER_H__
