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
		-- Initialization
		Tamara_about_bots_nodes = 0

		tux_says(_"Hi, I'm %s.", get_player_name())
		npc_says(_"Welcome to the Library, I'm Tamara.")
		npc_says(_"Please take a look around and see if there is something here that interests you.")
		npc_says(_"You are welcome to ask me if you have any questions.")
		set_bot_name(_"Tamara - Librarian")
		show("node10")
	end,

	EveryTime = function()
		if (tamara_shelf_count == 1) and
		   (not tamara_shelf_count_1_done) then
			npc_says(_"Hey, please don't make such a mess here.")
			tamara_shelf_count_1_done = true
		elseif (tamara_shelf_count == 2) and
		       (not tamara_shelf_count_2_done) then
			npc_says(_"Please put everything back in place if you don't need it.")
			tamara_shelf_count_2_done = true
		elseif (tamara_shelf_count == 3) and
		       (not tamara_shelf_count_3_done) then
			npc_says(_"Please be careful with these books. They are older than most computers in this town.")
			tamara_shelf_count_3_done = true
			tamara_shelf_stuff = "done"
		elseif (tamara_shelf_count) and
		       (tamara_shelf_count > 3) then
			npc_says("ERROR: Tamara.dialog, EveryTime LuaCode: unhandled value for tamara_shelf_count, please report. value is: %s", tamara_shelf_count)
		end

		if (has_met("Tamara")) then
			npc_says(_"What can I help you with?")
		end

		if (has_met("Sorenson")) and
		(not Tamara_talked_about_sister) then
			show("node1")
		end

		show_if(((not won_nethack) and
				      (not Tamara_asked_hacking)), "node2")

		show_if(Tamara_bot_apocalypse_book, "node21")

		if (Ewalds_296_needs_sourcebook) and
		   (not Tamara_have_296_book) then
			show("node50")
		end

		-- function to hide node in the topic Tamara_about_bot_nodes
		function hide_node_about_bots(node)
			Tamara_about_bots_nodes = Tamara_about_bots_nodes - 1
			hide("node" .. node)
			if (Tamara_about_bots_nodes <= 0) then
				pop_topic()
			end
		end

		if (Kevin_sigtalk) and
		   (not tamara_robotic_girlfriends_node) then
			Tamara_about_bots_nodes = Tamara_about_bots_nodes + 1
			tamara_robotic_girlfriends_node = true;
			show("node34")
		end

		show_if(Tamara_talked_about_bots and
		            (Tamara_about_bots_nodes > 0), "node31")

		show("node99")
	end,

	{
		id = "node1",
		text = _"Hmm, you look very similar to another person I met, by the name of Sorenson.",
		code = function()
			npc_says(_"Well, it's understandable. We are sisters.")
			npc_says(_"Or actually half sisters. She is also half crazy, so we don't communicate much.")
			npc_says(_"*sigh*")
			npc_says(_"We used to. Even ran this library together.")
			npc_says(_"Then she started reading more and more books and sitting in front of the computer day and night, never sleeping.")
			npc_says(_"In the end she completely lost her marbles, sadly.")
			npc_says(_"Now all she does is sit locked in her house staring into the computer.")
			Tamara_talked_about_sister = true
			hide("node1")
		end,
	},
	{
		id = "node2",
		text = _"I would like to learn how to hack.",
		code = function()
			Tamara_asked_hacking = true
			if (Tamara_talked_about_sister) then
				npc_says(_"My sister used to play Nethack all the time, about the time she became really good at hacking.")
			else
				npc_says(_"Everyone I know who is good with computers always talks about beating Nethack.")
			end
			npc_says(_"I think there might be a version on the town's computers.")
			npc_says(_"I've never played it.")
			npc_says(_"That might be why I'm no good with computers.")
			hide("node2")
		end,
	},
	{
		id = "node10",
		text = _"I see you have a huge source code book collection. Mind if I buy some from you?",
		code = function()
			npc_says(_"This is a library, not a book shop.")
			npc_says(_"However, valuable books have a tendency to simply vanish and never get returned by some people...")
			npc_says(_"Especially strangers just passing by...")
			npc_says(_"Thus I'm forced to take a deposit for each book.")
			hide("node10") show("node11")
		end,
	},
	{
		id = "node11",
		text = _"So what interesting books do you have available right now?",
		code = function()
			npc_says_random(_"Some of these might interest you.",
				_"I only have a few programming volumes, feel free to look through them.")
			trade_with("Tamara")
			show_if(not Tamara_bot_apocalypse_book, "node20")
			show_if(not Tamara_talked_about_bots, "node30")
		end,
	},
	{
		id = "node20",
		text = _"Do you have any books about the bot apocalypse?",
		code = function()
			npc_says(_"I'm writing one, but it isn't complete, and there are no publishers left.")
			Tamara_bot_apocalypse_book = true
			hide("node20")
		end,
	},
	{
		id = "node21",
		text = _"Have you been progressing on your book about the bots apocalypse?",
		code = function()
			npc_says_random(_"I need more time to finish it.",
				_"Page-by-page, my book is growing up.",
				_"Sorry, but you have to wait a bit more before you can read it.")
			hide("node21")
		end,
	},
	{
		id = "node30",
		text = _"Do you have any books about robotics?",
		code = function()
			npc_says(_"Sorry, most of them have been stolen or borrowed.")
			npc_says(_"However, I can tell you all about robots and automata in literature. Interested?")
			Tamara_talked_about_bots = true
			Tamara_about_bots_nodes = Tamara_about_bots_nodes + 2
			hide("node30") show("node32", "node33", "node39")
			push_topic("About bots")
		end,
	},
	{
		id = "node31",
		text = _"I would like to know some more about bots.",
		code = function()
			npc_says(_"If you mean in culture, I have some anecdote you could be interested in.")
			push_topic("About bots")
		end,
	},
	{
		id = "node32",
		text = _"Where does the word 'bot' come from?",
		topic = "About bots",
		code = function()
			npc_says(_"It is a shortening of the word 'robot', derived from the Czech word for forced labor.")
			npc_says(_"R.U.R. (Rossum's Universal Robots), a play, introduced \'robots\' as artificial people.")
			npc_says(_"In the play the robots revolted, took over the world, and killed all the humans.")
			tux_says(_"Ironic.")
			hide_node_about_bots(32)
		end,
	},
	{
		id = "node33",
		text = _"What about the creation of robots for defense?",
		topic = "About bots",
		code = function()
			npc_says(_"During the Holy Roman Empire, the Jewish people of the Prague ghetto needed protection.")
			npc_says(_"So a holy rabbi shaped a Golem out of clay, and brought it to life through rituals and writing 'emet' (truth) on its head.")
			npc_says(_"The Golem initially protected the Jews, but was brainless and stupid, and so soon became dangerously violent to even the Jews.")
			npc_says(_"It was only by trickery that the rabbi was able to even get close to the Golem.")
			npc_says(_"But as the rabbi changed 'emet' to 'met' (death), the Golem fell on him, and both the creator and creation became lifeless.")
			hide_node_about_bots(33)
		end,
	},
	{
		id = "node34",
		text = _"What can you tell me about robotic girlfriends?",
		topic = "About bots",
		code = function()
			npc_says(_"Well, the Greeks wrote down a story about Pygmalion, the sculptor of Cyprus.")
			npc_says(_"Pygmalion carved an ivory woman of far surpassing natural beauty and fell in love.")
			npc_says(_"Aphrodite, the goddess of love, brought the ivory woman to life.")
			npc_says(_"The woman, Galatea, likewise fell in love and married her creator Pygmalion.")
			npc_says(_"So you could say that this is one of the few stories that end well.")
			hide_node_about_bots(34)
		end,
	},
	{
		id = "node39",
		text = _"Can I come back to you on that later?",
		topic = "About bots",
		code = function()
			npc_says(_"Don't hesitate to talk to me again for futher questions.")
			pop_topic()
		end,
	},
	{
		id = "node50",
		text = _"Do you have a copy of Subatomic and Nuclear Science for Dummies, Volume IV?",
		code = function()
			npc_says(_"It's interesting you should be looking for that - the library has two copies.")
			tux_says(_"I need one - it's a matter of life and death!")
			npc_says(_"Life and death?")
			tux_says(_"There's a nuclear reactor going super critical under the town - if I have the book, maybe I can stop it.")
			npc_says(_"In that case, you can have it. My mission is to preserve our culture, which won't matter if we're all dead.")
			tux_says(_"Thank you, Tamara.")
			update_quest(_"An Explosive Situation", _"I was able to get a copy of Subatomic and Nuclear Science for Dummies, Volume IV from the librarian, Tamara. I'd better hurry back to Ewald's 296 with it.")
			add_item("Nuclear Science for Dummies IV")
			Tamara_have_296_book = true
			hide("node50")
			end_dialog()
		end,
	},
	{
		id = "node99",
		text = _"Thank you for the help.",
		code = function()
			npc_says_random(_"No problem, and remember to return your books in time.",
				_"I aim to ensure that the great works of literature will survive this horrible apocalypse.",
				_"That is what I'm here for. Come back at any time.")
			end_dialog()
		end,
	},
}
