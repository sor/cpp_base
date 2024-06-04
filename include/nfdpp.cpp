#include "nfdpp.h"

#include <nfd.h>        // https://github.com/mlabbe/nativefiledialog/blob/master/README.md

namespace JanSordid
{
	// This is in here, because nfdpp.h (therefore global.h) should not include nfd.h
	static_assert( static_cast<std::underlying_type_t<nfdresult_t>>( NativeFileDialog::Code::Error  ) == NFD_ERROR );
	static_assert( static_cast<std::underlying_type_t<nfdresult_t>>( NativeFileDialog::Code::Okay   ) == NFD_OKAY );
	static_assert( static_cast<std::underlying_type_t<nfdresult_t>>( NativeFileDialog::Code::Cancel ) == NFD_CANCEL );

	NativeFileDialog::Result
	NativeFileDialog::OpenDialog( const char * filterList, const char * defaultPath ) noexcept
	{
		Result ret;
		nfdchar_t * outPath = nullptr;
		ret.code = static_cast<Code>( NFD_OpenDialog( filterList, defaultPath, &outPath ) );
		if( ret.code == Code::Okay )
		{
			ret.path = outPath;
			free( outPath );
		}
		else if( ret.code == Code::Error )
		{
			ret.path = NFD_GetError();
		}
		return ret;
	}

	NativeFileDialog::ResultMultiple
	NativeFileDialog::OpenDialogMultiple( const char * filterList, const char * defaultPath ) noexcept
	{
		ResultMultiple ret;
		nfdpathset_t outPaths;
		ret.code = static_cast<Code>( NFD_OpenDialogMultiple( filterList, defaultPath, &outPaths ) );
		if( ret.code == Code::Okay )
		{
			const size_t count = NFD_PathSet_GetCount( &outPaths );
			ret.paths.reserve( count );
			for( size_t i = 0; i < count; ++i )
			{
				nfdchar_t * path = NFD_PathSet_GetPath( &outPaths, i );
				ret.paths.emplace_back( path );
			}
			NFD_PathSet_Free( &outPaths );
		}
		else if( ret.code == Code::Error )
		{
			ret.paths.emplace_back( NFD_GetError() );
		}
		return ret;
	}

	NativeFileDialog::Result
	NativeFileDialog::SaveDialog( const char * filterList, const char * defaultPath ) noexcept
	{
		Result ret;
		nfdchar_t * outPath = nullptr;
		ret.code = static_cast<Code>( NFD_SaveDialog( filterList, defaultPath, &outPath ) );
		if( ret.code == Code::Okay )
		{
			ret.path = outPath;
			free( outPath );
		}
		else if( ret.code == Code::Error )
		{
			ret.path = NFD_GetError();
		}
		return ret;
	}

	NativeFileDialog::Result
	NativeFileDialog::PickFolder( const char * defaultPath ) noexcept
	{
		Result ret;
		nfdchar_t * outPath = nullptr;
		ret.code = static_cast<Code>( NFD_PickFolder( defaultPath, &outPath ) );
		if( ret.code == Code::Okay )
		{
			ret.path = outPath;
			free( outPath );
		}
		else if( ret.code == Code::Error )
		{
			ret.path = NFD_GetError();
		}
		return ret;
	}

	const char * NativeFileDialog::GetError() noexcept
	{
		return NFD_GetError();
	}
}
