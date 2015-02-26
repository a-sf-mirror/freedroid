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

title_screen{
song = "Bleostrada.ogg",
text = [[
            O EDITOR DE MAPAS DO FREEDROIDRPG

=== INTRODUÇÃO ===

FreedroidRPG vem com um editor de mapas embutido. Esse editor permite-lhe controlar qualquer aspecto de um mapa normal do FreedroidRPG e salvar as mudanças.

Você pode acessá-lo pelo menu principal (clique em "Editor de Mapas") ou executando 'freedroidRPG -e'.

    --- Dicas ---
Para habilitar/desabilitar as descrições de interface quando o mouse paira sobre algo, clique no ícone de balão de conversa localizado perto da borda direita da janela (Linha de botões inferior).

    --- Detalhes de Sumário ---
Detalhes de sumário sobre obstáculos e itens irão aparecer se você clicar com o botão direito do mouse no seletor de objetos superior.

    --- Navegação ---
Para mudar o nível atual, clique no número do nível no mini mapa no canto inferior direito, ou procure pelo nível desejado no menu do editor (descrito posteriormente)

    --- Editando mapas ---
Existem quatro tipos de modo de edição: edição de obstáculos, edição de piso, edição de item e edição de coordenadas.

O botão selecionado ao lado inferior esquerdo indica os objetos que você pode selecionar ou colocar.
Quando o botão está selecionado, e você está no modo de distruibuição, o objeto que você irá posicionar será indicado pela fita no topo da tela. A seleção na fita é dividida nas quatro abas imediatamente abaixo.

Você pode selecionar os tipos de obstáculos que você deseja que sejam posicionados no mapa, no selecionador de objeto superior. Apelas clique e selecione. Obstáculos são divididos em grupos, para fornecer uma melhor visualização.

Pressionando espaço, você irá entrar no modo de seleção indicado pelo cursor de troca. Você pode selecionar apenas grupos de objetos representados pelo modo atual de seleção de objeto.
Nota importante: Você só será capaz de selecionar coisas que estão inclusas no modo de seleção atual. Se você está no modo obstáculo, você não será capaz de selecionar itens ou pisos.


        Modo edição de obstáculos:

Para selecionar esse modo, clique no botão "Obstáculo", no selecionador de categorias, na área esquerda inferior.
Tendo selecionado um obstáculo, apenas clique em qualquer lugar no mapa para coloca-lo na posição do cursor.
Caso clicar seja impreciso, você também pode usar seu teclado numérico para posicionar objetos.
Clicando mais a esquerda (isso irá mostrar uma pequena grade) dos cinco botões a cima do seletor de categoria para ter uma grade com numeros exibidos. Use o botão esquerdo do mouse para trocar entre grade ligada e desligada e o botão direito para trocar os tipos de grade
Esses números se referem ao número do seu teclado numérico, se você tiver um. Pressionando '1' irá colocar o obstáculo que está em destaque no seletor de objetos na posição do digito '1' na grade roxa.
Caso o posicionamento de linhas de parede dessa forma seja muito ineficiente, você pode simplesmente segurar o botão esquerdo do mouse e a linha de paredes será colocada enquanto você mover seu cursor se você tiver um objeto de parede selecionado. Isso funciona com a maioria das paredes comuns em FreedroidRPG
Enquanto segurar o botão esquerdo do mouse e colocando paredes, um clique com o botão direito do mouse irá remover todas as paredes que você desenhou após começar a segurar o botão esquerdo do mouse
Existem alguns objetos especiais. Paredes de vidro e paredes de blocos quebrados, mas também existem barris e engradados que podem ser destruidos com alguns golpes, enquanto os dois ultimos possam também liberar itens. Baús podem ser abertos e conter itens também.
O símbolo com pegadas cruzadas nao é realmente um objeto, porém uma area pura e invisivel bloqueada ('retangulo de colisão'). Retangulos de colisão sao a essência de cada objeto, desde que eles previnam você de apenas andar através deles da mesma forma que é possível para pontos de controle ou ladrilhos.

            Selecionando obstáculos

Mantendo pressionado o botão esquerdo do mouse você pode selecionar um retangulo de obstáculos. Após soltar o botão do mouse, os obstáculos selecionados irão trocar para uma cor diferente, indicando que eles estão selecionados. Para selecionar obstaculos que não estão no alcance do retângulo de seleção, segure pressionado 'Ctrl' e clique no obstáculo ou selecione outro retângulo para eles.
Você pode ter selecionado automaticamente muitos obstáculos com um clique. Você pode trocar entre os obstáculos clicando no icone com a cadeira e a prateleira sobre ele, ou pressionando 'n'.
The icon with the trash can delete the selected obstacle.
You can also cut (Ctrl+x, can also be used to delete obstacles by just not pasting them again ;) ), copy (Ctrl+c) and paste (Ctrl+v) cut or copied obstacles.
You can move selected obstacles holding down the left shift key while dragging the obstacle around. However, this may be quite imprecise.

            Colocando objetos em Baús

Simply select the desired chest and click the most left button in the upper button row.
You will be forwarded to a screen that looks like the shop screen.
There will be a knife displayed (which is actually not placed in the chest by the way) select it and click on the 'sell' button.
Select the items you want to be dropped when the player opens the chest.
These items will be displayed in the upper shop bar.
In order to remove one of these items, simply select it and click on 'buy'.
The red cross gets you out of the screen.

            Adding Text to a Sign

Select the sign and add an obstacle label with the sign text. Save the map and exit.
Open the level file (map/levels.dat) and find the new obstacle label. Change the line above the text from 'type=30' to 'type=32' and save.
Now when you click on the sign in the game your short message will appear.

            Adicionando Diálogo a um Terminal

Select the terminal and add an obstacle label with the dialog name you wish to use. Save the map and exit.
Open the level file (map/levels.dat) and find the new obstacle label.
Change the line above the text from 'type=30' to 'type=32' and save. Now when you click on the terminal in the game it will start the dialog you selected.

        Floor edit mode:

The floor edit mode works quite similar to the obstacle edit mode. You can select different types of floors at the object selector.
To fill a region with a single floor tile, first select the tile to use, then click and drag the left mouse button until it covers the desired region. The floor tiles are placed on the current floor layer.
There are no floors that are special in any way, they are pure decoration.

The visibility of floor layers can be controlled by a button with the layer icon. The button is only displayed for levels with multilayer floors.
Left click on the button switches between a single floor layer displayed and all floor layers displayed. Right click on the button changes the current floor layer.

            Selecting floor types

Selecting is as easy as in the the obstacle mode. Floor tiles can be moved to with the method described above.
For levels with multilayer floors only visible floor layers are selected. When a single floor layer is visible, only the tiles in the current floor layer are selected.

In order to have a look at the floor only, click the lamp icon to have no obstacles displayed. Another click will let obstacles appear again.
The icon with the turquoise rectangle displays collision rectangles. These rectangles indicate the blocking-area of an obstacle. Tux can't walk on such an area.
If you turn it on and playtest (explained later) your map, the rectangles are still displayed if activated which is quite useful for testing whether the player can pass a gap or not.

        Item edit mode:

You can place items to be used by the player on the map, too.
Items are objects that the player can pick up. They can be they can be carried, some even be used or equipped.
Some items are used to move the plot forward, others provide bonuses to the player, while still others do nothing at all.
Select the item mode and click on an item displayed at the object selector. For some items, you must specify an amount before they are placed.
You can set it by clicking the arrow buttons or dragging the blue orb to the left or the right.
Press 'g' to have a better overview of what items are available (can also be used for dropping, items will be dropped at the crosshair). Hit 'Esc' to abort the process without dropping any items.
You can also click the icon with the crossed-out boots to perform this.


        Waypoint edit mode:

Currently, droids (meaning all non-player characters) move around on levels using predefined waypoints.
To plant a waypoint, press the 'w' key. This will toggle the waypoint on the rectangle under the crosshair.
You can also click the map at a position you want to have a waypoint having this mode activated. Another click somewhere else plants another waypoint and automatically connects the previous selected one with it.
Clicking on a preexisting waypoint lets you connect it with another one (just click the other one, too, to do it).
However, there is a difference between those two planting methods. When you connect two waypoints using the keyboard, the connections will be unidirectional.
That means that when you make a connection from waypoint A to waypoint B, the bot will only be able to walk from A to B but not back.
You can remove an unidirectional connection by 'overlying' it with another one going into the very direction as the one you want to delete (this does not work with bidirectional connections!).
Bidirectional connections are however automatically done using the click method to connect waypoints.
Important note: It is not possible to connect waypoints on different maps with each other!
Waypoints are also used to position randomly spawned bots. However this might be inappropriate for some waypoints.
There are 'normal' ones which are white, for respawning bots and 'special', purple ones which should be used for NPCs. The normal ones are used for spawned bots, the purple ones should be used for NPCs.
You can select these different types of waypoints in the upper selection bar. To turn a normal waypoint into a purple one or back again, press shift+w.
Please make sure that paths between waypoints are not blocked by an obstacle in between of two waypoints.
To automatically check a entire map for this, you can use the map level validator which is explained later.


        Planting Labels:

There are two kinds of tables: map labels and obstacle labels.
Please make sure that each label ID is unique.
Giving an empty string will delete the respective label.


            Planting map labels

Map labels are used to define starting locations of NPCs (see ReturnOfTux.droids), events that occur when Tux moves over them (see events.dat), or locations used for movement of NPCs through the lua script files (events, quests, and the dialogs).
To define a new map label, press the 'm' key on the keyboard or click the button with the M on the sign on it. You will be prompted for the map label. Note that there will be a colorful circle appearing on any map tile that has been fitted with a map label.
The map label will be automatically planted on the tile in the middle of the screen.
You can switch the displaying of droids/NPCs on or off pressing the button with the 302 bot on it.

            Planting obstacle labels

Obstacle labels are important so that some obstacles can be marked for events to happen (for example during a quest). If e.g. an event is supposed to remove a special wall obstacle, then this obstacle must be given a name or ID first, so it can be referred to later in the definition of the event.
You can also use them to add dialogs to obstacles, so you can talk to them as they were NPCs.
To plant a label on an obstacle, you must first mark this obstacle (see obstacle mode explanation above).
Clicking the icon with the sign and the O on it you will be prompted for the new label to attach to this obstacle.

You can toggle display of map labels using the small icon with the label-circle on it.

        Salvando mapas:

In order to save a map, click the small disk icon in the upper right area of the editor screen. The door icon lets you exit the editor.
Você também pode fazer isso através do menu que é aberto pressionando a tecla 'Esc'.


Dicas gerais:

	Getting overview
In order to change the zoom factor, press the 'o' key or click the icon with the magnifying glass on it.
Try left and right clicking in order to access different zoom factors.


	The editor menu

Você pode acessar este menu pressionando ESC.

		"Level:"
Here you can easily navigate to other levels. You can either use the arrow keys having this option selected
in order to switch to the next or previous (refers to level numbers) level, or, clicking on it, enter the number of the desired level and press enter.

		Level options
				Level:	See above for explanation
				Name:	The map name displayed at the GPS in the upper right corner of the game screen. You can disable the GPS in-game using the options menu.
				Size:	You can increase or reduce the size of your level. Select the desired edge where you want to add/remove a line of tiles and click the <- or -> arrow buttons.
				Floor layers: In order to change the number of floor layers for the current level, use the <- or -> arrow buttons.
				Edge interface:	Here you can set the levels that shall be next to the current level. Enter the level number for the respective edge.
								A level can only have one adjacent level (one it touches edges with) in each of the four cardinal directions (North, South, West, East).
				Random dungeon:	If you set this option to 'Yes', the map will automatically generate a dungeon. You set the number of teleporters to and from this map clicking on the option.
								Randomly generated dungeons automatically will have everything necessary, like waypoints, bots, and obstacles, set.
				Item drop class for obstacles:	Set of what item class items dropped by barrels/chests/crates should be.
				Teleport blockade:	Make it (im)possible to teleport away from a level.
				Teleport pair:	This important if you make a dungeon that is not directly connected to another map. You can set the number of exits and entrances for a randomly generated dungeon here.
				Light:			How much light would you like to have? Press space to switch between ambient (general brightness of the present map) and bonus (light emitted by some obstacles, such as lamps or mushrooms) mode.
				Background music:	Here you can set a music track to be played while the player walks around on the map. Possible tracks can be found in ./sound/music/ .
									Just enter the file name including the .ogg extension.
				Infinite running Stamina:	If you have this set to "yes", Tux' stamina will not decrease while running across the map. This should only be used if the level has no hostile NPCs on it, like on level 0, the Town, for example.
				add/rem level:		Lets you add a new level or remove the current level.

		Opções avançadas
Here you can run the map level validator.
The map level validator checks all the paths between connected waypoints to ensure they are not blocked by obstacles. More detailed output explaining which paths are blocked can be found in the terminal, given the case that the game is run using it, or a global error output file.
It can also check if you have obstacles near map borders in a critial way.
This should ALWAYS be run before calling a map finished.
"freedroidRPG -b leveltest" does also run this check.

		Playtest mapfile
Allows you to playtest your modifications easily.
If you leave this mode, obstacle changes that were made while playing, destroying crates for example, will be reverted to the time where you started playtesting.




Atalhos:
space					toggle planting/selection mode
w						plant waypoint
shift+w					toggle mode for waypoints to 'random bot' or 'NPC'
escape					access menu
numberpad digits 1-9	used to plant obstacles at the respective positions of the grid
n						cycle through selected obstacles (next)
z						undo last action
y						redo last undid action
c						set paths between waypoints
ctrl+x or backspace		cut selected object(s), can be used to delete objects by not pasting afterwards
ctrl+c					copy selected object(s)
ctrl+v					paste cut/copied object(s)
alt+shift				drag/move selected object using the mouse
arrow keys				scroll around the map
ctrl+arrow keys			scroll around in bigger steps
mousewheel				scroll through obstacles of the object selector
ctrl+pageup/page down	scroll through obstacles of the object selector
g						access drop item screen
t						toggle 3x3 transparency around the crosshair
m						add/edit a map label at the position of the crosshair or the selected tile
o						zoom
tab						switch to the next editing mode
shift+tab				switch to the previous editing mode
f						switch to the next object tab
shift+f					switch to the previous object tab


Se você encontrar problemas com o editor, por favor entre em contato conosco.
E também, não tenha medo de nos enviar um mapa se achar que ele ficou legal. Nós não mordemos. :)
]]
}
