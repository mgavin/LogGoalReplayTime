#pragma once

#include <chrono>
#include <fstream>
#include "bakkesmod/imgui/imgui.h"
#include "bakkesmod/imgui/imgui_internal.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "bakkesmod/wrappers/MatchmakingWrapper.h"
#include "csv.hpp"
#include "HookedEvents.h"

class LogGoalReplayTime : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow {
private:
        struct data {
                int         game_num;
                std::string game_guid;
                PlaylistIds playlist_id;
                std::string playlist_name;
                bool        did_everyone_skip;
                int         milliseconds_spent_post_goal;
                int         milliseconds_spent_goal_replay;
                int         milliseconds_spent_countdown;
        };

        struct stats : public data {
                int max_post_goal;
                int min_post_goal;
                int max_goal_replay;
                int min_goal_replay;
                int max_countdown;
                int min_countdown;
        };

        const std::filesystem::path LOGBOOK_FILE_PATH = gameWrapper->GetDataFolder().append("LogGoalReplayTime.csv");

        bool in_post_goal_scored = false;
        bool in_goal_replay      = false;
        bool in_countdown        = false;
        bool did_post_goal       = false;
        bool did_goal_replay     = false;
        bool did_countdown       = false;

        std::chrono::time_point<std::chrono::steady_clock> t;

        data  current_data;
        stats stats_data;

        void        init_logfile();
        void        write_and_flush();
        void        center_imgui_text(const std::string &);
        void        generate_stats();
        std::string get_string_from_playlist_id(const PlaylistIds & id);
        void        reset_data(data & s);

        void HandlePostGoalScoredBegin();
        void HandlePostGoalScoredEnd();
        void HandleGoalReplayPlaybackBegin();
        void HandlePlayerReadyUp();
        void HandleServerReadyUp();
        void HandleGoalReplayPlaybackEnd();
        void HandleCountdownBegin();
        void HandleCountdownEnd();
        void HandleBackToMainMenu();

public:
        void onLoad() override;
        void onUnload() override;

        void        RenderSettings() override;
        std::string GetPluginName() override;
        void        SetImGuiContext(uintptr_t ctx) override;
        //
        // inherit from
        //				public BakkesMod::Plugin::PluginWindow
        //	 for the following
        // void        OnOpen() override;
        // void        OnClose() override;
        // void        Render() override;
        // std::string GetMenuName() override;
        // std::string GetMenuTitle() override;
        // bool        IsActiveOverlay() override;
        // bool        ShouldBlockInput() override;
};
