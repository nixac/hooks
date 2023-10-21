#ifndef _DEFINES_H
#define _DEFINES_H
#include <stdint.h>
#include <stdbool.h>
#include <windows.h>


enum LOG_LEVEL
{
    FATAL = 1,
    WARNING = 2,
    INFO = 3,
    MISC = 4
};
typedef struct od_payload od_payload_t;
typedef struct od_node_data od_node_data_t;
typedef union od_module od_module_t;
typedef __int64(__fastcall *read_node_data_s32_t)(void *, void *, unsigned int);
typedef bool(__fastcall *get_clear_rate_t)(int, int, unsigned int *);
typedef bool(__fastcall *get_fc_rate_t)(int, int, unsigned int *);
typedef int64_t(*game_log_t)(int64_t, enum LOG_LEVEL, const char *, const char *, ...);

union od_module
{
    long long base;
    HMODULE handle;
};
struct od_payload
{
    const char *revision;
    const int clear_rate_va;
    const int fc_rate_va;
    const int node_data_va;
    const int node_data_rtn_va;
};
struct od_node_data
{
	uint16_t song_id;
	int16_t values[12];
	od_node_data_t* next;
};

extern game_log_t game_log;

#define HOOK_NAME "omnimix_dataloader"
#ifdef BIN_BUILDREV
#define HOOK_REVISION BIN_BUILDREV
#else
#error "Specify BIN_BUILDREV"
#endif
#define INLINE_ASM 1
#define log_msg(level, msg, args...) game_log(0, level, HOOK_NAME, msg, ##args)


#endif