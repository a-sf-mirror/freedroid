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

return {
	FirstTime = function()
		show("node20", "node30", "node40", "node50")
	end,

	EveryTime = function()
		tux_says(_"Um... Hello?")
		npc_says(_"AH! A Visitor! Come in! Come in!")
		tux_says(_"I think I should go now. Right now. See you later.")
		npc_says(_"No, don't go. I will not let you go before we talk. Important things.")
		npc_says(_"I can help you understand computers... Greater than ever before.")
		show("node99")
	end,

	{
		id = "node20",
		text = _"Who are you?",
		code = function()
			npc_says(_"I? Sorenson is a great computer person.")
			npc_says(_"Knows the deep codes. Walks the paths.")
			npc_says(_"Sees the light of all chips. Knows how to use every keyboard.")
			tux_says(_"Erm...")
			npc_says(_"Sees the bit. Knows the bytes. Explores the great networks.")
			npc_says(_"The best friend of the clean and pure code.")
			npc_says(_"The great light on the night sky of the hollow web of computer power.")
			tux_says(_"Yes, I get the idea...")
			npc_says(_"Seeker of clean languages. The great knower of obscure secrets.")
			npc_says(_"The one who speaks to hard drives. The best --")
			tux_says(_"I KNOW! I KNOW!")
			npc_says(_"Oh.")
			npc_says(_"Sorenson did not tell you most yet.")
			npc_says(_"She did not tell you she is the great controller of light.")
			npc_says(_"She did not tell you she is the walker of the dark corridors --")
			tux_says(_"STOP! STOP! I CANNOT TAKE THIS ANYMORE! MY HEAD HURTS!")
			npc_says(_"Apology is being extended by me. Sorenson overdid it.")
			npc_says(_"Sorenson the coder greets you.")
			set_bot_name(_"Sorenson - Mystery coder")
			hide("node20")
		end,
	},
	{
		id = "node30",
		text = _"Why are you like this? What happened to you?",
		code = function()
			npc_says(_"Was learning COBOL. Pain. Torture. Horrible screaming.")
			npc_says(_"Sorenson asked for help. Many days and nights.")
			npc_says(_"Light arrived. Light offered help. Sorenson changed.")
			npc_says(_"Free person now.")
			hide("node30") show("node31")
		end,
	},
	{
		id = "node31",
		text = _"What is this COBOL?",
		code = function()
			npc_says(_"COBOL is the mind-crippler.")
			npc_says(_"Don't use it. Forget the cursed name. COBOL is pain. COBOL is hate.")
			hide("node31")
		end,
	},
	{
		id = "node40",
		text = _"I see you have a huge source code book collection. Mind if I buy some from you?",
		code = function()
			npc_says(_"Of course not. Trade is good.")
			trade_with("Sorenson")
			hide("node40") show("node41")
		end,
	},
	{
		id = "node41",
		text = _"Can I take a look at your books again?",
		code = function()
			npc_says(_"Yes. Sure. Of course.")
			trade_with("Sorenson")
		end,
	},
	{
		id = "node50",
		text = _"You got me curious. How can you help me understand computers better?",
		code = function()
			npc_says(_"Simplicity! Sign this contract. No need to read it.")
			npc_says(_"You trust Sorenson, right?")
			push_topic("Sorenson contract") show("node51", "node79")
		end,
	},
	{
		id = "node51",
		text = _"Sure. I'll sign it.",
		topic = "Sorenson contract",
		code = function()
			npc_says(_"Go0d. H3re is th3 con7ract. S1gn it!")
			tux_says(_"Ok, here go --")
			npc_says(_"N0! 1n bl00d!")
			hide("node51") show("node52", "node55", "node78")
		end,
	},
	{
		id = "node52",
		text = _"Um... Your voice sounds strange. Is everything fine?",
		topic = "Sorenson contract",
		code = function()
			npc_says(_"519n 7h3 c0n7r4c7!!!!")
			hide("node52") show("node53")
		end,
	},
	{
		id = "node53",
		text = _"Please calm down. You must speak clearly and slowly. Please stop breathing smoke like that, it bothers my throat. And what is the deal with the red glowing eyes? You should see a doctor. ",
		topic = "Sorenson contract",
		code = function()
			npc_says(_"519n 17!!!!")
			hide("node53")
		end,
	},
	{
		id = "node55",
		text = _"Sure, no problem... Now, how shall I sign this?",
		topic = "Sorenson contract",
		code = function()
			hide("node52", "node53", "node55", "node78", "node79")
			show("node60", "node61", "node62", "node63", "node64", "node65", "node66")
		end,
	},
	{
		id = "node60",
		text = _"Tux.",
		topic = "Sorenson contract",
		code = function()
			next("node70")
		end,
	},
	{
		id = "node61",
		text = _"Tux, the Linarian",
		topic = "Sorenson contract",
		code = function()
			next("node70")
		end,
	},
	{
		id = "node62",
		text = _"I accept the contract, Tux",
		topic = "Sorenson contract",
		code = function()
			next("node70")
		end,
	},
	{
		id = "node63",
		text = _"Elbereth",
		topic = "Sorenson contract",
		code = function()
			npc_says(_"4444! 17 burn5! 7'm m3171n9!")
			npc_says(_"44444444444444444!")
			drop_dead()
		end,
	},
	{
		id = "node64",
		text = _"Francis",
		topic = "Sorenson contract",
		code = function()
			npc_says(_"H4! Y0ur s0ul 15 min3, Francis!")
			npc_says(_"Wh47? N07 w0rk1b9?")
			tux_says(_"Erm. I have a confession to make. My name is not Francis.")
			npc_says(_"N0000000000!!!!!!!")
			npc_faction("crazy", _"COBOL Programmer - Madly Enraged")
			hide("node64")
			end_dialog()
		end,
	},
	{
		id = "node65",
		text = _"The great destroyer of evil, the purger of all things foul, the friend of mankind, the third duke of Linarius, esquire, Doctorate of Computer Sciences, high priest of Kernelius, the ray of light in the darkness, the bender of spoons, His Great Magnificence, Tux.",
		topic = "Sorenson contract",
		code = function()
			npc_says(_"Huh?")
			tux_says(_"I... Don't feel... good... Blood loss... So... Cold...")
			npc_says(_"N0! Idi07!")
			kill_tux()
			-- hide("node65")
		end,
	},
	{
		id = "node66",
		text = _"X",
		topic = "Sorenson contract",
		code = function()
			npc_says(_"N0! S7up1d illi73r4t3 p3n9u1n! D13!!")
			hide("node66")
			npc_faction("crazy", _"COBOL Programmer - Madly Enraged")
			end_dialog()
		end,
	},
	{
		id = "node70",
		text = _"Huh? I d0n'7 f33l g00d.",
		topic = "Sorenson contract",
		code = function()
			npc_says_random(_"Yes! I am free! At last I am free!",
							_"Yes! I am free!",
							_"Yes! I am free at last!")
			tux_says(_"W417... 1 f331... H0rr1b13...")
			npc_says(_"Yes, fool! You have freed me! I can finally die! Goodbye idiot!")
			improve_skill("programming")
			hide("node60", "node61", "node62", "node63", "node64", "node65", "node66")
			drop_dead()
		end,
	},
	{
		id = "node78",
		topic = "Sorenson contract",
		text = _"In blood? Yuck. I am not going to do this, blood is too messy for a binding legal contract. Forget about it.",
		code = function()
			npc_says(_"N00oo00oo0! T1m3 to d1e, m0rtal foo1!")
			hide("node78")
			npc_faction("crazy", _"COBOL Programmer - Madly Enraged")
			end_dialog()
		end,
	},
	{
		id = "node79",
		topic = "Sorenson contract",
		text = _"There is no way I will sign something without reading it. Forget about it.",
		code = function()
			npc_says(_"N00oo00oo0! T1m3 to d1e, m0rta1 foo1!")
			hide("node79")
			npc_faction("crazy", _"COBOL Programmer - Madly Enraged")
			end_dialog()
		end,
	},
	{
		id = "node99",
		text = _"I really must be going now. Goodbye.",
		code = function()
			npc_says(_"Please come back later, Linarian. We must talk again.")
			end_dialog()
		end,
	},
}
