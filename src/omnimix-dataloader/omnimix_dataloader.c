#include "iidx25.h"
#include "iidx26.h"
#include "iidx27.h"
#include <MinHook.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include <psapi.h>


static const od_payload_t *const payloads_[] = {
        &REV_20180919,
        &REV_20190902,
        &REV_20191007,
        &REV_20200929,
        NULL
};
static const od_payload_t *payload_;
static od_module_t avs2_;
static od_module_t bm2dx_;
static od_node_data_t *node_data_head = 0;
static od_node_data_t *node_data_tail = 0;
static read_node_data_s32_t read_node_data_s32;
game_log_t game_log;

HMODULE get_module(const char *name)
{
    HMODULE rv = NULL;

    // Prioritize exact match
    rv = GetModuleHandle(name);
    if (rv) return rv;

    // Find first .dll including specified name
    HMODULE modules[1024];
    DWORD cb_needed;
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
    if (EnumProcessModules(process, modules, sizeof(modules), &cb_needed))
    {
        const size_t needlelen = strlen(name) - 4;
        char *needle = malloc(needlelen + 1);
        memcpy(needle, name, needlelen);
        needle[needlelen] = '\0';
        for (unsigned int i = 0; i < (cb_needed / sizeof(HMODULE)); i++)
        {
            TCHAR szModName[MAX_PATH];
            if (GetModuleBaseName(process, modules[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
            {
                if (strstr(szModName, needle))
                {
                    log_msg(WARNING, "Loading %s as %s instead", szModName, name);
                    rv = modules[i];
                    break;
                }
            }
        }
        free(needle);
    }
    CloseHandle(process);

    return rv;
}
const char *get_revision()
{
    static char rv[9] = "";
    FILE *ea3f = fopen("prop/ea3-config.xml", "rb");
    if (!ea3f)return rv;
    fseek(ea3f, 0L, SEEK_END);
    int sz = ftell(ea3f);
    fseek(ea3f, 0, SEEK_SET);
    char *ea3buf = malloc(sz + 1);
    fread(ea3buf, sz, 1, ea3f);
    ea3buf[sz] = '\0';

    const char *cursor = ea3buf;
    cursor = strstr(cursor ? cursor : rv, "<soft>");
    cursor = strstr(cursor ? cursor : rv, "<ext __type=\"str\">");
    cursor = strstr(cursor ? cursor : rv, "2");
    memmove(rv, cursor ? cursor : rv, 8);
    rv[8] = '\0';
    if (!strstr(cursor ? cursor : rv, "</ext>")) rv[0] = '\0';

    fclose(ea3f);
    free(ea3buf);
    return rv;
}
const od_payload_t *get_payload()
{
    const char *revision = get_revision();
    const od_payload_t *const *it = payloads_;
    while (*it)
    {
        if (!strcmp((*it)->revision, revision))
            break;
        it++;
    }
    return *it;
}

void add_node_data(int song_id, const int clear_node_data[])
{
    od_node_data_t *data = malloc(sizeof(od_node_data_t));
    data->song_id = (uint16_t) song_id;
    data->next = 0;
    for (int i = 0; i < 12; i++)
    {
        data->values[i] = (int16_t) clear_node_data[i];
    }
    if (node_data_head == 0)
    {
        node_data_head = data;
        node_data_tail = node_data_head;
    }
    else
    {
        node_data_tail->next = data;
        node_data_tail = data;
    }
}
od_node_data_t *find_node_data(int song_id)
{
    od_node_data_t *current = node_data_head;
    while (current != 0)
    {
        if (current->song_id == song_id)
            return current;

        current = current->next;
    }
    return 0;
}
__int64 read_node_data_hook(void *node_reference, int *dest, unsigned int bytes)
{
    log_msg(INFO, "read_node_data_hook() triggered");
    void *retaddr = __builtin_return_address(0);
    if (retaddr != (void *) (bm2dx_.base + payload_->node_data_rtn_va))
    {
        return read_node_data_s32(node_reference, dest, bytes);
    }
    else
    {
#if INLINE_ASM
        //_BUG: original on-compile solution, this is no longer supported by GCC as well: https://stackoverflow.com/questions/2114163/reading-a-register-value-into-a-c-variable
        register void *r11 __asm__("r11");//_BUG: '__asm__ volatile' is a compilation error now
        char *song_id_text_tmp = (char *) (r11 + 0xA0);//_BUG: 0xA0 may also change between versions?
#else
        // compiler independent solution
        /*_INFO: post compilation patching
         * In disassembler go to read_node_data_hook and search for lea instruction referring to cs:read_node_data_original
         * If you disabled release optimization should be something like lea and than add 0xA0
         * mov rax, [r11 + 0xA0]
         * 49 8b 83 a0 00 00 00 .. (fill rest with 90)
         * or + 0x500 since r11+0xA0==r11+(0xA0*sizeof(void*)) in which case
         * 49 8b 83 a0 00 00 00 .. (fill rest with 90)
         * */
        char *song_id_text_tmp = (char *) (&read_node_data_s32 + 0xA0 / sizeof(void *)); /*leave for better assembly output, modify so it takes r11*/
#endif

        char song_id_text[8];
        for (int i = 0; i < 8; i++)
        {
            song_id_text[i] = song_id_text_tmp[i];
        }

        int song_id = atoi(song_id_text);
        int dest_buffer[12];
        __int64 result = read_node_data_s32(node_reference, dest_buffer, bytes);
        for (int i = 0; i < 12; i++)
        {
            dest[i] = dest_buffer[i];
        }

        add_node_data(song_id, dest_buffer);
        return result;
    }
}
bool __fastcall clear_rate_hook(int song_id, int difficulty, unsigned int *clear_rate)
{
    od_node_data_t *data = find_node_data(song_id);
    if (data == 0)
        return false;

    *clear_rate = (unsigned int) data->values[difficulty];
    if (*clear_rate > 1000)
        return false;

    return true;
}
bool __fastcall fc_rate_hook(int song_id, int difficulty, unsigned int *fc_rate)
{
    od_node_data_t *data = find_node_data(song_id);
    if (data == 0)
        return false;

    *fc_rate = (unsigned int) data->values[difficulty + 6];
    if (*fc_rate > 1000)
        return false;

    return true;
}
BOOL WINAPI DllMain(HMODULE module, DWORD reason, void *reserved)
{
    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
        {
            avs2_.handle = GetModuleHandle("avs2-core.dll");
            game_log = (game_log_t) (GetProcAddress(avs2_.handle, "XCgsqzn0000175"));
            payload_ = get_payload();
            if (!payload_)
            {
                log_msg(WARNING, "Invalid environment, aborting...");
                return false;
            }

            bm2dx_.handle = get_module("bm2dx.dll");
            read_node_data_s32 = (read_node_data_s32_t) (bm2dx_.base + payload_->node_data_va);
            get_clear_rate_t clear_rate = (get_clear_rate_t) (bm2dx_.base + payload_->clear_rate_va);
            get_fc_rate_t fc_rate = (get_fc_rate_t) (bm2dx_.base + payload_->fc_rate_va);
            log_msg(MISC, "Detected revision: hook=%s, payload=%s", HOOK_REVISION, payload_->revision);

            int error_code = MH_Initialize();
            if (error_code == MH_ERROR_ALREADY_INITIALIZED)
            {
                log_msg(INFO, "MinHook already initialized");
                error_code ^= MH_ERROR_ALREADY_INITIALIZED;
            }
            if (error_code == MH_OK)
            {
                error_code |= MH_CreateHook((PVOID) read_node_data_s32, (PVOID) read_node_data_hook, (PVOID *) &read_node_data_s32);
                error_code |= MH_CreateHook((PVOID) clear_rate, (PVOID) clear_rate_hook, (PVOID *) 0);
                error_code |= MH_CreateHook((PVOID) fc_rate, (PVOID) fc_rate_hook, (PVOID *) 0);
                error_code |= MH_EnableHook(MH_ALL_HOOKS);
            }
            if (error_code == MH_OK)
            {
                log_msg(INFO, "Injected payload: rrd=0x%x, gcr=0x%x, gfr=0x%x", payload_->node_data_va, payload_->clear_rate_va, payload_->fc_rate_va);
            }
            else
            {
                log_msg(WARNING, "MinHook error(0x%x)", error_code);
                log_msg(WARNING, "Error injecting into bm2dx.dll, aborting...");
                return false;
            }
            break;
        }
        case DLL_PROCESS_DETACH:
            MH_Uninitialize();
            break;
        default:
            break;
    }
    return TRUE;
}
