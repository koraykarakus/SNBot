// implementation of CBotManager.h

#include "CBotManager.h"
#include "CLogger.h"

bool CBotManager::ProcessPendingRequests()
{
    bool processed = false;

    while (true)
    {
        cmd_queue cmd;

        {
            std::lock_guard<std::mutex> lock(m_cmdMutex);

            if (m_commands.empty())
                break;

            cmd = m_commands.front();
            m_commands.pop();
        }

        processed = true;

        switch (cmd.type)
        {
        case 1:
            CreateBots(cmd.count);
            break;

        default:
            break;
        }
    }

    return processed;
}
void CBotManager::CreateBots(int count)
{
    CLogger::Info("Creating {} bots..", count);
    // create
}