#include "global.h"

using namespace JanSordid::Core;

int main( [[maybe_unused]] int argc, [[maybe_unused]] char * argv[] )
{
	const u8 answer = 0b101010;
	print( "Hello {0}{1}\nThe answer is {2}, not {3} (short: {3:0.3}).\n", "World", "!", answer, numbers::pi );

	IfNotFinal print( "\U0001F44B{1}{0}\n\n", "!", "\U0001F30D" );

	print_once( "Enough, " );
	print_once( "Enough, " ); // will not be printed as it was printed before
	print_once( "but this will again be shown\n" );

	{
		std::ifstream file( BasePath "vcpkg.json" );
		std::stringstream buffer;
		buffer << file.rdbuf();
		print( "{}", buffer.str() );
	}

	using NFD = JanSordid::NativeFileDialog;

	NFD::Result fileResult = NFD::OpenDialog();
	switch( fileResult.code )
	{
		case NFD::Code::Okay:
		{
			print( "The picked file is {}\n", fileResult.path );
			print( "Contents of this file are:\n" );
			std::ifstream file( fileResult.path );
			std::stringstream buffer;
			buffer << file.rdbuf();
			print( "{}", buffer.str() );
			break;
		}
		case NFD::Code::Error:
			print( "An error occurred while picking a file. Error is {}", fileResult.path );
			break;

		case NFD::Code::Cancel:
			print( "The file picker was canceled" );
			break;
	}
}
