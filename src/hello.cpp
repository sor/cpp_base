#include "global.hpp"

#include <variant>
#include <span>

#include <gtkmm.h>


using namespace JanSordid::Core;

template <typename T, T v>
struct IntegralConstant
{
    static constexpr T value = v;
    using value_type = T;
    using type       = IntegralConstant<T, v>;

    constexpr operator value_type  () const noexcept { return value; }
    constexpr value_type operator()() const noexcept { return value; }
};

using TrueType  = IntegralConstant<bool, true>;
using FalseType = IntegralConstant<bool, false>;

template <typename T, typename U> struct IsSameImpl       : public FalseType { };
template <typename T>             struct IsSameImpl<T, T> : public  TrueType { };
template <typename T, typename U> constexpr bool IsSameV      = IsSameImpl<T, U>::value;








template <typename T, typename U> constexpr bool IsSame                 = false;
template <typename T>             constexpr bool IsSame<T, T>           = true;

template <int T, int U> constexpr bool IsNearlySameValue            = false;
template <int T>        constexpr bool IsNearlySameValue<T,T>       = true;
template <int T>        constexpr bool IsNearlySameValue<T-1,T>     = true;
template <int T>        constexpr bool IsNearlySameValue<T,T-1>     = true;


template <typename T>
class MyClass
{
public:
	bool TIsInt  = IsSame<T,int>;
	bool TIsChar = IsSame<T,char>;
	// ...
};

int mainZZ()
{
	MyClass<char> mc;
	MyClass<int>  mi;
	Assert(mc.TIsInt == false);
	Assert(mi.TIsInt == true);
	return EXIT_SUCCESS;
}




template <typename T>             constexpr bool IsConst                = false;
template <typename T>             constexpr bool IsConst<const T>       = true;
template <typename T>             constexpr bool IsVolatile             = false;
template <typename T>             constexpr bool IsVolatile<volatile T> = true;
template <typename T>             constexpr bool IsReference            = false;
template <typename T>             constexpr bool IsReference<T&>        = true;
template <typename T>             constexpr bool IsReference<T&&>       = true;

//template <typename T> using RemoveConst = T;
//template <typename T> using RemoveConst<const T> = T;

static_assert(IsSameImpl<long,i64>::value);
static_assert(IsSameV<long,i64>);
static_assert(IsSame<long,i64>);

template <bool B, typename T = void> struct EnableIfImpl          {};
template <typename T>                struct EnableIfImpl<true, T> { typedef T type; };
template <bool B, typename T = void> using EnableIfT = typename EnableIfImpl<B, T>::type;



template <typename T, typename U>
EnableIfT<IsSame<T,U>> myFunc(T t, U u) {

}

template <u8 N> u64 fib = fib<N-1> + fib<N-2>;
template <>     u64 fib<1> = 1;
template <>     u64 fib<0> = 0;


template <typename TK>
class MyThing
{
public:
    TK member = 1;

    void func()
    {
        member = member % 10;
    }
    void func2()
    {
        member = member * 2;
    }
};

template<>
void MyThing<float>::func()
{
    member = (int)member % 10;
}
/*
template <typename TKV>
class MyThing<TKV, TKV>
{
public:
    float member = 1;
    float member2 = 1;

    template<typename X>
    void func(X x)
    {
        member = (int)member % 10;
        func2();
    }
};
*/

float middle1 = 777;
auto compare1(float left, float right) -> bool {
	return left < right;
};

int emptyIntFunc() {
	return 42;
}







struct Point
{
    f32 x, y;

    bool operator<(const Point& other)
    {
        return this->x < other.x;
    }

    static bool sortByX(const Point& left, const Point& right)
    {
        return left.x == right.x
             ? left.y < right.y
             : left.x < right.x;
    }

    static bool sortByY(const Point& left, const Point& right)
    {
        return left.y < right.y;
    }
};


