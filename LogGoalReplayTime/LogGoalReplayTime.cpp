#include "LogGoalReplayTime.h"
#include <format>
#include "HookedEvents.h"
#include "Logger.h"

BAKKESMOD_PLUGIN(LogGoalReplayTime, "LogGoalReplayTime", "0.0.0", /*UNUSED*/ NULL);
std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

/// <summary>
/// do the following when your plugin is loaded
/// </summary>
void LogGoalReplayTime::onLoad() {
        // initialize things
        _globalCvarManager        = cvarManager;
        HookedEvents::gameWrapper = gameWrapper;

        init_logfile();

        HookedEvents::AddHookedEvent(
                "Function GameEvent_Soccar_TA.PostGoalScored.BeginState",
                // AFTER GOAL IS SCORED ! and it shows "XYZ SCORED!"
                std::bind(&LogGoalReplayTime::HandlePostGoalScoredBegin, this),
                true);
        HookedEvents::AddHookedEvent(
                "Function GameEvent_Soccar_TA.PostGoalScored.EndState",
                // WHEN THE "XYZ" SCORED" TEXT IS OVER!!!
                std::bind(&LogGoalReplayTime::HandlePostGoalScoredEnd, this),
                true);
        HookedEvents::AddHookedEvent(
                "Function GameEvent_Soccar_TA.ReplayPlayback.BeginState",
                // WHEN THE REPLAY BEGINS
                std::bind(&LogGoalReplayTime::HandleGoalReplayPlaybackBegin, this),
                true);
        HookedEvents::AddHookedEvent(
                "Function TAGame.PRI_TA.ServerReadyUp",
                // EVERYONE SKIPPED THE REPLAY, AKA THEY'RE "READY"!
                std::bind(&LogGoalReplayTime::HandleServerReadyUp, this),
                true);
        HookedEvents::AddHookedEvent(
                "Function GameEvent_Soccar_TA.ReplayPlayback.EndState",
                // REPLAY IS OVER!
                std::bind(&LogGoalReplayTime::HandleGoalReplayPlaybackEnd, this),
                true);
        HookedEvents::AddHookedEvent(
                "Function GameEvent_Soccar_TA.Countdown.BeginState",
                // COUNTDOWN BEGINS!
                std::bind(&LogGoalReplayTime::HandleCountdownBegin, this),
                true);
        HookedEvents::AddHookedEvent(
                "Function GameEvent_Soccar_TA.Countdown.EndState",
                // COWNTDOWN ENDS! 3-2-1 GO!
                std::bind(&LogGoalReplayTime::HandleCountdownEnd, this),
                true);
        HookedEvents::AddHookedEvent(
                "Function TAGame.LoadingScreen_TA.HandlePreLoadMap",
                // went back to main menu
                std::bind(&LogGoalReplayTime::HandleBackToMainMenu, this),
                true);

        reset_data(current_data);
        reset_data(stats_data);
        generate_stats();
}

/// <summary>
///  do the following when your plugin is unloaded
/// </summary>
void LogGoalReplayTime::onUnload() {
        // destroy things
        // dont throw here
}

void LogGoalReplayTime::center_imgui_text(const std::string & text) {
        // calc width so far.
        int   cur_col            = ImGui::GetColumnIndex();
        float total_width_so_far = 0.0f;
        while (--cur_col >= 0) {
                // get each column width, down to 0, break at -1
                total_width_so_far += ImGui::GetColumnWidth(cur_col);
        }

        float width      = ImGui::GetContentRegionAvailWidth();
        float text_width = ImGui::CalcTextSize(text.c_str()).x;

        float indent = (width - text_width) * 0.5f;

        float min_indent = 0.0f;
        if (std::fabs(indent - min_indent) <= 1e-6) {
                indent = min_indent;
        }

        ImGui::SetCursorPosX(total_width_so_far + indent);
        ImGui::Text(text.c_str());
}

