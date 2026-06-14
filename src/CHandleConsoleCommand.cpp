// implementation of CBotManager.h

#include "CBotManager.h"
#include "CLogger.h"
#include "CDatabase.h"

bool CBotManager::ProcessPendingRequests()
{
    bool processed = false;

    while (true)
    {
        cmd_queue cmd;

        {
            std::lock_guard<std::mutex> lock(mutex_command_);

            if (commands_.empty())
                break;

            cmd = commands_.front();
            commands_.pop();
        }

        processed = true;

        switch (cmd.type)
        {
        case 1:
            CreateBots(cmd.count);
            break;
        case 2:
            RemoveBots();
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
    database_->AddBots(count);
}

void CBotManager::RemoveBots() 
{
    if (database_->RemoveBots())
    {
        bots_ = database_->GetLoadedBots();
        CLogger::Info("All bots and their planet removed from database by success.");
    }
}