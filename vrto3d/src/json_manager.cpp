/*
 * This file is part of VRto3D.
 *
 * VRto3D is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * VRto3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with VRto3D. If not, see <http://www.gnu.org/licenses/>.
 */

#include "json_manager.h"
#include "driverlog.h"
#include "key_mappings.h"

#include <windows.h>
#include <shlobj.h>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <iomanip>
#include <sstream>

// Include the nlohmann/json library
#include <nlohmann/json.hpp>

JsonManager::JsonManager() {
    vrto3dFolder = getDocumentsFolderPath();
    createFolderIfNotExist(vrto3dFolder);
}


//-----------------------------------------------------------------------------
// Purpose: Get path to user's Documents folder
//-----------------------------------------------------------------------------
std::string JsonManager::getDocumentsFolderPath() {
    PWSTR path = NULL;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path);
    if (SUCCEEDED(hr)) {
        char charPath[MAX_PATH];
        size_t convertedChars = 0;
        wcstombs_s(&convertedChars, charPath, MAX_PATH, path, _TRUNCATE);
        CoTaskMemFree(path);
        return std::string(charPath) + "\\My Games\\vrto3d";
    }
    else {
        DriverLog("Failed to get Documents folder path\n");
    }
}


//-----------------------------------------------------------------------------
// Purpose: Create vrto3d folder if it doesn't exist
//-----------------------------------------------------------------------------
void JsonManager::createFolderIfNotExist(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path);
    }
}


//-----------------------------------------------------------------------------
// Purpose: Write a JSON to Documents/My Games/vrto3d
//-----------------------------------------------------------------------------
void JsonManager::writeJsonToFile(const std::string& fileName, const nlohmann::ordered_json& jsonData) {
    std::string filePath = vrto3dFolder + "\\" + fileName;
    std::ofstream file(filePath);
    if (file.is_open()) {
        file << jsonData.dump(4); // Pretty-print the JSON with an indent of 4 spaces
        file.close();
        DriverLog("Saved profile: %s\n", fileName.c_str());
    }
    else {
        DriverLog("Failed to save profile: %s\n", fileName.c_str());
    }
}


//-----------------------------------------------------------------------------
// Purpose: Read a JSON from Documents/My Games/vrto3d
//-----------------------------------------------------------------------------
nlohmann::json JsonManager::readJsonFromFile(const std::string& fileName) {
    std::string filePath = vrto3dFolder + "\\" + fileName;
    std::ifstream file(filePath);
    if (file.is_open()) {
        nlohmann::json jsonData;
        file >> jsonData;
        file.close();
        return jsonData;
    }
    else {
        return {};
    }
}


//-----------------------------------------------------------------------------
// Purpose: Split a string by a delimiter
//-----------------------------------------------------------------------------
std::vector<std::string> JsonManager::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}


//-----------------------------------------------------------------------------
// Purpose: Create default_config.json if it doesn't exist
//-----------------------------------------------------------------------------
void JsonManager::EnsureDefaultConfigExists()
{
    // Check if the file exists
    std::string filePath = vrto3dFolder + "\\" + DEF_CFG;
    if (!std::filesystem::exists(filePath)) {
        DriverLog("%s does not exist. Writing default config to file...\n", DEF_CFG.c_str());

        // Create the example default JSON
        nlohmann::ordered_json defaultConfig = {
            {"window_width", 1920},
            {"window_height", 1080},
            {"render_width", 1920},
            {"render_height", 1080},
            {"hmd_height", 1.0},
            {"aspect_ratio", 1.77778},
            {"fov", 90.0},
            {"depth", 0.5},
            {"convergence", 0.02},
            {"disable_hotkeys", false},
            {"tab_enable", false},
            {"reverse_enable", false},
            {"depth_gauge", false},
            {"debug_enable", true},
            {"display_latency", 0.011},
            {"display_frequency", 60.0},
            {"pitch_enable", false},
            {"yaw_enable", false},
            {"pose_reset_key", "VK_NUMPAD7"},
            {"ctrl_toggle_key", "XINPUT_GAMEPAD_RIGHT_THUMB"},
            {"ctrl_toggle_type", "toggle"},
            {"pitch_radius", 0.0},
            {"ctrl_deadzone", 0.05},
            {"ctrl_sensitivity", 1.0},
            {"user_settings", {
                {
                    {"user_load_key", "VK_NUMPAD1"},
                    {"user_store_key", "VK_NUMPAD4"},
                    {"user_key_type", "switch"},
                    {"user_depth", 0.5},
                    {"user_convergence", 0.02}
                },
                {
                    {"user_load_key", "XINPUT_GAMEPAD_GUIDE"},
                    {"user_store_key", "VK_NUMPAD5"},
                    {"user_key_type", "toggle"},
                    {"user_depth", 0.1},
                    {"user_convergence", 0.02}
                },
                {
                    {"user_load_key", "XINPUT_GAMEPAD_LEFT_TRIGGER"},
                    {"user_store_key", "VK_NUMPAD6"},
                    {"user_key_type", "hold"},
                    {"user_depth", 0.25},
                    {"user_convergence", 0.02}
                }
            }}
        };

        // Write the default JSON to file
        std::ofstream file(filePath);
        if (file.is_open()) {
            file << defaultConfig.dump(4); // Pretty-print with 4 spaces of indentation
            file.close();
            DriverLog("Default config written to %s\n", DEF_CFG.c_str());
        }
        else {
            DriverLog("Failed to open %s for writing\n", DEF_CFG.c_str());
        }
    }
    else {
        DriverLog("Default config already exists\n");
    }
}


