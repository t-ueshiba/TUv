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
 *  $Id: BoxFilter.h 1909 2016-01-03 09:06:30Z ueshiba $
 */
/*!
  \file		BoxFilter.h
  \brief	box filterに関するクラスの定義と実装
*/
#ifndef	__TU_BOXFILTER_H
#define	__TU_BOXFILTER_H

#include "TU/Filter2.h"
#include "TU/Array++.h"

namespace TU
{
/************************************************************************
*  class box_filter_iterator<ITER, T>					*
************************************************************************/
//! コンテナ中の指定された要素に対してbox filterを適用した結果を返す反復子
/*!
  \param ITER	コンテナ中の要素を指す定数反復子の型
*/
template <class ITER, class T=iterator_value<ITER> >
class box_filter_iterator
    : public boost::iterator_adaptor<box_filter_iterator<ITER, T>,
				     ITER,			// base
				     T,				// value_type
				     boost::single_pass_traversal_tag>
{
  private:
    using super = boost::iterator_adaptor<box_filter_iterator,
					  ITER,
					  T,
					  boost::single_pass_traversal_tag>;
		    
  public:
    using	typename super::value_type;
    using	typename super::reference;

    friend class	boost::iterator_core_access;

  public:
		box_filter_iterator()
		    :super(), _head(super::base()), _val(), _valid(true)
		{
		}
    
		box_filter_iterator(const ITER& iter, size_t w=0)
		    :super(iter), _head(iter), _val(), _valid(true)
		{
		    if (w > 0)
		    {
			_val = *super::base();
				
			while (--w > 0)
			    _val += *++super::base_reference();
		    }
		}

    void	initialize(const ITER& iter, size_t w=0)
		{
		    super::base_reference() = iter;
		    _head = iter;
		    _valid = true;

		    if (w > 0)
		    {
			_val = *super::base();
				
			while (--w > 0)
			    _val += *++super::base_reference();
		    }
		}
    
  private:
    using value_is_expr	= std::integral_constant<bool, rank<value_type>() != 0>;

    template <class VITER_, class ITER_>
    static void	update(const VITER_& val,
		       const ITER_& curr, const ITER_& head, std::false_type)
		{
		    *val += (*curr - *head);
		}
    template <class VITER_, class ITER_>
    static void	update(const VITER_& val,
		       const ITER_& curr, const ITER_& head, std::true_type)
		{
		    using iterator	= subiterator<VITER_>;
		    using value_is_expr	= std::integral_constant<
					      bool,
					      rank<iterator_value<iterator> >()
					      != 0>;
		    
		    auto	c = curr->cbegin();
		    auto	h = head->cbegin();
		    for (iterator v = val->begin(), ve = val->end();
			 v != ve; ++v, ++c, ++h)
			update(v, c, h, value_is_expr());
		}

    reference	dereference() const
		{
		    if (!_valid)
		    {
			update(&_val, super::base(), _head, value_is_expr());
			++_head;
			_valid = true;
		    }
		    return _val;
		}
    
    void	increment()
		{
		  // dereference() せずに increment() する可能性があ
		  // るなら次のコードを有効化する．ただし，性能は低下．
#ifdef TU_BOX_FILTER_ITERATOR_CONSERVATIVE
		    if (!_valid)
		    {
			update(&_val, super::base(), _head, value_is_expr());
			++_head;
		    }
		    else
#endif
			_valid = false;
		    ++super::base_reference();
		}

  private:
    mutable ITER	_head;
    mutable value_type	_val;	// [_head, base()) or [_head, base()] の総和
    mutable bool	_valid;	// _val が [_head, base()] の総和ならtrue
};

//! box filter反復子を生成する
/*!
  \param iter	コンテナ中の要素を指す定数反復子
  \return	box filter反復子
*/
template <class T=void, class ITER>
box_filter_iterator<ITER, std::conditional_t<std::is_void<T>::value,
					     iterator_value<ITER>, T> >
make_box_filter_iterator(ITER iter, size_t w=0)
{
    return {iter, w};
}

/************************************************************************
*  class BoxFilter							*
************************************************************************/
//! 1次元入力データ列にbox filterを適用するクラス
class BoxFilter
{
  public:
  //! box filterを生成する．
  /*!
    \param w	box filterのウィンドウ幅
   */	
		BoxFilter(size_t w=3) :_winSize(w)	{}
    
  //! box filterのウィンドウ幅を設定する．
  /*!
    \param w	box filterのウィンドウ幅
    \return	このbox filter
   */
    BoxFilter&	setWinSize(size_t w)		{_winSize = w; return *this;}

  //! box filterのウィンドウ幅を返す．
  /*!
    \return	box filterのウィンドウ幅
   */
    size_t	winSize()		const	{return _winSize;}

