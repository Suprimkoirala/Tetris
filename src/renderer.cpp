#include "/include/renderer.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstdio>

//helper functions
/**@brief make SDL_Color from rgb values of given color
 *
 * @param rgb value of color as 32 bit unsigned integer
 * @return SDL_Color representing the provided color
 */
	
static auto make_sdl_color (uint32_t rgba_color)
{
	SDL_Color color;
	color.r = (unsigned char)((rgba_color >> 24) & 0xff);

	color.g = (unsigned char)((rgba_color >> 16) & 0xff);

	color.b = (unsigned char)((rgba_color >> 8) & 0xff);
	color.a = (unsigned char)((rgba_color >> 0) & 0xff);
	return color;

}
/**Print SDL renderer info on stdout
 *
 * function simply reads renderer info from provided object of type
 * SDL_rendererInfo and prints it to stdout.
 * @param SDL_RendererInfo type object containing renderer's info
 * @return void
 */

static auto print_renderer_info (SDL_RendererInfo &rendererInfo)
{
	printf("Renderer: %s software = %d accelerated = %d, presentvsync = %d " "targettexture = %d\n", rendererInfo.name, (rendererInfo.flags & SDL_RENDERER_SOFTWARE) != 0, (rendererInfo.flags & SDL_RENDERER_ACCELERATED)!= 0, (rendererInfo.flags & SDL_RENDERER_PRESENTVSYNC) != 0, (rendererInfo.flags & SDL_RENDERER_TARGETTEXTURE) != 0)
}
// class renderer

/**@brief Constructor of renderer class
 *
 * @param object of type SDL_Window on which rendering need to take palce
 * @param logical width of window
 * @param logical height of window
 */
renderer::renderer(SDL_Window &window, unsigned int width, unsigned int height): m_width(width), m_height(height), m_sdl_renderer(nullptr), m_font(nullptr)
{
	auto num_render_drivers = SDL_GetNumRenderDrivers();
	printf("%d render drivers: \n", num_render_drivers);
	for(auto i = 0; i<num_render_drivers; ++i)
	{
		SDL_RenderInfo renderer_info;
		SDL_GetRender_DriverInfo(i, &renderer_info);
		printf("%d", i);
		print_render_info(renderer_info);
	}
	Uint32 renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
	auto renderer_index = -1;
	m_sdl_renderer = SDL_CreateRenderer(&window, renderer_index, renderer_flags);
	if(!m_sdl_renderer)
	{
		fprintf(stderr, "SDL_CreateRenderer failed: %s \n", SDL_GetError());

	}
	SDL_RendererInfo renderer_info;
	if(SDL_GetRendererInfo(m_sdl_renderer, &renderer_info)!= 0)
	{
		fprintf(stderr, "SDL_GetRendererInfo failed %s\n", SDL_GetError());

	}
	printf("Created Renderer: \n");
	print_renderer_info(renderer_info);

	int display_width, display_height;
	SDL_GetWindowSize(&window, &display_width, &display_height);
	printf("Display size = (%d, %d)\n", display_width, display_height);
	if(display_width != static_cast<int> (width) || display_height != static_cast<int> (height))
	{
		printf("logical size != display size(%u, %u) vs (%u, %u). Scaling" "will be applied\n", width, height, display_width, display_height);
	}
	const auto display_aspect = static_cast<double>(display_width) / display_height;
	const auto aspect = static_cast<double> (width) / height;
	if (display_aspect != aspect)
	{
		printf("Logical Aspect != display aspect. Letterboxing will be applied\n");
	}
	SDL_RenderSetLogicalSize(m_sdl_renderer, width, height);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	//make the scaled rendering look smoother
	int default_font_size = 32;
	m_font = TTF_OpenFont("assets/clacon.ttf", default_font_size);	
	if (!m_font)
	{
		fprintf(stderr, "TTF_OpenFont failed : %s \n", TTF_GetError());
//failed to open file
	}

}
renderer::~renderer()
{
	TTF_CloseFont(m_font);
	SDL_DestroyRenderer(m_sdl_renderer);

}
//paintthe entire window with black color
auto renderer::clear() -> void
{	
	SDL_SetRenderDrawColor (m_sdl_renderer, 0, 0, 0, 255);
	SDL_RenderClear(m_sdl_renderer);
}
//update the screen with rendering info	
auto renderer::present() -> void
{
	SDL_RenderPresent(m_sdl_renderer);
}
/**@brief create an empty rectangle on the screen.
 *
 * Draw an empty rentange with the given dimentions on the given location the
 * window.
 *
 * @param location of the rectangle
 * @param width of the rectangle
 * @param height of the rectangle
 * @param color of the rectangle (rgb)
 * @return void
 */

auto renderer::draw_rectangle(const coords loc, const int width, const int height, const uint32_t rgba_color) -> void
{
	auto color = make_sdl_color (rgba_color);
	SDL_SetRenderDrawColor (m_sdl_renderer, color.r, color.g, color.b, color.a);
	SDL_Rect rect = {loc.x, loc.y, width, height};
	SDL_RenderDrawRect (m_sdl_renderer, &rect);
}


/**@brief create an filled rectangle on the screen.
 *
 * Draw an filled rentange with the given dimentions on the given location the
 * window.
 *
 * @param location of the rectangle
 * @param width of the rectangle
 * @param height of the rectangle
 * @param color of the rectangle (rgb)
 * @return void
 */
auto renderer::draw_filled_rectangle(const coords loc, const int width, const int height, const uint32_t rgba_color) -> void
{
	auto color = make_sdl_color (rgba_color);
	SDL_SetRenderDrawColor (m_sdl_renderer, color.r, color.g, color.b, color.a);
	SDL_Rect rect = {loc.x, loc.y, width, height};
	SDL_RenderFillRect (m_sdl_renderer, &rect);
	
}
/**@brief Draw text on scren
 *
 * Works for C-Style strings
 *
 * @param string (const char*) contining the text that needs to be rendered.
 * @param location on the screen where text need to be presented.
 * @param color of the text (rgb)
 * @return void
 */

auto renderer::draw_text(const char *text, const coords loc, const uint32_t rgba_color) -> void
{
	SDL_assert(text);
	SDL_Color color = make_sdl_color (rgba_color);
	SDL_Surface *sdl_surface = TTF_RenderText_Blended (m_font, text, color);
	SDL_Texture *texture = SDL_CreateTextureFromSurface(m_sdl_renderer, sdl_surface);
	int width, height;
	SDL_QueryTexture (texture, NULL, NULL, &width, &height);
	SDL_Rect dst_rect = {loc.x, loc.y, width, height};
	SDL_RenderCopy (m_sdl_renderer, texture, nullptr, &dst_rect);
	SDL_DestroyTexture (texture);
	SDL_FreeSurface(sdl_surface);
}