//-----------------------------------------------------------------------------
// Purpose: Load the VRto3D display from a JSON file
//-----------------------------------------------------------------------------
void JsonManager::LoadParamsFromJson(StereoDisplayDriverConfiguration& config)
{
    // Read the JSON configuration from the file
    nlohmann::json jsonConfig = readJsonFromFile(DEF_CFG);
    
    try {
        // Load values directly from the base level of the JSON
        config.window_width = jsonConfig.at("window_width").get<int>();
        config.window_height = jsonConfig.at("window_height").get<int>();
        config.render_width = jsonConfig.at("render_width").get<int>();
        config.render_height = jsonConfig.at("render_height").get<int>();

        config.aspect_ratio = jsonConfig.at("aspect_ratio").get<float>();
        config.fov = jsonConfig.at("fov").get<float>();

        config.disable_hotkeys = jsonConfig.at("disable_hotkeys").get<bool>();
        config.debug_enable = jsonConfig.at("debug_enable").get<bool>();
        config.tab_enable = jsonConfig.at("tab_enable").get<bool>();
        config.reverse_enable = jsonConfig.at("reverse_enable").get<bool>();
        config.depth_gauge = jsonConfig.at("depth_gauge").get<bool>();

        config.display_latency = jsonConfig.at("display_latency").get<float>();
        config.display_frequency = jsonConfig.at("display_frequency").get<float>();
        config.sleep_count_max = (int)(floor(1600.0 / (1000.0 / config.display_frequency)));
    }
    catch (const nlohmann::json::exception& e) {
        DriverLog("Error reading default_config.json: %s\n", e.what());
    }
}


