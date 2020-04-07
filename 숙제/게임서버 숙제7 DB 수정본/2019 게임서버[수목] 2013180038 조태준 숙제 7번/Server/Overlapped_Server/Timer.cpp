#pragma once

#include "Timer.h"

const bool CTimer::Initialize(HANDLE hiocp)
{
	m_hIocp = hiocp;

	return true;
}

const bool CTimer::AddTimer(ULONGLONG s_time, e_EventType etimerType, CChessClient* pclient, DWORD wtargetID)
{
	m_Mutex.lock();
	{
		m_TimerQueue.push(TimerEvent{ s_time, etimerType, pclient, wtargetID });
	}
	m_Mutex.unlock();
	return true;
}

void CTimer::ProcessTimer()
{
	while (true)
	{
		m_Mutex.lock();
		if (m_TimerQueue.empty() == true) {
			m_Mutex.unlock();
			break;
		}

		if (m_TimerQueue.top().s_time >= GetTickCount64()) {
			m_Mutex.unlock();
			break;
		}
		TimerEvent tevent = m_TimerQueue.top();
		m_TimerQueue.pop();
		m_Mutex.unlock();
		SOCKETINFO * poverlapped = new SOCKETINFO;
		poverlapped->targetid = tevent.targetID;
		poverlapped->eventType = tevent.type;

		PostQueuedCompletionStatus(m_hIocp, 1, reinterpret_cast<ULONG_PTR>(tevent.client), &poverlapped->overlapped);
	}
}