/// <summary>
/// This call usually includes ImGui code that is shown and rendered (repeatedly,
/// on every frame rendered) when your plugin is selected in the plugin
/// manager. AFAIK, if your plugin doesn't have an associated *.set file for its
/// settings, this will be used instead.
/// </summary>
void LogGoalReplayTime::RenderSettings() {
        // for imgui plugin window

        // PRETTY TABLES
        // output stats

        ImGui::TextUnformatted(
                std::vformat("Last Game Number: {}", std::make_format_args(stats_data.game_num)).c_str());
        ImGui::TextUnformatted(std::vformat("Last Game GUID: {}", std::make_format_args(stats_data.game_guid)).c_str());

        ImGui::BeginColumns("stats_columns", 4, ImGuiColumnsFlags_NoResize);
        float width = ImGui::GetWindowContentRegionWidth();
        ImGui::SetColumnWidth(0, width * 0.4);
        ImGui::SetColumnWidth(1, width * 0.1);
        ImGui::SetColumnWidth(2, width * 0.1);
        ImGui::SetColumnWidth(3, width * 0.1);
        ImGui::NextColumn();
        center_imgui_text("Min");
        ImGui::NextColumn();
        center_imgui_text("Average");
        ImGui::NextColumn();
        center_imgui_text("Max");
        ImGui::Separator();
        ImGui::NextColumn();
        ImGui::TextUnformatted("Time (in milliseconds) spent after goal scored:");
        ImGui::NextColumn();
        center_imgui_text(std::vformat("{:6d}ms", std::make_format_args(stats_data.min_post_goal)).c_str());
        ImGui::NextColumn();
        center_imgui_text(
                std::vformat("{:6d}ms", std::make_format_args(stats_data.milliseconds_spent_post_goal)).c_str());
        ImGui::NextColumn();
        center_imgui_text(std::vformat("{:6d}ms", std::make_format_args(stats_data.max_post_goal)).c_str());
        ImGui::Separator();
        ImGui::NextColumn();
        ImGui::TextUnformatted("Time (in milliseconds) spent in goal replay:");
        ImGui::NextColumn();
        center_imgui_text(std::vformat("{:6d}ms", std::make_format_args(stats_data.min_goal_replay)).c_str());
        ImGui::NextColumn();
        center_imgui_text(
                std::vformat("{:6d}ms", std::make_format_args(stats_data.milliseconds_spent_goal_replay)).c_str());
        ImGui::NextColumn();
        center_imgui_text(std::vformat("{:6d}ms", std::make_format_args(stats_data.max_goal_replay)).c_str());
        ImGui::Separator();
        ImGui::NextColumn();
        ImGui::TextUnformatted("Time (in milliseconds) spent in countdown:");
        ImGui::NextColumn();
        center_imgui_text(std::vformat("{:6d}ms", std::make_format_args(stats_data.min_countdown)).c_str());
        ImGui::NextColumn();
        center_imgui_text(
                std::vformat("{:6d}ms", std::make_format_args(stats_data.milliseconds_spent_countdown)).c_str());
        ImGui::NextColumn();
        center_imgui_text(std::vformat("{:6d}ms", std::make_format_args(stats_data.max_countdown)).c_str());
        ImGui::NextColumn();
        ImGui::Separator();
        ImGui::EndColumns();
}

/// <summary>
/// "SetImGuiContext happens when the plugin’s ImGui is initialized."
/// https://wiki.bakkesplugins.com/imgui/custom_fonts/
///
/// also:
/// "Don't call this yourself, BM will call this function with a pointer
/// to the current ImGui context"
/// ...
///
/// so ¯\(°_o)/¯
/// </summary>
/// <param name="ctx">AFAIK The pointer to the ImGui context</param>
void LogGoalReplayTime::SetImGuiContext(uintptr_t ctx) {
        ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext *>(ctx));
}

std::string LogGoalReplayTime::GetPluginName() {
        return "LogGoalReplayTime";
}

void LogGoalReplayTime::init_logfile() {
        if (!std::filesystem::exists(LOGBOOK_FILE_PATH)) {
                // file aint written to yet?
                std::ofstream  logbook_file {LOGBOOK_FILE_PATH, std::ios::app};
                csv::CSVWriter logbook_writer {logbook_file};
                logbook_writer << std::vector {
                        "LoggedGame#",
                        "LoggedGameGUID",
                        "PlaylistID",
                        "PlaylistName",
                        "DidEveryoneSkip",
                        "MillisecondsSpentPostGoal",
                        "MillisecondsSpentGoalReplay",
                        "MillisecondsSpentCountdown",
                };
        } else {
                csv::CSVFileInfo fi = csv::get_file_info(LOGBOOK_FILE_PATH.string());
                // DEBUG: LOG("fi info: ncols {}, nrows {}", fi.n_cols, fi.n_rows);
                if (fi.n_rows > 1) {
                        csv::CSVReader logbook_reader {LOGBOOK_FILE_PATH.string()};
                        csv::CSVRow    row;
                        while (logbook_reader.read_row(row))
                                ;
                        current_data.game_num  = row["LoggedGame#"].get<int>();
                        current_data.game_guid = row["LoggedGameGUID"].get<std::string>();
                }
        }
}