//-----------------------------------------------------------------------------
// Purpose: Load a VRto3D profile from a JSON file
//-----------------------------------------------------------------------------
bool JsonManager::LoadProfileFromJson(const std::string& filename, StereoDisplayDriverConfiguration& config)
{
    // Read the JSON configuration from the file
    nlohmann::json jsonConfig = readJsonFromFile(filename);

    if (jsonConfig.is_null() && filename != DEF_CFG) {
        DriverLog("No profile found for %s\n", filename.c_str());
        return false;
    }

    try {
        // Profile settings
        config.hmd_height = jsonConfig.at("hmd_height").get<float>();
        config.depth = jsonConfig.at("depth").get<float>();
        config.convergence = jsonConfig.at("convergence").get<float>();

        // Controller settings
        config.pitch_enable = jsonConfig.at("pitch_enable").get<bool>();
        config.pitch_set = config.pitch_enable;
        config.yaw_enable = jsonConfig.at("yaw_enable").get<bool>();
        config.yaw_set = config.yaw_enable;

        config.pose_reset_str = jsonConfig.at("pose_reset_key").get<std::string>();
        
        if (VirtualKeyMappings.find(config.pose_reset_str) != VirtualKeyMappings.end()) {
            config.pose_reset_key = VirtualKeyMappings[config.pose_reset_str];
            config.reset_xinput = false;
        }
        else if (XInputMappings.find(config.pose_reset_str) != XInputMappings.end() || config.pose_reset_str.find('+') != std::string::npos) {
            config.pose_reset_key = 0x0;
            auto hotkeys = split(config.pose_reset_str, '+');
            for (const auto& hotkey : hotkeys) {
                if (XInputMappings.find(hotkey) != XInputMappings.end()) {
                    config.pose_reset_key |= XInputMappings[hotkey];
                }
            }
            config.reset_xinput = true;
        }
        config.pose_reset = true;

        config.ctrl_toggle_str = jsonConfig.at("ctrl_toggle_key").get<std::string>();
        if (VirtualKeyMappings.find(config.ctrl_toggle_str) != VirtualKeyMappings.end()) {
            config.ctrl_toggle_key = VirtualKeyMappings[config.ctrl_toggle_str];
            config.ctrl_xinput = false;
        }
        else if (XInputMappings.find(config.ctrl_toggle_str) != XInputMappings.end() || config.ctrl_toggle_str.find('+') != std::string::npos) {
            config.ctrl_toggle_key = 0x0;
            auto hotkeys = split(config.ctrl_toggle_str, '+');
            for (const auto& hotkey : hotkeys) {
                if (XInputMappings.find(hotkey) != XInputMappings.end()) {
                    config.ctrl_toggle_key |= XInputMappings[hotkey];
                }
            }
            config.ctrl_xinput = true;
        }

        config.ctrl_type_str = jsonConfig.at("ctrl_toggle_type").get<std::string>();
        config.ctrl_type = KeyBindTypes[config.ctrl_type_str];

        config.pitch_radius = jsonConfig.at("pitch_radius").get<float>();
        config.ctrl_deadzone = jsonConfig.at("ctrl_deadzone").get<float>();
        config.ctrl_sensitivity = jsonConfig.at("ctrl_sensitivity").get<float>();

        // Read user binds from user_settings array
        const auto& user_settings_array = jsonConfig.at("user_settings");

        // Resize vectors based on the size of the user_settings array
        config.num_user_settings = user_settings_array.size();
        config.user_load_key.resize(config.num_user_settings);
        config.user_store_key.resize(config.num_user_settings);
        config.user_key_type.resize(config.num_user_settings);
        config.user_depth.resize(config.num_user_settings);
        config.user_convergence.resize(config.num_user_settings);
        config.prev_depth.resize(config.num_user_settings);
        config.prev_convergence.resize(config.num_user_settings);
        config.was_held.resize(config.num_user_settings);
        config.load_xinput.resize(config.num_user_settings);
        config.sleep_count.resize(config.num_user_settings);
        config.user_load_str.resize(config.num_user_settings);
        config.user_store_str.resize(config.num_user_settings);
        config.user_type_str.resize(config.num_user_settings);

        for (size_t i = 0; i < config.num_user_settings; ++i) {
            const auto& user_setting = user_settings_array.at(i);

            config.user_load_str[i] = user_setting.at("user_load_key").get<std::string>();
            if (VirtualKeyMappings.find(config.user_load_str[i]) != VirtualKeyMappings.end()) {
                config.user_load_key[i] = VirtualKeyMappings[config.user_load_str[i]];
                config.load_xinput[i] = false;
            }
            else if (XInputMappings.find(config.user_load_str[i]) != XInputMappings.end() || config.user_load_str[i].find('+') != std::string::npos) {
                config.user_load_key[i] = 0x0;
                auto hotkeys = split(config.user_load_str[i], '+');
                for (const auto& hotkey : hotkeys) {
                    if (XInputMappings.find(hotkey) != XInputMappings.end()) {
                        config.user_load_key[i] |= XInputMappings[hotkey];
                    }
                }
                config.load_xinput[i] = true;
            }

            config.user_store_str[i] = user_setting.at("user_store_key").get<std::string>();
            if (VirtualKeyMappings.find(config.user_store_str[i]) != VirtualKeyMappings.end()) {
                config.user_store_key[i] = VirtualKeyMappings[config.user_store_str[i]];
            }

            config.user_type_str[i] = user_setting.at("user_key_type").get<std::string>();
            if (KeyBindTypes.find(config.user_type_str[i]) != KeyBindTypes.end()) {
                config.user_key_type[i] = KeyBindTypes[config.user_type_str[i]];
            }

            config.user_depth[i] = user_setting.at("user_depth").get<float>();
            config.user_convergence[i] = user_setting.at("user_convergence").get<float>();
        }

    }
    catch (const nlohmann::json::exception& e) {
        DriverLog("Error reading config from %s: %s\n", filename.c_str(), e.what());
        return false;
    }

    return true;
}


//-----------------------------------------------------------------------------
// Purpose: Save Game Specific Settings to Documents\My games\vrto3d\app_name_config.json
//-----------------------------------------------------------------------------
void JsonManager::SaveProfileToJson(const std::string& filename, StereoDisplayDriverConfiguration& config)
{
    // Create a JSON object to hold all the configuration data
    nlohmann::ordered_json jsonConfig;

    // Populate the JSON object with settings
    jsonConfig["hmd_height"] = config.hmd_height;
    jsonConfig["depth"] = config.depth;
    jsonConfig["convergence"] = config.convergence;
    jsonConfig["pitch_enable"] = config.pitch_enable;
    jsonConfig["yaw_enable"] = config.yaw_enable;
    jsonConfig["pose_reset_key"] = config.pose_reset_str;
    jsonConfig["ctrl_toggle_key"] = config.ctrl_toggle_str;
    jsonConfig["ctrl_toggle_type"] = config.ctrl_type_str;
    jsonConfig["pitch_radius"] = config.pitch_radius;
    jsonConfig["ctrl_deadzone"] = config.ctrl_deadzone;
    jsonConfig["ctrl_sensitivity"] = config.ctrl_sensitivity;

    // Store user settings as an array
    for (int i = 0; i < config.num_user_settings; i++) {
        nlohmann::ordered_json userSettings;
        userSettings["user_load_key"] = config.user_load_str[i];
        userSettings["user_store_key"] = config.user_store_str[i];
        userSettings["user_key_type"] = config.user_type_str[i];
        userSettings["user_depth"] = config.user_depth[i];
        userSettings["user_convergence"] = config.user_convergence[i];

        // Append to JSON array in the main config
        jsonConfig["user_settings"].push_back(userSettings);
    }

    writeJsonToFile(filename, jsonConfig);
}
