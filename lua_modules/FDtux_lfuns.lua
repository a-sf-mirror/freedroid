--[[

  Copyright (c) 2013 Samuel Degrande

  This file is part of Freedroid

  Freedroid is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Freedroid is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Freedroid; see the file COPYING. If not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
  MA  02111-1307  USA

]]--

--!
--! \file FDtux_lfuns.lua
--! \brief This file contains the Lua functions to add to FDtux (Lua binding of Tux).
--!

local function output_msg(msg)
	if (run_from_dialog()) then
		cli_says(msg, "NO_WAIT")
		npc_says("")
	else
		display_big_message(msg)
	end
end

--! \addtogroup FDtux
--!@{
-- start FDtux submodule

--! \fn void says(self, format, ...)
--!
--! \brief Display a formatted text in the chat log
--!
--! Output a text in the chat log and wait the user to click.\n
--! The text is displayed using the color/font associated to Tux words.\n
--! If the last argument is "NO WAIT", the user's click is not waited. This
--! optional argument is not used to create the formatted text.
--!
--! \param self   [\p FDtux]  FDtux instance
--! \param format [\p string] Format string (as used in string.format())
--! \param ...    [\p any]    Arguments expected by the format string to create the displayed text
--!
--! \bindtype lfun

function FDtux.says(self, format, ...)
	local text, no_wait = chat_says_format('\1- ' .. format .. '\n', ...)
	chat_says(text, no_wait)
end

-- end FDtux submodule
--!@}
