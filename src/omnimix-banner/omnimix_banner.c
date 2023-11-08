#include "iidx25.h"
#include "iidx26.h"
#include "iidx27.h"
#include "iidx28.h"
#include "iidx29.h"
#include "iidx30.h"
#include <MinHook.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include <psapi.h>


static const ob_payload_t *const payloads_[] = {
        &REV_20180919,
        &REV_20190902,
        &REV_20191007,
        &REV_20200929,
        &REV_20210830,
        &REV_20210915,
        &REV_20220824,
        &REV_20230905,
        NULL
};
static const ob_payload_t *payload_;
static ob_module_t avs2_;
static ob_module_t bm2dx_;
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
const ob_payload_t *get_payload()
{
    const char *revision = get_revision();
    const ob_payload_t *const *it = payloads_;
    while (*it)
    {
        if (!strcmp((*it)->revision, revision))
            break;
        it++;
    }
    return *it;
}
inline int count_ids(const uint16_t *it)
{
    int rv = 0;
    while (*it)
    {
        rv++;
        it++;
    }
    return rv;
}

int binary_search(const uint16_t arr[], int l, int r, uint16_t x)
{
    while (l <= r)
    {
        int m = l + (r - l) / 2;
        if (arr[m] == x)
            return m;
        if (arr[m] < x)
            l = m + 1;
        else
            r = m - 1;
    }
    return -1;
}
bool __fastcall hook(void *some_pointer, int64_t song_id)
{
    return binary_search(payload_->song_by_id, 0, count_ids(payload_->song_by_id), (uint16_t) song_id) != -1 ? true : false;
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
            rainbow_banner_t rainbow_banner = (rainbow_banner_t) (bm2dx_.base + payload_->banner_va);
            char *omni_string = ((char *) (bm2dx_.base + payload_->songdb_va));
            if (strcmp(omni_string, "music_data.bin") != 0 && strcmp(omni_string, "music_omni.bin") != 0)
            {
                log_msg(WARNING, "Invalid bm2dx.dll, aborting...");
                return false;
            }
            log_msg(MISC, "Detected revision: hook=%s, payload=%s", HOOK_REVISION, payload_->revision);

            int error_code = MH_Initialize();
            if (error_code == MH_ERROR_ALREADY_INITIALIZED)
            {
                log_msg(INFO, "MinHook already initialized");
                error_code ^= MH_ERROR_ALREADY_INITIALIZED;
            }
            if (error_code == MH_OK)
            {
                error_code |= MH_CreateHook((PVOID) rainbow_banner, (PVOID) hook, 0);
                error_code |= MH_EnableHook(MH_ALL_HOOKS);
            }
            if (error_code == MH_OK)
            {
                log_msg(INFO, "Injected payload: sva=0x%x, bva=0x%x, sbi=%d", payload_->songdb_va, payload_->banner_va, count_ids(payload_->song_by_id));
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
