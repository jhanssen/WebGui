#pragma once

#include <zep.h>
#include <string>

// Helpers to create zep editor
Zep::ZepEditor& zep_get_editor();
void zep_init(const Zep::NVec2f& pixelScale, const std::string& appRoot);
void zep_update();
void zep_show(const Zep::NVec2i& displaySize);
void zep_destroy();
void zep_load(const std::filesystem::path& file);
void zep_load(const std::string& name, const std::string& text);