template <typename T>
class FrameAlloc {
    static T  pool[1024*1024];
    static T* pos = &pool;
    constexpr T * allocate(size_t n) {
        const size_t newPos = pos + n;
        assert( newPos < pool+(sizeof(pool)/sizeof(*pool)) );
        T* ret = pos;
        pos = newPos;
        return ret;
    }
    void deallocate(T* p, size_t n) {
        // Maybe call the destructor?
		// But no need to deallocate
    }
    void reset() {
        pos = &pool;
    }
};

#ifdef GTKMM_MAJOR_VERSION
class MyWindow : public Gtk::Window
{
    Gtk::Label label;

public:
    MyWindow()
        : label( "Testomania" )
    {
        set_title( "Basic application" );
        set_default_size( 200, 200 );
        set_child( label );
    }
};
#endif


int main( [[maybe_unused]] int argc, [[maybe_unused]] char * argv[] )
{
#ifdef GTKMM_MAJOR_VERSION
	SharedPtr<Gtk::Application> app = Gtk::Application::create( "de.sordid.examples" );
	app->make_window_and_run<MyWindow>( argc, argv );
#endif

	assert( argc >= 1 && "argc was too small" );

	DynArray<Point> vop = {{1,2}, {3,4}, {1,9}, {8,8}, {4,2}};

	DynArray<Point> vopx = vop; // To be sorted by X axis
	std::sort(
		vopx.begin(),
		vopx.end(),
		[](const Point& left, const Point& right)
		{
			return left.y < right.y;
		} );

	// processing

	std::sort(
		vopx.begin(),
		vopx.end(),
		[&vop](const Point& left, const Point& right)
		{
			return left.y > right.y;
		} );

	auto left  = std::span(vop).subspan(0, 2);
	auto right = std::span(vop).subspan(2, 3);

	for( auto e : right )
		(void)e.x;

	DynArray<Point> vopy = vop; // To be sorted by Y axis
	std::stable_sort( vopy.begin(), vopy.end(), Point::sortByX );
	std::stable_sort( vopy.begin(), vopy.end(), Point::sortByY );

	//myFunc(42.0f, 60);
	//MyThing<int,int>   thing;
	//MyThing<char,int>  thing2;
	MyThing<float> thingFloating;
	//thing.func();
	//thing2.func();
	thingFloating.func();
	thingFloating.func2();

	//print( "{}", fib<30> );

	const u8 answer = 0b101010;
	print( "Hello {0}{1}\nThe answer is {2}, not {3} (short: {3:1.3}).\n",
	       "World", "!", answer, std::numbers::pi_v<float> );

	enum class A : size_t { A };
	std::tuple<int> ti;
	std::get<std::to_underlying( A::A )>( ti );

	IfNotFinal print( "\U0001F44B{1}{0}\n\n", "!", "\U0001F30D" );

	print_once( "Enough, " );
	print_once( "Enough, " ); // will not be printed as it was printed before
	{
		std::ifstream file( BasePath "vcpkg.json" );
		std::stringstream buffer;
		buffer << file.rdbuf();
		print( "{}", buffer.str() );
	}

	//using NFD = JanSordid::NativeFileDialog;
	NFD::Guard g;

	NFD::UniquePath upath;
	auto result = NFD::OpenDialog( upath, NFD::EmptyFilter, 0, NFD::EmptyPath );

	switch( result )
	{
		case NFD_OKAY:
		{
			print( "The picked file is {}\n", upath.get() );
			print( "Contents of this file are:\n" );
			std::ifstream file( upath.get() );
			std::stringstream buffer;
			buffer << file.rdbuf();
			print( "{}", buffer.str() );
			break;
		}
		case NFD_ERROR:
			// This can be caused by pressing ESC or Alt+F4
			print( "An error occurred while picking a file. Error is: {}", NFD_GetError() );
			break;

		case NFD_CANCEL:
			print( "The file picker was canceled" );
			break;
	}

	return 0;
}
