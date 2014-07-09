/*
 *  平成14-24年（独）産業技術総合研究所 著作権所有
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
 *  Copyright 2002-2012.
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
 *  $Id$
 */
/*!
  \file		Filter2.h
  \brief	水平/垂直方向に分離不可能な2次元フィルタを表すクラスの定義
*/
#ifndef __TU_FILTER2_H
#define __TU_FILTER2_H

#include "TU/iterator.h"
#if defined(USE_TBB)
#  include <tbb/parallel_for.h>
#  include <tbb/blocked_range.h>
#endif

namespace TU
{
/************************************************************************
*  class Filter2							*
************************************************************************/
//! 水平/垂直方向に分離不可能な2次元フィルタを表すクラス
class Filter2
{
  public:
#if defined(USE_TBB)
  private:
    template <class IN, class OUT>
    class FilterRows
    {
      public:
	FilterRows(IN const& in, OUT const& out) :_in(in), _out(out)	{}

	void	operator ()(const tbb::blocked_range<size_t>& r) const
		{
		    IN	ib = _in, ie = _in;
		    std::advance(ib, r.begin());
		    std::advance(ie, r.end());
		    OUT	out = _out;
		    std::advance(out, r.begin());
		    Filter2::filterRows(ib, ie, out);
		}

      private:
	IN     const&	_in;
	OUT    const&	_out;
    };
#endif
  public:
    Filter2()	:_grainSize(1)				{}

    template <class IN, class OUT>
    void	operator ()(IN ib, IN ie, OUT out) const;
    size_t	grainSize()			const	{ return _grainSize; }
    void	setGrainSize(size_t gs)			{ _grainSize = gs; }

  private:
    template <class IN, class OUT>
    static void	filterRows(IN ib, IN ie, OUT out)	;
    
  private:
    size_t	_grainSize;
};
    
//! 与えられた2次元配列にこのフィルタを適用する
/*!
  \param ib	入力2次元データ配列の先頭行を指す反復子
  \param ie	入力2次元データ配列の末尾の次の行を指す反復子
  \param out	出力2次元データ配列の先頭行を指す反復子
*/
template <class IN, class OUT> void
Filter2::operator ()(IN ib, IN ie, OUT out) const
{
#if defined(USE_TBB)
    tbb::parallel_for(tbb::blocked_range<size_t>(0, std::distance(ib, ie),
						 _grainSize),
		      FilterRows<IN, OUT>(ib, out));
#else
    filterRows(ib, ie, out);
#endif
}

template <class IN, class OUT> void
Filter2::filterRows(IN ib, IN ie, OUT out)
{
    for (; ib != ie; ++ib, ++out)
	std::copy(ib->begin(), ib->end(), out->begin());
}

}
#endif	// !__TU_FILTER2_H
