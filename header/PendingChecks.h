#pragma once
#include <queue>

// One instance per check type: detectors push a payload the moment they observe a completion,
// Mod retries the send every tick and confirms only once the socket accepted it - so checks
// that fire while the client is disconnected survive until the connection comes back.
template <typename T>
class PendingChecks
{
public:
	void push(const T& t_value)
	{
		m_pending.push(t_value);
	}

	bool hasPending() const
	{
		return !m_pending.empty();
	}

	const T& front() const
	{
		return m_pending.front();
	}

	void confirm()
	{
		if (!m_pending.empty())
		{
			m_pending.pop();
		}
	}

private:
	std::queue<T> m_pending;
};