void LogGoalReplayTime::HandlePostGoalScoredBegin() {
        in_post_goal_scored = true;

        if (gameWrapper->IsInOnlineGame()) {
                // if you're in an online game
                // why not freeplay? idk...
                csv::CSVFileInfo fi = csv::get_file_info(LOGBOOK_FILE_PATH.string());
                if (fi.n_rows > 1) {  // COUNTS THE HEADER ROW
                        csv::CSVReader logbook_reader {LOGBOOK_FILE_PATH.string()};
                        /* skip to end :rolling_eyes: */
                        csv::CSVRow    row;
                        while (logbook_reader.read_row(row))
                                ;
                        current_data.game_num  = row["LoggedGame#"].get<int>();
                        current_data.game_guid = row["LoggedGameGUID"].get<std::string>();
                }
                ServerWrapper sw = gameWrapper->GetCurrentGameState();
                if (sw) {
                        std::string game_guid = sw.GetMatchGUID();
                        if (game_guid != current_data.game_guid) {
                                // if you're in a new match
                                current_data.game_guid  = game_guid;
                                current_data.game_num  += 1;
                        }

                        GameSettingPlaylistWrapper gspw = sw.GetPlaylist();
                        current_data.playlist_id        = static_cast<PlaylistIds>(gspw.GetPlaylistId());
                        current_data.playlist_name      = get_string_from_playlist_id(current_data.playlist_id);
                }

                t = std::chrono::steady_clock::now();
        }
}
void LogGoalReplayTime::HandlePostGoalScoredEnd() {
        if (in_post_goal_scored) {
                did_post_goal                            = true;
                in_post_goal_scored                      = false;
                const std::chrono::duration<double> diff = std::chrono::steady_clock::now() - t;
                current_data.milliseconds_spent_post_goal =
                        static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(diff).count());
        }
}

void LogGoalReplayTime::HandleGoalReplayPlaybackBegin() {
        in_goal_replay = true;
        t              = std::chrono::steady_clock::now();
}

void LogGoalReplayTime::HandleServerReadyUp() {
        current_data.did_everyone_skip = true;
}

void LogGoalReplayTime::HandleGoalReplayPlaybackEnd() {
        if (in_goal_replay) {
                in_goal_replay                           = false;
                did_goal_replay                          = true;
                const std::chrono::duration<double> diff = std::chrono::steady_clock::now() - t;
                current_data.milliseconds_spent_goal_replay =
                        static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(diff).count());
        }
}

void LogGoalReplayTime::HandleCountdownBegin() {
        in_countdown = true;
        t            = std::chrono::steady_clock::now();
}

void LogGoalReplayTime::HandleCountdownEnd() {
        if (in_countdown) {
                in_countdown                             = false;
                did_countdown                            = true;
                const std::chrono::duration<double> diff = std::chrono::steady_clock::now() - t;
                current_data.milliseconds_spent_countdown =
                        static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(diff).count());

                if (did_post_goal && did_goal_replay && did_countdown) {
                        write_and_flush();
                }
        }
}

void LogGoalReplayTime::HandleBackToMainMenu() {
        if (in_post_goal_scored || in_goal_replay || in_countdown) {
                // somehow got kicked back to the main manu without going through a countdown
                // maybe afk?, or the goal and replay was the last one of the game
                write_and_flush();
                in_post_goal_scored = in_goal_replay = in_countdown = false;
        }
}

void LogGoalReplayTime::write_and_flush() {
        // write everything out
        std::ofstream logbook_file {LOGBOOK_FILE_PATH, std::ios::app};  // fuck. this should be checked.
        if (!logbook_file) {
                LOG("CANNOT OPEN LOGBOOK FILE! THROWING AWAY DATA! CURRENT DATA:");
                LOG("Game Number: {}\nGame GUID: {}\nPlaylist ID: {}\nPlaylist Name: {}\nEveryone Skipped: {}\nMilliseconds Spent Post Goal: {}\nMilliseconds Spent Goal Replay: {}\nMilliseconds Spent In Countdown: {}",
                    current_data.game_num,
                    current_data.game_guid,
                    static_cast<int>(current_data.playlist_id),
                    current_data.playlist_name,
                    current_data.did_everyone_skip,
                    current_data.milliseconds_spent_post_goal,
                    current_data.milliseconds_spent_goal_replay,
                    current_data.milliseconds_spent_countdown);
        } else {
                csv::CSVWriter logbook_writer {logbook_file};
                logbook_writer << std::vector<std::string> {
                        std::to_string(current_data.game_num),
                        current_data.game_guid,
                        std::to_string(static_cast<int>(current_data.playlist_id)),
                        current_data.playlist_name,
                        std::to_string(current_data.did_everyone_skip),
                        std::to_string(current_data.milliseconds_spent_post_goal),
                        std::to_string(current_data.milliseconds_spent_goal_replay),
                        std::to_string(current_data.milliseconds_spent_countdown)};
        }
        // clear everything out.
        reset_data(current_data);
        // reset flags
        did_post_goal = did_goal_replay = did_countdown = false;
        generate_stats();
}

