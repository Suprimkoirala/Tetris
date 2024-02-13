//contains function that implement the game logic 

#include "game.hpp"
#include "renderer.hpp"
#include <random>
#include <algorithm>


static constexpr auto board_width = 10u;
static constexpr auto board_height = 20u;
static constexpr auto initial_frames_fall_step = 45u;
//rotation(uses SRS)

static const tetromino tetromino_data[static_cast<int> (tetromino_type::count)] =
{ //I
	{
		{
			{ { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 } },
			{ { 2, 0 }, { 2, 1 }, { 2, 2 }, { 2, 3 } },
			{ { 0, 2 }, { 1, 2 }, { 2, 2 }, { 3, 2 } },
			{ { 1, 0 }, { 1, 1 }, { 1, 2 }, { 1, 3 } },
		},
		0xd0edff, // Light blue
	
	},
	//J
	{
		{
			{ { 0, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } },
			{ { 1, 0 }, { 2, 0 }, { 1, 1 }, { 1, 2 } },
			{ { 0, 1 }, { 1, 1 }, { 2, 1 }, { 2, 2 } },
			{ { 1, 0 }, { 1, 1 }, { 0, 2 }, { 1, 2 } },
		},
		0xfffed9, // Light yellow


	},//L
	{
		{
			{ { 2, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } },
			{ { 1, 0 }, { 1, 1 }, { 1, 2 }, { 2, 2 } },
			{ { 0, 1 }, { 1, 1 }, { 2, 1 }, { 0, 2 } },
			{ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 1, 2 } },
		},
		0xa9a9a9ff, // Dark gray


	},//O
	{
		{
			{ { 1, 0 }, { 2, 0 }, { 1, 1 }, { 2, 1 } },
			{ { 1, 0 }, { 2, 0 }, { 1, 1 }, { 2, 1 } },
			{ { 1, 0 }, { 2, 0 }, { 1, 1 }, { 2, 1 } },
			{ { 1, 0 }, { 2, 0 }, { 1, 1 }, { 2, 1 } },
		},
		0xdcdcdcff, // Light gray


	},//S
	{
		{
			{ { 1, 0 }, { 2, 0 }, { 0, 1 }, { 1, 1 } },
			{ { 1, 0 }, { 1, 1 }, { 2, 1 }, { 2, 2 } },
			{ { 1, 1 }, { 2, 1 }, { 0, 2 }, { 1, 2 } },
			{ { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 2 } },
		},
		0x98fb98ff, // Pale green


	},//T
	{
		{
			{ { 1, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } },
			{ { 1, 0 }, { 1, 1 }, { 2, 1 }, { 1, 2 } },
			{ { 0, 1 }, { 1, 1 }, { 2, 1 }, { 1, 2 } },
			{ { 1, 0 }, { 0, 1 }, { 1, 1 }, { 1, 2 } },
		},
		0xe1cff8, // Plum


	},//Z
	{
		{
			{ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 2, 1 } },
			{ { 2, 0 }, { 1, 1 }, { 2, 1 }, { 1, 2 } },
			{ { 0, 1 }, { 1, 1 }, { 1, 2 }, { 2, 2 } },
			{ { 1, 0 }, { 0, 1 }, { 1, 1 }, { 0, 2 } },
		},
		 0xffb6c1ff, // Light pink


	}	
};

//check for colisions on the board
//check if the given isntanc of tetormino would collide with the board or not.
//@param Tetromino instance whose colisions need to be checked.
//@param Instanc of board against which the collisions need to be checked.
//@return true if the given tetromino would collide with the board, false otherwise

static auto is_overlap(const tetromino_instance &p_instance, const board &p_board)
{
	const auto tet = tetromino_data[static_cast<int>(p_instance.m_tetromino_type)];
	const auto block_coords = tet.block_coords[p_instance.m_rotation];
	for(auto i = 0u; i< tetromino::block_num; ++i)
	{	
		const int x = p_instance.m_pos.x + block_coords[i].x;
		const int y = p_instance.m_pos.y + block_coords[i].y;
		//overlapping with p_board borders
		if(x<0 || x>= static_cast<int>(p_board.width)|| y< 0 || y>= static_cast<int>(p_board.height))
			return true;
		if (p_board.static_blocks[x+y * p_board.width] != -1)
		return true;
	}
	return false;

}
//set the block on the board
//change the value of static block on the board
//param- instance of board on whose static blocks needs to be changed
//param - coordinates on board which needs to be set/ unset
//param value with which the static block needs to be filled
//return void

