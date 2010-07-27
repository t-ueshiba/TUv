/*
 *  平成14-19年（独）産業技術総合研究所 著作権所有
 *  
 *  創作者：植芝俊夫
 *
 *  本プログラムは（独）産業技術総合研究所の職員である植芝俊夫が創作し，
 *  （独）産業技術総合研究所が著作権を所有する秘密情報です．著作権所有
 *  者による許可なしに本プログラムを使用，複製，改変，第三者へ開示する
 *  等の行為を禁止します．
 *  
 *  このプログラムによって生じるいかなる損害に対しても，著作権所有者お
 *  よび創作者は責任を負いません。
 *
 *  Copyright 2002-2007.
 *  National Institute of Advanced Industrial Science and Technology (AIST)
 *
 *  Creator: Toshio UESHIBA
 *
 *  [AIST Confidential and all rights reserved.]
 *  This program is confidential. Any using, copying, changing or
 *  giving any information concerning with this program to others
 *  without permission by the copyright holder are strictly prohibited.
 *
 *  [No Warranty.]
 *  The copyright holder or the creator are not responsible for any
 *  damages caused by using this program.
 *
 *  $Id: Thread++.h,v 1.10 2010-07-27 05:30:13 ueshiba Exp $  
 */
#ifndef __TUThreadPP_h
#define __TUThreadPP_h

#include <pthread.h>
#include "TU/Array++.h"

namespace TU
{
/************************************************************************
*  class Thread								*
************************************************************************/
class __PORT Thread
{
  public:
    void		wait()					const	;
    
  protected:
    Thread()								;
    virtual ~Thread()							;

    void		preRaise()				const	;
    void		postRaise()				const	;

  private:
    virtual void	doJob()						= 0;
    static void*	threadProc(void* thread)			;
    
    enum State		{Ready, Idle, Exit};

    mutable pthread_mutex_t	_mutex;
    mutable pthread_cond_t	_cond;
    pthread_t			_thread;
    mutable State		_state;
};

inline void
Thread::preRaise() const
{
    pthread_mutex_lock(&_mutex);
}

inline void
Thread::postRaise() const
{
    _state = Ready;
    pthread_cond_signal(&_cond);	// Send Ready signal to the child.
    pthread_mutex_unlock(&_mutex);
}

/************************************************************************
*  class MultiThread<OP, DATA>						*
************************************************************************/
template <class OP, class DATA>
class MultiThread : public OP
{
  private:
    class OperatorThread : public Thread
    {
      public:
	OperatorThread()
	    :Thread(), _op(0), _data(0), _is(0), _ie(0)			{}

	void		raise(const OP& op, DATA& data,
			      u_int is, u_int ie)		const	;

      private:
	virtual void	doJob()						;

	mutable const OP*	_op;
	mutable DATA*		_data;
	mutable u_int		_is, _ie;
    };

  public:
    MultiThread(u_int nthreads=1)	:OP(), _threads(nthreads)	{}

    void	createThreads(u_int nthreads)				;
    void	operator ()(DATA& data)				const	;
    
  private:
    Array<OperatorThread>	_threads;
};

template <class OP, class DATA> inline void
MultiThread<OP, DATA>::createThreads(u_int nthreads)
{
    _threads.resize(nthreads);
}

template <class OP, class DATA> inline void
MultiThread<OP, DATA>::operator ()(DATA& data) const
{
    const u_int	d = data.dim() / _threads.dim();
    for (u_int is = 0, n = 0; n < _threads.dim(); ++n)
    {
	const u_int	ie = (n < _threads.dim() - 1 ? is + d : data.dim());
	_threads[n].raise(*this, data, is, ie);
	is = ie;
    }
    for (u_int n = 0; n < _threads.dim(); ++n)
	_threads[n].wait();
}

template <class OP, class DATA> inline void
MultiThread<OP, DATA>::OperatorThread::raise(const OP& op, DATA& data,
					     u_int is, u_int ie) const
{
    preRaise();
    _op	  = &op;
    _data = &data;
    _is	  = is;
    _ie	  = ie;
    postRaise();
}

template <class OP, class DATA> void
MultiThread<OP, DATA>::OperatorThread::doJob()
{
    (*_op)(*_data, _is, _ie);
}

/************************************************************************
*  class MultiThread2<OP, INPUT, OUTPUT>				*
************************************************************************/
template <class OP, class INPUT, class OUTPUT=INPUT>
class MultiThread2 : public OP
{
  private:
    class OperatorThread : public Thread
    {
      public:
	OperatorThread()
	    :Thread(), _op(0), _in(0), _out(0), _is(0), _ie(0)		{}

	void		raise(const OP& op, const INPUT& in,
			      OUTPUT& out, u_int is, u_int ie)	const	;

      private:
	virtual void	doJob()						;

	mutable const OP*	_op;
	mutable const INPUT*	_in;
	mutable OUTPUT*		_out;
	mutable u_int		_is, _ie;
    };

  public:
    MultiThread2(u_int nthreads=1)	:OP(), _threads(nthreads)	{}

    void	createThreads(u_int nthreads)				;
    void	operator ()(const INPUT& in, OUTPUT& out)	const	;
    
  private:
    void	raiseThreads(const INPUT& in, OUTPUT& out)	const	;
    
    Array<OperatorThread>	_threads;
};

template <class OP, class INPUT, class OUTPUT> inline void
MultiThread2<OP, INPUT, OUTPUT>::createThreads(u_int nthreads)
{
    _threads.resize(nthreads);
}

template <class OP, class INPUT, class OUTPUT> void
MultiThread2<OP, INPUT, OUTPUT>::raiseThreads(const INPUT& in,
					      OUTPUT& out) const
{
    const u_int	d = out.dim() / _threads.dim();
    for (u_int is = 0, n = 0; n < _threads.dim(); ++n)
    {
	const u_int	ie = (n < _threads.dim() - 1 ? is + d : out.dim());
	_threads[n].raise(*this, in, out, is, ie);
	is = ie;
    }
    for (u_int n = 0; n < _threads.dim(); ++n)
	_threads[n].wait();
}

template <class OP, class INPUT, class OUTPUT> inline void
MultiThread2<OP, INPUT, OUTPUT>::OperatorThread::raise(const OP& op,
						       const INPUT& in,
						       OUTPUT& out,
						       u_int is, u_int ie) const
{
    preRaise();
    _op  = &op;
    _in  = &in;
    _out = &out;
    _is  = is;
    _ie  = ie;
    postRaise();
}

template <class OP, class INPUT, class OUTPUT> void
MultiThread2<OP, INPUT, OUTPUT>::OperatorThread::doJob()
{
    (*_op)(*_in, *_out, _is, _ie);
}

}
#endif	// !__TUThreadPP_h