void LogGoalReplayTime::generate_stats() {
        csv::CSVFileInfo fi = csv::get_file_info(LOGBOOK_FILE_PATH.string());
        if (fi.n_rows < 2) {
                return;
        }
        csv::CSVReader logbook_reader(LOGBOOK_FILE_PATH.string());

        using ull           = unsigned long long;
        ull         tmppost = 0, tmpgoal = 0, tmpcount = 0;
        int         tmppostc = 0, tmpgoalc = 0, tmpcountc = 0;
        csv::CSVRow row;
        while (logbook_reader.read_row(row)) {
                if (!row["DidEveryoneSkip"].get<bool>()) {
                        if (auto n = row["MillisecondsSpentPostGoal"].get<int>(); n != 0) {
                                tmppost  += n;
                                tmppostc += 1;
                                stats_data.min_post_goal =
                                        (stats_data.min_post_goal == 0) ? n : std::min(stats_data.min_post_goal, n);
                                stats_data.max_post_goal =
                                        (stats_data.max_post_goal == 0) ? n : std::max(stats_data.max_post_goal, n);
                        }
                        if (auto n = row["MillisecondsSpentGoalReplay"].get<int>(); n != 0) {
                                tmpgoal  += n;
                                tmpgoalc += 1;
                                stats_data.min_goal_replay =
                                        (stats_data.min_goal_replay == 0) ? n : std::min(stats_data.min_goal_replay, n);
                                stats_data.max_goal_replay =
                                        (stats_data.max_goal_replay == 0) ? n : std::max(stats_data.max_goal_replay, n);
                        }
                        if (auto n = row["MillisecondsSpentCountdown"].get<int>(); n != 0) {
                                tmpcount  += n;
                                tmpcountc += 1;
                                stats_data.min_countdown =
                                        (stats_data.min_countdown == 0) ? n : std::min(stats_data.min_countdown, n);
                                stats_data.max_countdown =
                                        (stats_data.max_countdown == 0) ? n : std::max(stats_data.max_countdown, n);
                        }
                }
        }

        stats_data.milliseconds_spent_post_goal   = static_cast<int>(tmppost / static_cast<ull>(tmppostc));
        stats_data.milliseconds_spent_goal_replay = static_cast<int>(tmpgoal / static_cast<ull>(tmpgoalc));
        stats_data.milliseconds_spent_countdown   = static_cast<int>(tmpcount / static_cast<ull>(tmpcountc));
        stats_data.game_num                       = row["LoggedGame#"].get<int>();
        stats_data.game_guid                      = row["LoggedGameGUID"].get<std::string>();
}