static auto set_block(board &p_board, const coords i, const unsigned int val)
{
	p_board.static_blocks[i.x * p_board.width + i.y] = val;
}
//default constructor of the game class
game::game()
:m_frames_until_fall (initial_frames_fall_step), m_game_state(game::state::title_screen), m_score(0), m_lines_cleared(0)
{
} 
//initialize the game
auto game::init_game() const -> bool
{
	return true;
}

//get the playing field ready

auto game:: start_playing ()-> void 
{
	m_board.width = board_width;
	m_board.height = board_height;
	m_score = 0;
	m_lines_cleared = 0;
	m_board.static_blocks = std::vector<int> (m_block_width * m_board_height);
	
	for(auto i = 0u; i< m_board.height; ++i)
	{
		for(auto j = 0u; j< m_board.width; ++j)
		{
			m_board.static_blocks[i * m_board.width + j] = -1;
		}
	}


//random block generation using bag ; 7-bag randomizer fills bag with 7 different tetromino pieces in any order 
//picks from bag until half empty and refills the bag with 7+ pieces

bag = {0,1,2,3,4,5,6};
std::random_device rd_bag;
std::mt19937 gen_bag(rd_bag());
std::shuffle(bag.begin(), bag.end(), gen_bag);

generate_tetromino();
m_frames_per_fall_step = initial_frames_fall_step;
}

// Contains tetromino movement and rotation mechanics and update the playing  board with their effects. The rotation of tetrominos follow Tetris's 
// standard super rotation system.
// @param input given to the game
// @return void

auto game::update_playing(const game_input &input) -> void
{
	if(input.m_pause)
	{
		m_game_state = state::paused;	
	}
 	// horizontal movement
	if(input.m_move_left)
	{
		auto temp_instance = m_active_tetromino;
		--temp_instance.m_pos.x;
		if(!is_overlap(temp_instance, m_board))
			m_active_tetromino.m_pos.x = temp_instance.m_pos.x;
	}
	if(input.m_move_right)
	{
		auto temp_instance = m_active_tetromino;
		++temp_instance.m_pos.x;
		if(!is_overlap (temp_instance, m_board))
			m_active_tetromino.m_pos.x = temp_instance.m_pos.x;	
	}
	 //rotation floowing tetris SRS
	if (input.m_rotate_clockwise)
	{
		auto temp_instance = m_active_tetromino;
		temp_instance.m_rotation = (temp_instance.m_rotation + tetromino::rotation_num -1) % tetromino::rotation_num;
		if(is_overlap(temp_instance, m_board))
		{
			temp_instance.m_pos.x = m_active_tetromino.m_pos.x -1;
			if(!is_overlap(temp_instance, m_board))
				m_active_tetromino = temp_instance;
			else
			{
				temp_instance.m_pos.x = m_active_tetromino.m_pos.x + 1;
				if(!is_overlap(temp_instance, m_board))
				m_active_tetromino = temp_instance;
			}
		}
		else 
		{
			m_active_tetromino = temp_intance;
		}

	}
	if (input.m_rotate_anticlockwise)
	{
		auto temp_instance = m_active_tetromino;
		temp_instance.m_rotation = (temp_instance.m_rotation + 1) % tetromino::rotation_num;
		if (is_overlap(temp_instance, m_board))
		{
			temp_instance.m_pos.x = m_active_tetromino.m_pos.x - 1;
			if(!is_overlap(temp_instance, m_board))
				m_active_tetromino = temp_instance;
			else
			{
				temp_instance.m_pos.x = m_active_tetromino.m_pos.x +1;
				if(!is_overlap(temp_instance, m_board))
				m_active_tetromino = temp_instance;

			}
		
		}
		else
		{
			m_active_tetromino = temp_instance;
		}
	}
	//fall
	--m_frames_until_fall;
	if (m_frames_until_fall <= 0)
	{
		m_frames_until_fall = m_frames_per_step;
		auto temp_instance = m_active_tetromino;
		++temp_instance.m_pos.y;
		if(is_overlap(temp_instance, m_board))
		{
			summon_tetris_to_board(m_board, m_active_tetromino);
			if(!generate_tetromino())
				m_game_state = state::game_over;
		}
		else
		{
			m_active_tetromino.m_pos.y = temp_instance.m_pos.y;
		}
	}
	if(input.m_soft_drop)
	{	
		auto temp_instance = m_active_tetromino;
		++temp_instance.m_pos.y;
		if(!is_overlap(temp_instance, m_board))
		{
			m_active_tetromino.m_pos.y = temp_instance.m_pos.y;
		}
	}
	if(input.m_hard_drop)
	{
		auto temp_instance = m_active_tetromino;
		while(!is_overlap(temp_instance, m_board))
		{
			++temp_instance.m_pos.y;
		}
		--temp_instance.m_pos.y;
		summon_tetromino_to_board(m_board, temp_instance);
		if(!generate_tetromino())
			m_game_state = state::game_over;
	}
	if(input.m_reset)
	{
		reset();
	}

}
// Draw the small windows for the current and next tetrominos (in queue)
// Draws a smaller icon of the tetromino to be displayed outside the falling board this is a function to be used inside the game::draw_playing function 
// @param renderer object which is used to render stuff
//* @param tetromino_index index of tetromino
// @param x0 starting x coordinate in screen from where to draw small tetromino window
//@param y0 starting y coordinate in screen from where to draw small tetromino window
// @return void


