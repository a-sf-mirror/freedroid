---------------------------------------------------------------------
-- This file is part of Freedroid
--
-- Freedroid is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--
-- Freedroid is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with Freedroid; see the file COPYING. If not, write to the
-- Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
-- MA 02111-1307 USA
----------------------------------------------------------------------

local Tux = FDrpg.get_tux()

return {
	EveryTime = function()
		npc_says(_"Welcome to the MegaSys Factory complex.")
		npc_says(_"Access is restricted to authorized personnel.")
		npc_says(_"Proof of authorization is required.")
		if (HF_EntranceBot_MSStockCertificateOpensGate) then
			Tux:says(_"How many times do I have to show you my certificate, tin can?")
			if (Tux:has_item("MS Stock Certificate")) then
				npc_says(_"[b]Validating certificate...[/b]")
				npc_says(_"[b]Validation complete.[/b]", "NO_WAIT")
				npc_says(_"[b]Certificate valid.[/b]", "NO_WAIT")
				npc_says(_"You may enter.")
				change_obstacle_state("HF-EntranceInnerGate", "opened")
				hide("node1", "node2", "node3")
			else
				Tux:says(_"Oh, I... uh... must have left it in my other armor.")
				Tux:says(_"I'll go get it for you. Yes. Because I TOTALLY know where it is. I hope.")
			end
		else
			show("node1", "node2", "node3")
		end
		show("node99")
	end,

	{
		id = "node1",
		text = _"I am THE ONE.",
		code = function()
			npc_says(_"You are THE ONE without permission.")
			npc_says(_"Please consider leaving.")
			hide("node1")
			end_dialog()
		end,
	},
	{
		id = "node2",
		text = _"I am working here.",
		code = function()
			npc_says(_"Me too.")
			npc_says(_"Please prove your statement.")
			Tux:says(_"Do I look like a typical MegaSys slave, errr, worker to you, stupid bot?")
			npc_says(_"No insults, please. But, no")
			if (Tux:has_item("MS Stock Certificate")) then
				Tux:says(_"But I have this certificate")
				npc_says(_"[b]Validating certificate...[/b]")
				npc_says(_"[b]Validation complete.[/b]", "NO_WAIT")
				npc_says(_"[b]Certificate valid.[/b]", "NO_WAIT")
				npc_says(_"You may enter.")
				change_obstacle_state("HF-EntranceInnerGate", "opened")
				HF_EntranceBot_MSStockCertificateOpensGate = true
			else
				end_dialog()
			end
			hide("node2")
		end,
	},
	{
		id = "node3",
		text = _"I have come to save the world, I don't need any proof.",
		code = function()
			npc_says(_"Feel uncertain about the future?")
			npc_says(_"Purchase the MegaSys Security Bundle to help safeguard your home.")
			npc_says(_"It contains:")
			npc_says(_"The latest version of the [b]MegaSys[/b] operating system for [b]ONE DROID[/b]")
			npc_says(_"Ten mini surveillance robots.")
			npc_says(_"The book 'Nuclear Science for Dummies IV'.")
			npc_says(_"And a MegaSys Vision Enhancement Device 3000 - what you cannot see, can't see you either!")
			npc_says(_"If you order [b]RIGHT NOW[/b], we will [b]SHIP FOR FREE!!![/b]")
			Tux:says(_"No, thanks.")
			hide("node3")
		end,
	},
	{
		id = "node99",
		text = _"Bye",
		code = function()
			npc_says(_"Remember, MegaSys products are the best!")
			end_dialog()
		end,
	},
}
