#pragma once

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "../../Common.h"

namespace FileSystem
{
	std::vector<char> ReadFile(const std::string& FileName);

	void LoadTextures();
}

#endif