#include <SDL2/SDL.h> 
#include <iostream>
#include <cmath>
#include <vector>
using namespace std;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 500

struct Point {
	float x;
	float y;
	Point() {
		x = WINDOW_WIDTH/2;
		y = WINDOW_HEIGHT/2;
	}
	Point(float newx, float newy) {
		x = newx;
		y = newy;
	}
};

struct RGBcolor {
	int r, g, b, a;
	RGBcolor() {
		r = 255;
		g = 255;
		b = 255;
		a = 255;
	}
	RGBcolor(int ri, int gi, int bi, int ai) {
		r = ri;
		g = gi;
		b = bi;
		a = ai;
	}
};

class Screen_memory {

public:
	vector<RGBcolor> tempv;
	vector<vector<RGBcolor>> v;

	Screen_memory() {
		RGBcolor r = RGBcolor();
		tempv = vector<RGBcolor>(WINDOW_WIDTH, r);
		v = vector<vector<RGBcolor>>(WINDOW_HEIGHT, tempv);
	}
	vector<RGBcolor> &operator [](int index) {
		return v[index];
	}
};

// Declarations
int color = 0;
float pixel_size = 50.0;
Screen_memory sm;
Point screen_center;
Point center;
bool isRunning = true;
SDL_Event event;

void drawBox(SDL_Renderer* renderer, Point center, float side_len, RGBcolor color) {
	int startx = floor (center.x - (float)((float)side_len) / (float)2);
	int endx = floor( center.x + ( (float) side_len ) / (float)2 );
	int starty = floor( center.y - ((float)side_len) / (float)2);
	int endy = floor(center.y + ((float)side_len) / (float)2);


	for (int i = startx; i < endx; i++) {
		for (int j = starty; j < endy; j++) {
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b,color.a);
			SDL_RenderDrawPoint(renderer, i, j);

			if (j == starty || j == endy - 1 || i == startx || i == endx - 1) {
				SDL_SetRenderDrawColor(renderer, 0, 0xFF,0, 0);
				SDL_RenderDrawPoint(renderer, i, j);
			}
		}
	}
}

void draw_grid(SDL_Renderer *renderer, float size, int gap, Screen_memory& sm, Point v_center) {
	
	float v_pixels_x = ((float)WINDOW_WIDTH * ((float)(101 - size) / (float)100)); //5.5
	v_pixels_x = size;
	float v_pixels_y = ((float)WINDOW_HEIGHT / (float)WINDOW_WIDTH) * v_pixels_x;

	int horizontal_pixels = ceil(v_pixels_x); //6
	int vertical_pixels = ceil(v_pixels_y);

	float v_pixel_length = (float)WINDOW_WIDTH / v_pixels_x; //145.45
	cout << "v_pixel_length" << v_pixel_length << endl;

	int int_vpl = ceil((float)WINDOW_WIDTH / v_pixels_x); // 146


	float v_window_x = v_center.x - v_pixels_x / (float)2; // 8.55
	float v_window_y = v_center.y - v_pixels_y / (float)2;

	int box_center_x = ceil ((floor(v_window_x) + 0.5 - v_window_x) * v_pixel_length); //  -7.2
	int box_center_y = ceil((floor(v_window_y) + 0.5 - v_window_y) * v_pixel_length);


	//int box_length = size - 2 * (gap);
	float temp_center_x, temp_center_y;
	int temp_pixel_x, temp_pixel_y;
	cout << horizontal_pixels * vertical_pixels << endl;
	for (int i = 0; i < horizontal_pixels; i++) {
		for (int j = 0; j < vertical_pixels; j++) {

			temp_center_x = box_center_x + (i * v_pixel_length);
			temp_center_y = box_center_y + (j * v_pixel_length);
			temp_pixel_x = i + floor(v_window_x);
			temp_pixel_y = j + floor(v_window_y);

			Point center = Point( temp_center_x , temp_center_y);

			drawBox(renderer, center, v_pixel_length, RGBcolor());
		}
	}
}

void handle_size() {
	if (event.key.keysym.sym == SDLK_DOWN) {
		pixel_size = ceil((float)pixel_size * 1.1);
		//pixel_size += 0.5;
		if (pixel_size > 800)
			pixel_size = 800;
	}
	if (event.key.keysym.sym == SDLK_UP) {
		pixel_size = floor((float)pixel_size / 1.1);
		if (pixel_size < 1)
			pixel_size = 1;
	}
}

void handle_center() {

	if (event.key.keysym.sym == SDLK_w) {
		center.y -= 1.0;
	}
	if (event.key.keysym.sym == SDLK_a) {
		center.x -= 1.0;
	}
	if (event.key.keysym.sym == SDLK_s) {
		center.y += 1.0;
	}
	if (event.key.keysym.sym == SDLK_d) {
		center.x += 1.0;
	}
}



int main(int argv, char** args)
{
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window = SDL_CreateWindow("Hello SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,WINDOW_HEIGHT, 0);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	center = Point(250, 250);
	cout << center.x << endl;

	while (isRunning)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				isRunning = false;
				break;

			case SDL_KEYDOWN:

				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					isRunning = false;
				}
				handle_size();
				handle_center();
				
			}
		}

		//background --
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0XFF);
		SDL_RenderClear(renderer);
	
        //draw grid
		draw_grid(renderer,pixel_size, 0, sm, center);
		cout << "pixel size: " << pixel_size << endl;


		//line
		for (int i = 0; i < WINDOW_HEIGHT; i++) {
			SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0XFF);
			SDL_RenderDrawPoint(renderer, 500, i);
		}
		color += 1;
		color = color % 255;

        //box
		drawBox(renderer, center, 100, RGBcolor());

		SDL_RenderPresent(renderer);
	}
	

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}