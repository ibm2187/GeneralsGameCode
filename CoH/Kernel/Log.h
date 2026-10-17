// CoH Kernel: a log for checking features in unattended test games. The profile build has no debug
// log, so with -cohLog on the command line these lines go to CoH.log in the user data folder (the
// file starts empty each run). Without -cohLog nothing is written. Debug builds also send them to
// the debug log.

#pragma once

namespace CoH
{

bool isLogging();

// printf-style; a line break is added.
void log(const char *format, ...);

} // namespace CoH
