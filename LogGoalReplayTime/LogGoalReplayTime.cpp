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
        logbook.open(gameWrapper->GetDataFolder().append("GoalReplayTime.csv"));
        if (!logbook) {
                LOG("UNABLE TO OPEN LOGBOOK TO LOG GOAL REPLAY TIMES! CANNOT MOVE FORWARD!");
                LOG("TRY RELOADING THE PLUGIN: plugin reload/load LogGoalReplayTime");

                return;
        }

        HookedEvents::AddHookedEvent(
                "Function GameEvent_Soccar_TA.PostGoalScored.BeginState",
                [](std::string eventName) {
                        // AFTER GOAL IS SCORED !
                },
                true);
        HookedEvents::AddHookedEvent(
                "Function GameEvent_Soccar_TA.PostGoalScored.EndState",
                [](std::string eventName) {
                        // idk yet
                        // WHEN THE "XYZ" SCORED" TEXT IS OVER!!!
                },
                true);
        HookedEvents::AddHookedEvent(
                "Function GameEvred.BeginState",
                [](std::string eventName) {
                        // AFTER GOAL IS SCORED !
                },
                true);
        HookedEvents::AddHookedEvent(
                "Function GameEvent_Soccar_TA.PostGoalScored.BeginState",
                [](std::string eventName) {
                        // AFTER GOAL IS SCORED !
                },
                true);
        HookedEvents::AddHookedEvent(
                "Function GameEvent_Soccar_TA.PostGoalScored.BeginState",
                [](std::string eventName) {
                        // AFTER GOAL IS SCORED !
                },
                true);
        HookedEvents::AddHookedEvent(
                "Function GameEvent_Soccar_TA.PostGoalScored.BeginState",
                [](std::string eventName) {
                        // AFTER GOAL IS SCORED !
                },
                true);
}

/// <summary>
///  do the following when your plugin is unloaded
/// </summary>
void LogGoalReplayTime::onUnload() {
        // destroy things
        // dont throw here
}

/// <summary>
/// This call usually includes ImGui code that is shown and rendered (repeatedly,
/// on every frame rendered) when your plugin is selected in the plugin
/// manager. AFAIK, if your plugin doesn't have an associated *.set file for its
/// settings, this will be used instead.
/// </summary>
void LogGoalReplayTime::RenderSettings() {
        // for imgui plugin window
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