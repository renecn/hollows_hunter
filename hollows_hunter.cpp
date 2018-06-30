#include "hollows_hunter.h"

#include <iostream>

bool is_replaced_process(t_params args)
{
    t_report report = PESieve_scan(args);
    if (report.errors) return false;
    if (report.replaced) {
        std::cout << "Found replaced: " << std::dec << args.pid << std::endl;
        return true;
    }
    if (report.suspicious) {
        std::cout << "Found suspicious: " << std::dec << args.pid << std::endl;
        return true;
    }
    return false;
}

bool get_process_name(IN HANDLE hProcess, OUT LPSTR nameBuf, IN SIZE_T nameMax)
{
    HMODULE hMod;
    DWORD cbNeeded;

    if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
        GetModuleBaseNameA(hProcess, hMod, nameBuf, nameMax);
        return true;
    }
    return false;
}

bool is_searched_process(DWORD processID, const char* searchedName)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (hProcess == NULL) return false;

    CHAR szProcessName[MAX_PATH];
    if (get_process_name(hProcess, szProcessName, MAX_PATH)) {
        if (strcmp(szProcessName, searchedName) == 0) {
            printf("%s  (PID: %u)\n", szProcessName, processID);
            CloseHandle(hProcess);
            return true;
        }
    }
    CloseHandle(hProcess);
    return false;
}

size_t find_suspicious_process(std::vector<DWORD> &replaced, t_params args, std::string pname)
{
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
        return NULL;
    }

    //calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);

    char image_buf[MAX_PATH] = { 0 };

    for (i = 0; i < cProcesses; i++) {
        if (aProcesses[i] == 0) continue;
        DWORD pid = aProcesses[i];
        if (pname != "") {
            if (!is_searched_process(pid, pname.c_str())) {
                //it is not the searched process, so skip it
                continue;
            }
        }
        std::cout << ">> Scanning PID: " << std::dec << pid << std::endl;
        args.pid = pid;
        if (is_replaced_process(args)) {
            replaced.push_back(pid);
        }
    }
    return replaced.size();
}
