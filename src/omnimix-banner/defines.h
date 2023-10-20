#ifndef _DEFINES_H
#define _DEFINES_H
#include <stdint.h>
#include <windows.h>


enum LOG_LEVEL
{
	FATAL = 1,
	WARNING = 2,
	INFO = 3,
	MISC = 4
};
typedef struct ob_payload ob_payload_t;
typedef union ob_module ob_module_t;
typedef void(__fastcall *rainbow_banner_t)(void *, int32_t);
typedef int64_t(*game_log_t)(int64_t, enum LOG_LEVEL, const char*, const char*, ...);

union ob_module
{
    long long base;
    HMODULE handle;
};
struct ob_payload
{
    const char *revision;
    const int songdb_va;
    const int banner_va;
    const uint16_t *song_by_id;
};

extern game_log_t game_log;

#define HOOK_NAME "omnimix_banner"
#ifdef BIN_BUILDREV
#define HOOK_REVISION BIN_BUILDREV
#else
#error "Specify BIN_BUILDREV"
#endif
#define log_msg(level, msg, args...) game_log(0, level, HOOK_NAME, msg, ##args)



#endif