auto game::draw::draw_smalltetromino(renderer &p_renderer, int tetromino_index, int x0, int y0)
{
	static auto block_size_in_pixels = 32;
	auto board_offset_in_pixels = 32;
	for(auto i = 0u; i< 4; ++i)
	{
		const auto &tet = tetromino_data[tetromino_index];
		const auto &block_coords = tet.block_coords[m_active_tetromino.m_rotation];
		auto tetromino_color_rgba = test.color;
		const mini_scale = 0.7; //small preview block scale
		const auto x = board_offset_in_pixels.x + (block_coords[i].x) * block_size_in_pixels*mini_scale;
		const auto y = board_offset_in_pixels.y	+ (block_coords[i].y) * block_size_in_pixels*mini_scale;
		p_renderer.draw_filled_rectangle(coords (x0+x, y0+ y), block_size_in_pixels*mini_block_size_in_pixels*mini_scale, tetromino_color_rgba);
	
	}
}

//draw the renderer field
// function only tells what to render , how to render is handled by the renderer class
//param -- renderer object which is used to render stuff
//return void

auto game::draw_playing(renderer &p_renderer) -> void
{
	static auto block_size_in_pixels = 32;
	auto board_width_in_pixels = m_board.width * block_size_in_pixels;
	auto board height_in_pixels = m_board.height * block_size_in_pixels;
	auto board_offset_in_pixels = coords(0,0);
	if(p_renderer.get_width()> board_width_in_pixels)
	{
		board_offset_in_pixels.x = (p_renderer.get_width() - board_width_in_pixels)/2;
	}
	if(p_renderer.get_height()>board_height_in_pixels)
	{
		board_offset_in_pixels.y = (p_renderer.get_height() - board_height_in_pixels)/2;
	}
	for(auto i = 0u; i< m_board.height; ++i)
	{
		const auto y = board_offset_in_pixels.y + i * block_size_in_pixels;
		for(auto j = 0u; j< m_board.width; ++j)
		{
			const auto x = board_offset_in_pixels.x + j* block_size_in_pixels;
			const auto block_state = m_board.static_blocks[i*m_board.width +j];
			auto block_rgba_color = 0x333333ff;
			if(block_state != -1)
			{
				block_rgba_color = tetromino_data[block_state].color;
			}
			p_renderer.draw_filled_rectangle( coords(x,y), block_size_in_pixels, block_size_in_pixels, block_rgba_color);
			p_renderer.draw_rectangle(coords(x,y), block_size_in_pixels, block_size_in_pixels, 0x404040ff);
		}
	}
//active tetromino
	for(auto i = 0u; i<4; ++i)
	{
		const auto &tet = tetromino_data[static_cast<int>(m_active_tetromino.m_tetromino_type)];
		const auto &block_coords = tet.block_coords[m_active_tetromino.m_rotation];
		auto tetromino_color_rgba = tet.color;
		const auto x = board_offset_in_pixels.x + (m_active_tetromino.m_pos.x + block_coords[i].x) * block_size_in_pixels;
		const auto y = board_offset_in_pixels.y + (m_active_tetromino.m_pos.y + block_coords[i].y) * block_size_in_pixels;
		p_renderer.draw_filled_rectangle (coords(x, y), block_size_in_pixels, block_size_in_pixels, tetromino_color_rgba);
	}
//ghost block (represents location of current block if it were to be hard dropped
	for (auto i = 0u; i < 4; ++i)
	{
		auto ghost_block = m_active_tetromino;
		while(!is_overlap (ghost_block, m_board))
		{	
			++ghost_block.m_pos.y
		}
		--ghost_block.m_pos.y;
		const auto &tet = tetromino_data[static_cast<int>(ghost_block.m_tetromino_type)];
		const auto &block_coords = tet.block_coords[ghost_block.m_rotation];
		auto tetromino_color_rgba = tet.color;
		const auto x = board_offset_in_pixels.x + (ghost_block.m_pos.x + block_coords[i].x) * block_size_in_pixels;
		const auto y = board_offset_in_pixels.y + (ghost_block.m_pos.y + block_coords[i].y) * block_size_in_pixels;
		p_renderer.draw_rectangle(coords(x, y), block_size_in_pixels, tetromino_color_rgba);

	}
	//calculate and print 
	m_score = 100 *m_lines_cleared;
	p_renderer.draw_text("score:", {100, 100}, 0xffffffff);
	p_renderer.draw_text(std::to_string(m_score), {100, 130}, 0xffffffff);
//next 3 blocks
	p_renderer.draw_text("Next Blocks", {100, 170}, 0xffffffff);

	int next_tetro =bag.back(), next_tetro2 =bag[bag.size()-2], next_tetro3 = bag[bag.size()-3];

	p_renderer.draw_text(std::to_string(static_cast<int>(next_tetro)), {100, 200}, 0xffffffff);
	draw_smalltetromino(p_renderer, static_cast<int>(next_tetro), 130, 210);

	p_renderer.draw_text(std::to_string(static_cast<int>(next_tetro2)), {100, 350}, 0xffffffff);
	draw_smalltetromino(p_renderer, static_cast<int>(next_tetro2), 130, 360);
	
	p_renderer.draw_text(std::to_string(static_cast<int>(next_tetro3)), {100,500}, 0xffffffff);
	draw_smallertetromino(p_renderer, static_cast<int> (next_tetro3), 130, 510);


	
	


}
auto game::reset()-> void
{
	for(size_t i = 0; i<m_board.height; i++)
	{
		for(size_t j = 0; j<m_board.width;j++)
		{
			m_board.static_blocks[i*m_board.width +j] = -1;
		}
	}
	m_lines_cleared = 0;
	m_score = 0;

}

