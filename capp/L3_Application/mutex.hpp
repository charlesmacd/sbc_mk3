
#ifndef _MUTEX_HPP_
#define _MUTEX_HPP_

#ifdef __cpluslpus
extern "C" {
#endif

#include <stdint.h>
#include "../L0_Platform/cpu_asm.hpp"

class Mutex
{
	volatile uint8_t flag;
public:

	Mutex()
	{
		flag = 0;
	}

	/* Lock mutex (will block if waiting for prior unlock) */
	bool lock(void)
	{
		while(__atomic_tas(&flag))
		{
			/* Spin (TODO: Issue STOP command) */
			;
		}
		return true;
	}

	bool attempt_lock(void)
	{
		if(__atomic_tas(&flag))
		{
			// it was already locked
			return false;
		}
		else
		{
			// we've now locked it
			return true;
		}
	}

	/* Test if mutex is locked */
	bool is_locked(void)
	{
		return (flag & 0x80);
	}

	/* Unlock mutex */
	bool unlock(void)
	{
		flag = 0;
		return true;
	}
}; /* class Mutex */

#ifdef __cpluslpus
}
#endif

#endif /* _MUTEX_HPP_ */
