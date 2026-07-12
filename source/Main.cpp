#include <winsock2.h>
#include <ws2tcpip.h>
#include <plugin.h> // Plugin-SDK version 1004 from 2026-04-18 13:03:53
#include <CMessages.h>
#include "CStreaming.h"
#include "Mod.h"

using namespace plugin;

struct Main
{
    size_t m_frame = 0; // render frame counter
    Mod mod;

    Main()
    {
        // register event callbacks
        Events::gameProcessEvent += []{ gInstance.OnGameProcess(); };
    }

    void OnGameProcess()
    {
        //onScreenText();
        mod.start();
    }

    void onScreenText()
    {
        m_frame++;

        static char msg[255];
        sprintf_s(msg, "Hello from '%s' plugin! Frame %d", TARGET_NAME, m_frame);

        CMessages::AddMessageJumpQ(msg, 500, 0);
    }
} gInstance;