    template <class T=void, class IN, class OUT>
    void	convolve(IN ib, IN ie, OUT out)	const	;

  //! 与えられた長さの入力データ列に対する出力データ列の長さを返す
  /*!
    \param inLen	入力データ列の長さ
    \return		出力データ列の長さ
   */
    size_t	outLength(size_t inLen)	const	{return inLen + 1 - _winSize;}
	
  private:
    size_t	_winSize;		//!< box filterのウィンドウ幅
};
    
/*!
  \param ib	1次元入力データ列の先頭を示す反復子
  \param ie	1次元入力データ列の末尾の次を示す反復子
  \param out	box filterを適用した出力データ列の先頭を示す反復子
  \return	出力データ列の末尾の次を示す反復子
*/
template <class T, class IN, class OUT> void
BoxFilter::convolve(IN ib, IN ie, OUT out) const
{
    std::copy(make_box_filter_iterator<T>(ib, _winSize),
	      make_box_filter_iterator<T>(ie), out);
}

/************************************************************************
*  class BoxFilter2							*
************************************************************************/
//! 2次元入力データ列にbox filterを適用するクラス
class BoxFilter2 : public Filter2<BoxFilter2>
{
  private:
    typedef Filter2<BoxFilter2>	super;
    
  public:
    using	super::grainSize;
    using	super::setGrainSize;
    
  //! box filterを生成する．
  /*!
    \param wrow	box filterのウィンドウの行幅(高さ)
    \param wcol	box filterのウィンドウの列幅(幅)
   */	
		BoxFilter2(size_t wrow=3, size_t wcol=3)
		    :super(*this), _rowWinSize(wrow), _colFilter(wcol)
		{
		    if (grainSize() < 2*_rowWinSize)
			setGrainSize(2*_rowWinSize);
		}
    
  //! box filterのウィンドウの行幅(高さ)を設定する．
  /*!
    \param wrow	box filterのウィンドウの行幅
    \return	このbox filter
   */
    BoxFilter2&	setRowWinSize(size_t wrow)
		{
		    _rowWinSize = wrow;
		    if (grainSize() < 2*_rowWinSize)
			setGrainSize(2*_rowWinSize);
		    return *this;
		}

  //! box filterのウィンドウの列幅(幅)を設定する．
  /*!
    \param wcol	box filterのウィンドウの列幅
    \return	このbox filter
   */
    BoxFilter2&	setColWinSize(size_t wcol)
		{
		    _colFilter.setWinSize(wcol);
		    return *this;
		}

  //! box filterのウィンドウ行幅(高さ)を返す．
  /*!
    \return	box filterのウィンドウの行幅
   */
    size_t	rowWinSize()	const	{ return _rowWinSize; }

  //! box filterのウィンドウ列幅(幅)を返す．
  /*!
    \return	box filterのウィンドウの列幅
   */
    size_t	colWinSize()	const	{ return _colFilter.winSize(); }

  //! 与えられた行幅(高さ)を持つ入力データ列に対する出力データ列の行幅を返す．
  /*!
    \param inRowLength	入力データ列の行幅
    \return		出力データ列の行幅
   */
    size_t	outRowLength(size_t inRowLength) const
		{
		    return inRowLength + 1 - rowWinSize();
		}
    
  //! 与えられた列幅(幅)を持つ入力データ列に対する出力データ列の列幅を返す．
  /*!
    \param inColLength	入力データ列の列幅
    \return		出力データ列の列幅
   */
    size_t	outColLength(size_t inColLength) const
		{
		    return inColLength + 1 - colWinSize();
		}

    size_t	overlap()	const	{ return rowWinSize() - 1; }

    template <class IN, class OUT>
    void	convolveRows(IN ib, IN ie, OUT out)	const	;
    
  private:
    size_t	_rowWinSize;
    BoxFilter	_colFilter;
};

//! 与えられた2次元配列とこのフィルタの畳み込みを行う
/*!
  \param ib	入力2次元データ配列の先頭行を指す反復子
  \param ie	入力2次元データ配列の末尾の次の行を指す反復子
  \param out	出力2次元データ配列の先頭行を指す反復子
*/
template <class IN, class OUT> void
BoxFilter2::convolveRows(IN ib, IN ie, OUT out) const
{
    typedef Array<iterator_value<subiterator<IN> > >	row_type;
    typedef box_filter_iterator<IN, row_type>		row_iterator;

    if (std::distance(ib, ie) < rowWinSize())
	throw std::runtime_error("BoxFilter2::convolveRows(): not enough rows!");
    
    for (row_iterator row(ib, _rowWinSize), rowe(ie); row != rowe; ++row, ++out)
	_colFilter.convolve(row->begin(), row->end(), out->begin());
}

}
#endif	// !__TU_BOXFILTER_H