#include "../filezilla.h"

#include "chmod.h"
#include "../directorycache.h"

namespace {
enum chmodStates
{
	chmod_init,
	chmod_waitcwd,
	chmod_chmod
};
}

int CSftpChmodOpData::Send()
{
	if (opState == chmod_init) {
		log(logmsg::status, _("Setting permissions of '%s' to '%s'"), command_.GetPath().FormatFilename(command_.GetFile()), command_.GetPermission());
		controlSocket_.ChangeDir(command_.GetPath());
		opState = chmod_waitcwd;
		return FZ_REPLY_CONTINUE;
	}
	else if (opState == chmod_chmod) {
		engine_.GetDirectoryCache().UpdateFile(currentServer_, command_.GetPath(), command_.GetFile(), false, CDirectoryCache::unknown);

		std::wstring quotedFilename = controlSocket_.QuoteFilename(command_.GetPath().FormatFilename(command_.GetFile(), !useAbsolute_));

		return controlSocket_.SendCommand(L"chmod " + command_.GetPermission() + L" " + quotedFilename);
	}

	return FZ_REPLY_INTERNALERROR;
}

int CSftpChmodOpData::ParseResponse()
{
	return controlSocket_.result_;
}

int CSftpChmodOpData::SubcommandResult(int prevResult, COpData const&)
{
	if (opState == chmod_waitcwd) {
		if (prevResult != FZ_REPLY_OK) {
			useAbsolute_ = true;
		}

		opState = chmod_chmod;
		return FZ_REPLY_CONTINUE;
	}
	else {
		return FZ_REPLY_INTERNALERROR;
	}
}
