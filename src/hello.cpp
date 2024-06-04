#include "global.h"

int main( [[maybe_unused]] int argc, [[maybe_unused]] char * argv[] )
{
	const u8 answer = 0b101010;
	print( "Hello {0}{1}\nThe answer is {2}, not {3} (short: {3:0.3}).\n", "World", "!", answer, numbers::pi );

	IfDebug print( "\U0001F44B{1}{0}\n\n", "!", "\U0001F30D" );

	print( "Reading whole vcpkg.json from base folder:\n" );
	std::ifstream      file( BasePath "vcpkg.json" );
	std::stringstream  buffer;
	buffer << file.rdbuf();
	print( "{}", buffer.str() );

	print_once( "Enough, " );
	print_once( "Enough, " ); // will no be printed as it was printed before
	print_once( "but this will again be shown\n" );
}
