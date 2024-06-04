#pragma once

#include <global.h>

namespace JanSordid
{
	class NativeFileDialog
	{
	public:
		enum class Code
		{
			Error,  /* programmatic error */
			Okay,   /* user pressed okay, or successful return */
			Cancel, /* user pressed cancel */
		};

		struct Result
		{
			Code   code;
			String path;            // only valid if code == Okay, on code == Error this contains the error message
		};

		struct ResultMultiple
		{
			Code           code;
			Vector<String> paths;   // only valid if code == Okay, on code == Error the first entry contains the error message
		};

		constexpr static const char * EmptyFilter      = nullptr;
		constexpr static const char * EmptyDefaultPath = nullptr;

		// single file open dialog
		[[nodiscard]] static Result OpenDialog( const char * filterList = EmptyFilter, const char * defaultPath = EmptyDefaultPath ) noexcept;

		// multiple file open dialog
		[[nodiscard]] static ResultMultiple OpenDialogMultiple( const char * filterList = EmptyFilter, const char * defaultPath = EmptyDefaultPath ) noexcept;

		// save dialog
		[[nodiscard]] static Result SaveDialog( const char * filterList = EmptyFilter, const char * defaultPath = EmptyDefaultPath ) noexcept;

		// select folder dialog
		[[nodiscard]] static Result PickFolder( const char * defaultPath = EmptyDefaultPath ) noexcept;

		// get last error -- set when Result::code is Code::Error
		[[nodiscard]] static const char * GetError() noexcept;
	};
}