auto game::shutdown()-> void
{
}
//update on overallstate ofthe game
auto game::update(const game_input &input, float delta_time_seconds) -> void
{
	m_delta_time_seconds = delta_time_seconds;
	switch(m_game_state)
	{
		case state::title_screen:
		{
			if(input.m_start)
			{
				start_playing();
				m_game_state = state::playing;
			}
		}
		break;
		case state::playing:
		{
			update_playing(input);
		}
		break;
		case state::paused:
		{
			if(input.m_pause)
			{
				m_game_state = state::playing;
			}
		}	
		break;
		case state::game_over:
		{
			if(input.m_start)
			{
				m_game_start = state::title_screen;
			}
		}
		break;

	}
}


//draw current screen depending on the game status

auto game::draw(renderer &p_renderer) -> void
{
	switch(m_game_state)
	{
		case state::title_screen:
		{
			coords center (p_renderer.get_width()/2, p_renderer.get_height()/2;)

			p_renderer.draw_text("CONTROLS: " coords(center.x - 150, center.y -130), 0xffffffff);

			p_renderer.draw_text("Left/ right arrow --Move left / right", coords(center.x - 100, center.y-100), 0xffffffff);
			p_renderer.draw_text("Z OR UP Arrow --Rotate clockwise", coords(center.x -100, center.y - 80), 0xffffffff);

			p_renderer.draw_text("X or Left ctrl -- Rotate Counterclockwise", coords(center.x - 100, center.y -60), 0xffffffff);

			p_renderer.draw_text("SPACE --Hard Drop", coords(center.x -100, center.y - 40), 0xffffffff);
			
			p_renderer.draw_text("Down Arrow -- Soft Drop", coords(center.x - 100, center.y -20), 0xffffffff);
			p_renderer.draw_text("P --pause the game ", coords(center.x - 100, center.y), 0xffffffff);
			p_renderer.draw_text("R-- Reset game", coords(center.x - 100, center.y + 20), 0xffffffff);
			p_renderer.draw_text("Press enter to start", coords(center.x - 100, center.y +60), 0xffffffff);
		}
		break;
	
		case state::playing:
		{
			draw_playing(p_renderer);
		}
		break;
		case state::paused:
		{
			draw_playing(p_renderer);
			p_renderer.draw_text("PAUSED", coords(p_renderer.get_width() / 2 -40, p_renderer.get_height()/ 2), 0xffffffff);
		}
		break;
		case state::game_over:
		{
			draw_playing(p_renderer);
			p_renderer.draw_text("GAMEOVER", coords(p_renderer.get_width()/2 - 100,p_renderer.get_heigth()/2), 0xffffffff);
		}
		break;

	
	}
}
//generate a tetromino instance with random type and configuration
//use a pseudo random number generator to generate a tetromino with andom config and check ifit is possible tobring iton board ornot
//return true if the tetromino can be generated false otherwise
auto game::generate_tetromino()-> bool
{	pick tetromino[index] from bag
	int tetro_chosen = bag.back(); // pick from bag
	bag.pop_back(); //remove from bag

	m_active_tetromino.m_tetromino_type = static_cast<tetromino_type>(tetro_chosen);
	m_active_tetromino.m_pos.x = (m_board.width -4)/2;
	m_active_tetromino.m_pos.y = 0;

	if(bag.size() < 4)
	{
		std::vector<int>pieces = {0,1,2,3,4,5,6};
		std::random_device rd_bag;
		std::mt19937 gen_bag (rd_bag());
		std::shuffle(pieces.begin(), pieces.end(), gen_bag);
		pieces.insert(pieces.end(), bag.begin(), bag.end());
		bag = pieces;	
	}
	if(is_overlap(m_active_tetromino, m_board))
		return false;
	m_frames_until_fall = initial_frames_fall_step;
	return true;	
}

