// 定义新的signalex类,使其支持参数绑定机制
// 示例:
//  假如有类定义如下:
//	struct SigslotDemo : public sigslot::has_slots<> {
//		void Method0() { }
//		void Method1(int a) {	}
//		void Method2(int a, std::string s) { }
//		void Method3(int a, std::string s, int b) {	}
//		void Method4(int a, std::string s, int b, std::string d) { }
//	};
//
//  绑定示例(以signalex2为例,	其它个数的参数示例类似):
//  using namespace sigslot;
//	signalex2<int, int> event;
//	SigslotDemo demo;
//	event.connect_bind(&demo, &SigslotDemo::Method0);
//	event.connect_bind(&demo, &SigslotDemo::Method1, _2);
//	event.connect_bind(&demo, &SigslotDemo::Method2, _2, "sigslot");
//	event.connect_bind(&demo, &SigslotDemo::Method3, _1, "sigslot", _2);
//	event.connect_bind(&demo, &SigslotDemo::Method4, _1, "sigslot", _2, "hello");
//	event(1, 2);

#ifndef _SIGSLOT_BIND_C3C0E9E5_17E9_4B46_B7FC_D41509ACD812_
#define _SIGSLOT_BIND_C3C0E9E5_17E9_4B46_B7FC_D41509ACD812_

#include "base/sigslot.h"
#include "boost/mpl/at.hpp"
#include "boost/mpl/vector.hpp"
#include "boost/tuple/tuple.hpp"
#include "boost/preprocessor/repetition/enum.hpp"
#include "boost/preprocessor/repetition/enum_params.hpp"
#include "boost/preprocessor/repetition/repeat_from_to.hpp"
#include "boost/preprocessor/repetition/repeat_from_to.hpp"

namespace sigslot {

// 参数最大支持个数(signalex0~signalex8)
#define SIGNAL_MAX_PARAMS  9 

// 函数参数定义宏
#define SIGNAL_FUNCTION_PARAM(z, n, TUPLE)     \
	BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 0, TUPLE), n) BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 1, TUPLE), n)

// 占位符支持的个数(_1~_3)
#define SIGNAL_MAX_PLACEHOLDER  3

// 占位符定义
template<int I> 
struct uarg {
	uarg() {}
};

#define SIGNAL_placeholder(z, n, data)  \
	static uarg<n> BOOST_PP_CAT(_, BOOST_PP_INC(n));
	BOOST_PP_REPEAT_FROM_TO(0, SIGNAL_MAX_PLACEHOLDER, SIGNAL_placeholder, BOOST_PP_EMPTY)
#undef SIGNAL_placeholder

// arglist_type##n
struct empty_class {};

#define SIGNAL_operator(z, n, data)                                                                \
	BOOST_PP_CAT(arg_type, n) operator[] (BOOST_PP_CAT(BOOST_PP_CAT(uarg<, n), >)) const {           \
	  return BOOST_PP_CAT(BOOST_PP_CAT(storage_type::get<, n), >)();                                 \
	}                                                                                                \
	BOOST_PP_CAT(arg_type, n) operator[] (BOOST_PP_CAT(BOOST_PP_CAT(uarg<, n), >) (*) ()) const {    \
	  return BOOST_PP_CAT(BOOST_PP_CAT(storage_type::get<, n), >)();                                 \
	}

#define SIGNAL_invoker(z, n, alist)                                                                \
	BOOST_PP_CAT(BOOST_PP_CAT(alist[storage_type::get<, n), >()])

#define SIGNAL_arglist(z, n, data)                                                                 \
template<BOOST_PP_ENUM_PARAMS(n, typename arg_type)                                                \
	BOOST_PP_COMMA_IF(n) typename dummy = empty_class>                                               \
class BOOST_PP_CAT(arglist_type, n) : public boost::tuple<BOOST_PP_ENUM_PARAMS(n, arg_type)> {     \
	typedef boost::tuple<BOOST_PP_ENUM_PARAMS(n, arg_type)> storage_type;														 \
public:																																														 \
	explicit BOOST_PP_CAT(arglist_type, n)(BOOST_PP_ENUM(n, SIGNAL_FUNCTION_PARAM, (arg_type, a)))	 \
	  : storage_type(BOOST_PP_ENUM_PARAMS(n, a)) {}																									 \
	                                                                                                 \
	BOOST_PP_REPEAT(n, SIGNAL_operator, BOOST_PP_EMPTY)																							 \
	template< typename T > T & operator[] (T & var) const {  return var; }													 \
	template< typename T > T const & operator[] (T const & var) const { return var; }		    				 \
	                                                                                                 \
	template<typename F, typename L>																																 \
	void operator()(F& f, L& l) { f(BOOST_PP_ENUM(n, SIGNAL_invoker, l)); }                          \
	template<typename F, typename L>                                                                 \
	void operator()(F& f, L& l) const { f(BOOST_PP_ENUM(n, SIGNAL_invoker, l)); }                    \
};

BOOST_PP_REPEAT(SIGNAL_MAX_PARAMS, SIGNAL_arglist, BOOST_PP_EMPTY)

#undef SIGNAL_operator
#undef SIGNAL_invoker
#undef SIGNAL_arglist

// mem_fn_wrapper##n
#define SIGNAL_memfn(z, n, data)                                                                   \
template<typename dest_type BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename arg_type)>       \
struct BOOST_PP_CAT(mem_fn_wrapper, n) {																													 \
	typedef dest_type dest_type;                                                                     \
	typedef void (dest_type::*method_type)(BOOST_PP_ENUM_PARAMS(n, arg_type));											 \
	explicit BOOST_PP_CAT(mem_fn_wrapper, n)(method_type method, dest_type* object)                  \
	  : method_(method), object_(object) {}                                                          \
	void operator()(BOOST_PP_ENUM(n, SIGNAL_FUNCTION_PARAM, (arg_type, a))) const {									 \
	  (object_->*method_)(BOOST_PP_ENUM_PARAMS(n, a));                                               \
	}                                                                                                \
	method_type   method_;                                                                           \
	dest_type*    object_;                                                                           \
};

