#pragma once
#include <cstdio>
#include <Windows.h>

namespace console
{
    class ConsoleHelper
    {
    public:
        ConsoleHelper()
        {
            Attach();
        }

        ~ConsoleHelper()
        {
            Detach();
        }

        static bool Attach()
        {
            if (!AllocConsole())
            {
                MessageBox(nullptr, "Failed to attach console!", "Error", MB_OK);
                return false;
            }
            (void)freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
            (void)freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);
            (void)freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);

            return true;
        }

        static bool Detach()
        {
            if (!FreeConsole())
            {
                MessageBox(nullptr, "Failed to detach console!", "Error", MB_OK);
                return false;
            }

            return true;
        }
    };
}