//bring the given tetromino instance on board 
auto game::summon_tetromino_to_board(board &p_board, const tetromino_instance &p_tetromino_instance) -> void
{
	const auto &tet = tetromino_data[static_cast<int>(p_tetromino_instance.m_tetromino_type)];
	const auto &block_coords = tet.block_coords[ p_tetromino_instance.m_rotation];
	for(auto i = 0u; i<tetromino::block_num; ++i)
	{
		const int x = p_tetromino_instance.m_pos.x + block_coords[i].x;
		const int y = p_tetromino_instance.m_pos.y + block_coords[i].y;
		p_board.static_blocks[x + y* p_board.width] = static_cast<unsigned int >(p_tetromino_instance.m_tetromino_type);
	}
	//clear rows
	for(auto y = 0u; y< p_board.height; ++y)
	{
		bool current_row_filled = true;
		for(auto x = 0u; x<p_board.width; ++x)
		{
			if(p_board.static_blocks[x + y* p_board.width] == -1)
			{
				current_row_filled = false;
				break;
			}
		}
		if(current_row_filled)
		{
			++m_lines_cleared;
			constexpr int difficult_step = 5;
			if(m_lines_cleared && (m_lines_cleared % difficulty_step) == 0)
			m_frames_per_fall_step = std::max(15, m_frames_per_fall_step - 5);
			if(auto i = y; i>0; --i)
			{
				for (auto j= 0u; p_board.width; ++j)
				{
					p_board.static_blocks[j + i*p_board.width] = p_board.static_blocks[j + (i - 1) * p_board.width];
				}
			}
			
		}
	}
}		
	