BOOST_PP_REPEAT(SIGNAL_MAX_PARAMS, SIGNAL_memfn, BOOST_PP_EMPTY)

#undef SIGNAL_memfn

// connectionex##n
#define SIGNAL_typelist(z, n, typelist)  \
	typename boost::mpl::at<typelist, boost::mpl::int_<n> >::type

#define SIGNAL_typedef(z, n, typelist)   \
	typedef SIGNAL_typelist(z, n, typelist) BOOST_PP_CAT(arg_type, n);

#define SIGNAL_connect(z, n, data)																																 \
template<typename typelist, typename functor_type, typename arglist_type, typename mt_policy>			 \
class BOOST_PP_CAT(connectionex, n) : public BOOST_PP_CAT(_connection_base, n)<										 \
	BOOST_PP_ENUM(n, SIGNAL_typelist, typelist) BOOST_PP_COMMA_IF(n) mt_policy>											 \
{																																																	 \
	BOOST_PP_REPEAT(n, SIGNAL_typedef, typelist)																										 \
	typedef BOOST_PP_CAT(_connection_base, n)<BOOST_PP_ENUM_PARAMS(n, arg_type)											 \
	  BOOST_PP_COMMA_IF(n) mt_policy> base_connection;																							 \
public:																																														 \
	BOOST_PP_CAT(connectionex, n)(functor_type functor, arglist_type const& list_av)  							 \
	  : functor_(functor), list_av_(list_av){ }																											 \
																																																	 \
	virtual base_connection* clone() {																															 \
		return new BOOST_PP_CAT(connectionex, n)(*this);																							 \
	}																																																 \
																																																	 \
	virtual base_connection* duplicate(has_slots<mt_policy>* pnewdest) {														 \
		functor_type new_functor(functor_.method_, (typename functor_type::dest_type*)pnewdest);			 \
		return new BOOST_PP_CAT(connectionex, n)(new_functor, list_av_);															 \
	}																																																 \
																																																	 \
	virtual void emit(BOOST_PP_ENUM(n, SIGNAL_FUNCTION_PARAM, (arg_type, a))) {						  				 \
		BOOST_PP_CAT(arglist_type, n)<BOOST_PP_ENUM_PARAMS(n, arg_type)>  arg  =                       \
		  BOOST_PP_CAT(arglist_type, n)<BOOST_PP_ENUM_PARAMS(n, arg_type)>(BOOST_PP_ENUM_PARAMS(n, a));\
		list_av_(functor_, arg);																																			 \
	}																																																 \
																																																	 \
	virtual has_slots<mt_policy>* getdest() const {																									 \
		return functor_.object_;																																			 \
	}																																																 \
																																																	 \
private:																																													 \
	functor_type  functor_;																																					 \
	arglist_type  list_av_;																																					 \
};

BOOST_PP_REPEAT(SIGNAL_MAX_PARAMS, SIGNAL_connect, BOOST_PP_EMPTY)

#undef SIGNAL_typelist
#undef SIGNAL_typedef
#undef SIGNAL_connect

// signalex##n
#define SIGNAL_method(z, n, connectex)																														 \
	template<typename desttype BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename A)              \
	  BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename T)>																		   \
	void connect_bind(desttype* object, void (desttype::*method)(BOOST_PP_ENUM_PARAMS(n, A)) 						 \
		 BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM(n, SIGNAL_FUNCTION_PARAM, (T, arg)))                       \
	{                                                                                                \
		typedef BOOST_PP_CAT(mem_fn_wrapper, n)<desttype                                               \
      BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, A)> functor_type;                               \
		typedef BOOST_PP_CAT(arglist_type, n)<BOOST_PP_ENUM_PARAMS(n, T)> arglist_type;								 \
		typedef connectex<typelist, functor_type, arglist_type, mt_policy> bind_type;                  \
		bind_type* conn = new bind_type(functor_type(method, object),                                  \
		                                arglist_type(BOOST_PP_ENUM_PARAMS(n, arg)));                   \
		lock_block<mt_policy> lock(this);                                                              \
		m_connected_slots.push_back(conn);                                                             \
		object->signal_connect(this);                                                                  \
	}

#define SIGNAL_signal(z, n, data)                                                                  \
	template<BOOST_PP_ENUM_PARAMS(n, typename arg_type) BOOST_PP_COMMA_IF(n)                         \
    typename mt_policy = SIGSLOT_DEFAULT_MT_POLICY>                                                \
	class BOOST_PP_CAT(signalex, n) :                                                                \
		public BOOST_PP_CAT(signal, n)<BOOST_PP_ENUM_PARAMS(n, arg_type) BOOST_PP_COMMA_IF(n) mt_policy>   \
  {																																															   \
		typedef boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, arg_type)> typelist;                        \
	public:																																													 \
		BOOST_PP_REPEAT_FROM_TO(0, SIGNAL_MAX_PARAMS, SIGNAL_method, BOOST_PP_CAT(connectionex, n))		 \
	};

BOOST_PP_REPEAT(SIGNAL_MAX_PARAMS, SIGNAL_signal, BOOST_PP_EMPTY)

#undef SIGNAL_method
#undef SIGNAL_signal
}

#endif//_SIGSLOT_BIND_C3C0E9E5_17E9_4B46_B7FC_D41509ACD812_