/// <summary>
/// Gets a string representation of the playlist from the ID.
/// </summary>
/// <param name="id">the playlist ID returned from bakkesmod</param>
/// <returns></returns>
std::string LogGoalReplayTime::get_string_from_playlist_id(const PlaylistIds & id) {
        switch (id) {
                case PlaylistIds::Unknown:
                        return "Unknown";
                case PlaylistIds::Casual:
                        return "Casual";
                case PlaylistIds::Duel:
                        return "Duel";
                case PlaylistIds::Doubles:
                        return "Doubles";
                case PlaylistIds::Standard:
                        return "Standard";
                case PlaylistIds::Chaos:
                        return "Chaos";
                case PlaylistIds::PrivateMatch:
                        return "Private Match";
                case PlaylistIds::Season:
                        return "Season";
                case PlaylistIds::OfflineSplitscreen:
                        return "Offline Splitscreen";
                case PlaylistIds::Training:
                        return "Training";
                case PlaylistIds::RankedSoloDuel:
                        return "Ranked Solo Duel";
                case PlaylistIds::RankedTeamDoubles:
                        return "Ranked Team Doubles";
                case PlaylistIds::RankedStandard:
                        return "Ranked Standard";
                case PlaylistIds::SnowDayPromotion:
                        return "SnowDay Promotion";
                case PlaylistIds::Experimental:
                        return "Experimental";
                case PlaylistIds::BasketballDoubles:
                        return "Basketball Doubles";
                case PlaylistIds::Rumble:
                        return "Rumble";
                case PlaylistIds::Workshop:
                        return "Workshop";
                case PlaylistIds::UGCTrainingEditor:
                        return "UGC Training Editor";
                case PlaylistIds::UGCTraining:
                        return "UGC Training";
                case PlaylistIds::Tournament:
                        return "Tournament";
                case PlaylistIds::Breakout:
                        return "Breakout";
                case PlaylistIds::TenthAnniversary:
                        return "Tenth Anniversary";
                case PlaylistIds::FaceIt:
                        return "Face It";
                case PlaylistIds::RankedBasketballDoubles:
                        return "Ranked Basketball Doubles";
                case PlaylistIds::RankedRumble:
                        return "Ranked Rumble";
                case PlaylistIds::RankedBreakout:
                        return "Ranked Breakout";
                case PlaylistIds::RankedSnowDay:
                        return "Ranked SnowDay";
                case PlaylistIds::HauntedBall:
                        return "Haunted Ball";
                case PlaylistIds::BeachBall:
                        return "Beach Ball";
                case PlaylistIds::Rugby:
                        return "Rugby";
                case PlaylistIds::AutoTournament:
                        return "Auto Tournament";
                case PlaylistIds::RocketLabs:
                        return "Rocket Labs";
                case PlaylistIds::RumShot:
                        return "Rum Shot";
                case PlaylistIds::GodBall:
                        return "God Ball";
                case PlaylistIds::CoopVsAI:
                        return "Coop Vs AI";
                case PlaylistIds::BoomerBall:
                        return "Boomer Ball";
                case PlaylistIds::GodBallDoubles:
                        return "God Ball Doubles";
                case PlaylistIds::SpecialSnowDay:
                        return "Special SnowDay";
                case PlaylistIds::Football:
                        return "Football";
                case PlaylistIds::Cubic:
                        return "Cubic";
                case PlaylistIds::TacticalRumble:
                        return "Tactical Rumble";
                case PlaylistIds::SpringLoaded:
                        return "Spring Loaded";
                case PlaylistIds::SpeedDemon:
                        return "Speed Demon";
                case PlaylistIds::RumbleBM:
                        return "Rumble BM";
                case PlaylistIds::Knockout:
                        return "Knockout";
                case PlaylistIds::Thirdwheel:
                        return "Thirdwheel";
                case PlaylistIds::MagnusFutball:
                        return "Magnus Futball";
        }

        return std::string();
}

void LogGoalReplayTime::reset_data(data & s) {
        s.did_everyone_skip              = false;
        s.game_guid                      = "";
        s.game_num                       = -1;
        s.milliseconds_spent_countdown   = -1;
        s.milliseconds_spent_goal_replay = -1;
        s.milliseconds_spent_post_goal   = -1;
        s.playlist_id                    = PlaylistIds::Unknown;
        s.playlist_name                  = "";
}

/*
 * for when you've inherited from BakkesMod::Plugin::PluginWindow.
 * this lets  you do "togglemenu (GetMenuName())" in BakkesMod's console...
 * ie
 * if the following GetMenuName() returns "xyz", then you can refer to your
 * plugin's window in game through "togglemenu xyz"
 */

/*
/// <summary>
/// do the following on togglemenu open
/// </summary>
void LogGoalReplayTime::OnOpen() {};

/// <summary>
/// do the following on menu close
/// </summary>
void LogGoalReplayTime::OnClose() {};

/// <summary>
/// (ImGui) Code called while rendering your menu window
/// </summary>
void LogGoalReplayTime::Render() {};

/// <summary>
/// Returns the name of the menu to refer to it by
/// </summary>
/// <returns>The name used refered to by togglemenu</returns>
std::string LogGoalReplayTime::GetMenuName() {
        return "$safeprojectname";
};

/// <summary>
/// Returns a std::string to show as the title
/// </summary>
/// <returns>The title of the menu</returns>
std::string LogGoalReplayTime::GetMenuTitle() {
        return "";
};

/// <summary>
/// Is it the active overlay(window)?
/// </summary>
/// <returns>True/False for being the active overlay</returns>
bool LogGoalReplayTime::IsActiveOverlay() {
        return true;
};

/// <summary>
/// Should this block input from the rest of the program?
/// (aka RocketLeague and BakkesMod windows)
/// </summary>
/// <returns>True/False for if bakkesmod should block input</returns>
bool LogGoalReplayTime::ShouldBlockInput() {
        return false;
};
*/