/*
  The contents of this file are dedicated by all of its authors, including

    Michael S. Gashler,
    Eric Moyer,
    anonymous contributors,

  to the public domain (http://creativecommons.org/publicdomain/zero/1.0/).

  Note that some moral obligations still exist in the absence of legal ones.
  For example, it would still be dishonest to deliberately misrepresent the
  origin of a work. Although we impose no legal requirements to obtain a
  license, it is beseeming for those who build on the works of others to
  give back useful improvements, or find a way to pay it forward. If
  you would like to cite us, a published paper about Waffles can be found
  at http://jmlr.org/papers/volume12/gashler11a/gashler11a.pdf. If you find
  our code to be useful, the Waffles team would love to hear how you use it.
*/

#ifndef __GTIME_H__
#define __GTIME_H__

#include <string>

namespace GClasses {

/// Provides some time-related functions
class GTime
{
public:
	/// Returns the number of seconds since "time X" with at least millisecond precision.
	/// On Linux, "time X" is the Epoch (midnight, Jan 1, 1970, GMT).
	/// On Windows, "time X" is the time when the system was started.
	static double seconds();

	/// Returns a string representation of the current time
	static const char* asciiTime(char* szBuf, int nSize, bool bGreenwichMeanTime = false);

	/// Adds a string representation of the current time to pS in big Endian format. For example, if sep1="-",
	/// sep2=" ", and sep3=":", and the time is one second before 2010, then it would append a string like
	/// this to pS: "2009-12-31 23:59:59". If the separators are all "", then it would append "20091231235959".
	static void appendTimeStampValue(std::string* pS, const char* sep1 = "-", const char* sep2 = " ", const char* sep3 = ":", bool bGreenwichMeanTime = false);

	/// Parses szData into a time_t. The format of szData is specified in szFormat, where
	/// Y=year, M=month, D=day, h=hour, m=minute, s=second, and all other characters are expected
	/// to match exactly. For example, if szFormat is "YYYY-MM-DD hh:mm:ss" then szData might be
	/// "2014-04-01 23:59:59". If the string fits, then *pOutTime will be set to the parsed time,
	/// and true is returned. Otherwise, *pOutTime is unchanged, and false is returned.
	static bool fromString(time_t* pOutTime, const char* szData, const char* szFormat);
};

} // namespace GClasses

#endif // __GTIME_H__
