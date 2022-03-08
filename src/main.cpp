#include <SDL.h>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include <random>
#include <map>
#include <climits>
#include <SDL_ttf.h>
//#include <SDL_image.h>

using namespace std;

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 400
#define MAIN_VIEW_WIDTH 400
#define MAIN_VIEW_HEIGHT 400
#define DEFAULT_BORDER_SIZE 0
#define BORDER_SIZE_LIMIT 100
#define PRINT_ON true
#define FONT_PATH "src/OpenSans-Bold.ttf"

//Classes
struct Point {
	float x;
	float y;
	Point() {
		x = MAIN_VIEW_WIDTH / 2;
		y = MAIN_VIEW_HEIGHT / 2;
	}
	Point(float newx, float newy) {
		x = newx;
		y = newy;
	}
	Point operator+(Point const &p){
		Point ans;
		ans.x = x + p.x;
		ans.y = y + p.y;
		return ans;
	}
	bool isequal(Point p){
		if (p.x == x && p.y == y){
			return true;
		}
		return false;
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
	bool is_equal(RGBcolor object2) {
		if (r == object2.r && g == object2.g && b == object2.b && a == object2.a) {
			return true;
		}
		return false;
	}
	RGBcolor operator + (RGBcolor & c_temp){
		RGBcolor ans;
		ans.r = ((r + c_temp.r)>255)?255:(r + c_temp.r);
		ans.g = ((g + c_temp.g)>255)?255:(g + c_temp.g);
		ans.b = ((b + c_temp.b)>255)?255:(b + c_temp.b);
		ans.a = ((a + c_temp.a)>255)?255:(a + c_temp.a);
		return ans;
	}
	RGBcolor operator - (RGBcolor const &c_temp){
		RGBcolor ans;
		ans.r = ((r - c_temp.r)<0)?0:(r - c_temp.r);
		ans.g = ((g - c_temp.g)<0)?0:(g - c_temp.g);
		ans.b = ((b - c_temp.b)<0)?0:(b - c_temp.b);
		ans.a = ((a - c_temp.a)<0)?0:(a - c_temp.a);
		return ans;
	}
};

class Screen_memory {

	public:
		vector<RGBcolor> tempv;
		vector<vector<RGBcolor>> v;

		Screen_memory() {
			RGBcolor r = RGBcolor(255, 225, 0, 255);
			tempv = vector<RGBcolor>(MAIN_VIEW_WIDTH, r);
			v = vector<vector<RGBcolor>>(MAIN_VIEW_HEIGHT, tempv);
		}
		vector<RGBcolor>& operator [](int index) {
			return v[index];
		}
		int size() {
			return v.size();
		}

		RGBcolor point(Point p) {
		return v[p.y][p.x];
	}

};

// Declarations -----
SDL_Renderer* renderer;
SDL_Window* window;
SDL_Event event;
int color = 0;
float pixel_size = 20.0, CENTER_SPEED;
Screen_memory sm, initial_sm;
Point screen_center;
Point center, virtual_center;
bool isRunning = true;
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<> color_distr;
float grid_border_size = DEFAULT_BORDER_SIZE, limit_border_size = BORDER_SIZE_LIMIT;
float zoom_parameter = 1.1, drag_threshold = 4;
Point mousePoint, lastmousePoint;
//managing events
int turn_on = -1, event_on = 0, first_frame_mouse_down = 1;
// game of life
RGBcolor on_color = RGBcolor(0xFF, 0, 0, 0xFF), highlight_color = RGBcolor(120, 0, 0xFF, 0xFF), fill_color = on_color;
map<string, int> continous_events_map = {
	{"mouse_left_click",0},
	{"enter",0},
	{"ctrl_left",0},
	{"mouse_right_click",0},
	{"t", 1},
	{"p",0},
	{"f",0},
	{"e",0},
	{"m",0}
};
int listening_first_point = 1;
Point line_first_screen_point, last_recorded_point, text_slot = Point(10, 10), last_highlighted_point;
TTF_Font* f;
int font_h = 30, save_last_polygon = 0;
vector<vector<Point>> polygons(1, vector<Point>()), viewport_polygons(0), polygons_being_filled(0);
int temp_count = 0, temp_limit = 50, fill_speed = 64;
vector<vector<vector<int>>> directions = {{ {1,0},{0,1} }, { {1,0},{0,-1} }, { {-1,0},{0,1} }, { {-1,0},{0,-1} }};
vector<SDL_Rect> all_viewports;
int erase_mode = 0;
vector<Point> fill_stack, fill_stack_scancode;
vector<Point> all_directions = {Point(1,0),Point(0,1), Point(-1,0), Point(0,-1)};
long long int new_time,time_diff,old_time;
int frame_rate,frame_count=0;
string input_text = "#ff0000";


//methods -------------
void print(string s) {
	if (PRINT_ON == true)
		cout << s;
	printf("\r");
}

float round_custom(float var)
{
	// 37.66666 * 100 =3766.66
	// 3766.66 + .5 =3767.16    for rounding off value
	// then type cast to int so value is 3767
	// then divided by 100 so the value converted into 37.67
	float value = (int)(var * 1000 - 0.5);
	return (float)value / 1000;
}

void next_generation() {
	Screen_memory tmp;
	for (int i = 0; i < MAIN_VIEW_HEIGHT; i++) {
		for (int j = 0; j < MAIN_VIEW_WIDTH; j++) {
			int c = 0;
			if (i == 0 || i == MAIN_VIEW_HEIGHT - 1 || j == 0 || j == MAIN_VIEW_WIDTH - 1) {
				for (int x = i - 1; x <= i + 1; x++) {
					for (int y = j - 1; y <= j + 1; y++) {
						if (x > 0 && y > 0 && x < MAIN_VIEW_WIDTH && y < MAIN_VIEW_HEIGHT && !(x == i && y == j)) {
							if (sm[x][y].is_equal(on_color)) {
								c++;
							}
						}
					}
				}
			}
			else {
				for (int x = i - 1; x <= i + 1; x++) {
					for (int y = j - 1; y <= j + 1; y++) {
						if (!(x == i && y == j)) {
							if (sm[x][y].is_equal(on_color)) {
								c++;
							}
						}
					}
				}
			}

			if (c < 2 || c > 3) {
				tmp[i][j] = initial_sm[i][j];
			}
			else if (c == 3) {
				tmp[i][j] = on_color;
			}
			else {
				tmp[i][j] = sm[i][j];
			}


		}
	}
	sm = tmp;
}

Point ConvertScreenPointToDataPoint(Point point) {

	float x = point.x, y = point.y;
	float v_pixels_x = pixel_size;
	float v_pixels_y = ((float)MAIN_VIEW_HEIGHT / (float)MAIN_VIEW_WIDTH) * v_pixels_x;
	float v_window_x = center.x - ((float)v_pixels_x / (float)2); // 8.55
	float v_window_y = center.y - ((float)v_pixels_y / (float)2);

	int current_pixel_x = v_window_x + ((float)x / (float)MAIN_VIEW_WIDTH) * (float)v_pixels_x;
	int current_pixel_y = v_window_y + ((float)y / (float)MAIN_VIEW_HEIGHT) * (float)v_pixels_y;


	return Point(current_pixel_x, current_pixel_y);
}

Point ConvertScreenPointToDataPointFloat(Point point) {

	float x = point.x, y = point.y;
	float v_pixels_x = pixel_size;
	float v_pixels_y = ((float)MAIN_VIEW_HEIGHT / (float)MAIN_VIEW_WIDTH) * v_pixels_x;
	float v_window_x = center.x - ((float)v_pixels_x / (float)2); // 8.55
	float v_window_y = center.y - ((float)v_pixels_y / (float)2);

	float current_pixel_x = v_window_x + ((float)x / (float)MAIN_VIEW_WIDTH) * (float)v_pixels_x;
	float current_pixel_y = v_window_y + ((float)y / (float)MAIN_VIEW_HEIGHT) * (float)v_pixels_y;

	return Point(current_pixel_x, current_pixel_y);
}

void reset_polygons() {
	if (save_last_polygon == 1) {
		polygons.push_back(vector<Point>());
	}
	else {
		polygons[polygons.size() - 1] = vector<Point>();
	}
	save_last_polygon = 0;
}

bool do_lines_intersect(vector<Point> line1, vector<Point> line2) {
	// (x1 - x2)y = x(y1-y2) + 
	// m = (l1.p1.y - l1.p2.y)/ (l1.p1.x - l1.p2.x)
	// y = mx + c;
	// y = m2.x + c2;
	// mx + c = m2 x + c2
	// x = (c2 - c)/(m-m2)
	// y = m x + c;

	Point ans;


	Point l1p1 = line1[0], l1p2 = line1[1], l2p1 = line2[0], l2p2 = line2[1];
	float d_l1_y = l1p2.y - l1p1.y, d_l1_x = l1p2.x - l1p1.x, d_l2_y = l2p2.y - l2p1.y, d_l2_x = l2p2.x - l2p1.x;

	if (d_l1_x == 0 && d_l2_x != 0) {
		ans.x = l1p1.x;
		ans.y = ((d_l2_y / d_l2_x) * (ans.x - l2p1.x)) + l2p1.y;
	}
	else if (d_l1_x != 0 && d_l2_x == 0) {
		ans.x = l2p1.x;
		ans.y = ((d_l1_y / d_l1_x) * (ans.x - l1p1.x)) + l1p1.y;
	}
	else {
		float m1 = (d_l1_y) / (d_l1_x), m2 = (d_l2_y) / (d_l2_x);

		if (m1 == m2) {
			return false;
		}
		else {
			ans.x = ((l2p1.y - (m2 * l2p1.x)) + ((m1 * l1p1.x) - l1p1.y)) / (m1 - m2);
			ans.y = ((d_l1_y / d_l1_x) * (ans.x - l1p1.x)) + l1p1.y;
		}

	}

	
	if (ans.y >= 0 && ans.y < MAIN_VIEW_HEIGHT && ans.x >= 0 && ans.x < MAIN_VIEW_WIDTH){
		// sm[floor(ans.y)][floor(ans.x)] = RGBcolor(0, 0, 255, 255);
	}
	if (ans.x <= max(l1p1.x, l1p2.x) && ans.x >= min(l1p1.x, l1p2.x) && ans.y <= max(l1p1.y, l1p2.y) && ans.y >= min(l1p1.y, l1p2.y)) {
		if (signbit(ans.x - l2p1.x) == signbit(l2p2.x - l2p1.x) && signbit(ans.y - l2p1.y) == signbit(l2p2.y - l2p1.y)) {
			// cout << ans.x << "::" << ans.y << endl;
			return true;
		}
	}
	return false;

}

vector<int> point_inside_any_polygon(Point p, int convert, vector<vector<Point>> polygon_list) {
	if (convert == 1) p = ConvertScreenPointToDataPointFloat(p);
	vector<int> ansv;
	for (int pi = 0; pi < polygon_list.size(); pi++) {
		vector<Point> curr_polygon = polygon_list[pi];
		if (curr_polygon.size() < 3) continue;
		int c = 0;
		Point temp_direction_point = Point((curr_polygon[0].x + curr_polygon[1].x) / (float)2, (curr_polygon[0].y + curr_polygon[1].y) / (float)2);
		vector<Point> temp_ray = { p, temp_direction_point };
		
		for (int i = 0; i < curr_polygon.size(); i++) {
			if (do_lines_intersect({ curr_polygon[(i - 1 + curr_polygon.size()) % curr_polygon.size()], curr_polygon[i] }, temp_ray)) {
				c++;
			}
		}
		;
		if (c % 2 == 1) {
			ansv.push_back(pi);
		}
	}
	return ansv;
}

void newViewPort(int x, int y, int w, int h){
	//Top left corner viewport
	SDL_Rect main_view_port;
	main_view_port.x = x;
	main_view_port.y = y;
	main_view_port.w = w;
	main_view_port.h = h;
	viewport_polygons.push_back({Point(main_view_port.x, main_view_port.y),
								Point(main_view_port.x+main_view_port.w,main_view_port.y),
								Point(main_view_port.x+main_view_port.w,main_view_port.y+main_view_port.h),
								Point(main_view_port.x,main_view_port.y+main_view_port.h)});
	all_viewports.push_back(main_view_port);
}

bool set_hex_color(string hex_color, RGBcolor &clr){
	if (!hex_color.size() == 7){
		return false;
	}
	if (!hex_color[0] == '#'){
		return false;
	}
	for (int i = 1; i <= 6; i++){
		if (!((int(hex_color[i]) >= 65 && int(hex_color[i]) <= 70) || (int(hex_color[i]) >= 48 && int(hex_color[i]) <= 57) || (int(hex_color[i]) >= 97 && int(hex_color[i]) <= 102)) ){
			cout << hex_color[i] << endl;
			return false;	
		}
	}
	int r,g,b;
	sscanf_s(hex_color.c_str(), "#%02x%02x%02x", &r, &g, &b);
    std::cout << r << ',' << g << ',' << b;
	clr = RGBcolor(r,g,b,255);
	return true;
}

//drawing
void drawBox(SDL_Renderer* renderer, Point center, float side_len, RGBcolor color, RGBcolor border_color, float border_size_percent) {
	int startx = floor(center.x - (float)((float)side_len) / (float)2);
	int endx = floor(center.x + ((float)side_len) / (float)2);
	int starty = floor(center.y - ((float)side_len) / (float)2);
	int endy = floor(center.y + ((float)side_len) / (float)2);

	int border_size = floor((border_size_percent/(float)100) * (side_len / (float)2));
	for (int i = startx; i < endx; i++) {
		for (int j = starty; j < endy; j++) {
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
			SDL_RenderDrawPoint(renderer, i, j);

			if (j < starty + border_size || j > endy - 1 - border_size
				|| i < startx + border_size || i > endx - 1 - border_size) {
				SDL_SetRenderDrawColor(renderer, border_color.r, border_color.g, border_color.b, border_color.a);
				SDL_RenderDrawPoint(renderer, i, j);
			}
		}
	}
}

void draw_rect(SDL_Renderer* renderer, Point center, float side_len_h, float side_len_w, RGBcolor color, RGBcolor border_color, float border_size_float) {
	int startx = floor(center.x - (float)((float)side_len_w) / (float)2);
	int endx = floor(center.x + ((float)side_len_w) / (float)2);
	int starty = floor(center.y - ((float)side_len_h) / (float)2);
	int endy = floor(center.y + ((float)side_len_h) / (float)2);

	int border_size = floor(border_size_float);

	for (int i = startx; i < endx; i++) {
		for (int j = starty; j < endy; j++) {
			SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
			SDL_RenderDrawPoint(renderer, i, j);

			if (j <= starty + border_size || j >= endy - 1 - border_size
				|| i <= startx + border_size || i >= endx - 1 - border_size) {
				SDL_SetRenderDrawColor(renderer, border_color.r, border_color.g, border_color.b, border_color.a);
				SDL_RenderDrawPoint(renderer, i, j);
			}
		}
	}
}

void draw_grid(SDL_Renderer* renderer, float size, int gap, Screen_memory& sm, Point v_center,
	RGBcolor grid_border_color, float grid_border_size) {

	// v_center.x = round_custom(v_center.x);
	// v_center.y = round_custom(v_center.y);

	// total Number of virtual pixels in window ++
	float v_pixels_x = size; //5.5
	float v_pixels_y = ((float)MAIN_VIEW_HEIGHT / (float)MAIN_VIEW_WIDTH) * v_pixels_x;
	float went_in = 0;

	int horizontal_pixels = ceil(v_pixels_x); //6
	int vertical_pixels = ceil(v_pixels_y);

	// Length of single virtual pixel in terms of original pixels
	float v_pixel_length = (float)MAIN_VIEW_WIDTH / v_pixels_x; //145.45
	int int_vpl = ceil((float)MAIN_VIEW_WIDTH / v_pixels_x); // 146

	// At which original pixel does the window start
	float v_window_x = v_center.x - ((float)v_pixels_x / (float)2); // 8.55
	float v_window_y = v_center.y - ((float)v_pixels_y / (float)2); //

	if (v_pixels_y + v_window_y - (float)ceil(v_window_y) > (float)(vertical_pixels - 1)) {
		vertical_pixels += 1;
	}
	if (v_pixels_x + v_window_x - (float)ceil(v_window_x) > (float)(horizontal_pixels - 1)) {
		horizontal_pixels += 1;
		//went_in = v_pixels_x + v_window_x - (float)ceil(v_window_x);
	}

	if (horizontal_pixels > MAIN_VIEW_WIDTH)horizontal_pixels = MAIN_VIEW_WIDTH;
	if (vertical_pixels > MAIN_VIEW_HEIGHT)vertical_pixels = MAIN_VIEW_HEIGHT;

	int box_center_x = ceil((floor(v_window_x) + 0.5 - v_window_x) * v_pixel_length); //  -7.2 -> -7
	int box_center_y = ceil((floor(v_window_y) + 0.5 - v_window_y) * v_pixel_length);

	//int box_length = size - 2 * (gap);
	float temp_center_x, temp_center_y;
	int temp_pixel_x, temp_pixel_y;

	for (int i = 0; i < vertical_pixels; i++) {
		temp_center_y = box_center_y + (i * v_pixel_length);

		for (int j = 0; j < horizontal_pixels; j++) {

			temp_center_x = box_center_x + (j * v_pixel_length);
			temp_pixel_x = j + floor(v_window_x);
			temp_pixel_y = i + floor(v_window_y);
			Point center = Point(temp_center_x, temp_center_y);
			if (temp_pixel_y >= MAIN_VIEW_HEIGHT) {
				temp_pixel_y = MAIN_VIEW_HEIGHT - 1;
			}
			if (temp_pixel_y < 0) {
				temp_pixel_y = 0;
			}
			if (temp_pixel_x >= MAIN_VIEW_WIDTH) {
				temp_pixel_x = MAIN_VIEW_WIDTH - 1;
			}
			if (temp_pixel_x < 0) {
				temp_pixel_x = 0;
			}

			drawBox(renderer, center, v_pixel_length, sm[temp_pixel_y][temp_pixel_x],
				grid_border_color, grid_border_size);
			if (ConvertScreenPointToDataPoint(mousePoint).y == temp_pixel_y && ConvertScreenPointToDataPoint(mousePoint).x == temp_pixel_x){
				drawBox(renderer, center, v_pixel_length, sm[temp_pixel_y][temp_pixel_x],
				highlight_color, 20);
			}
			// drawBox(renderer, center, v_pixel_length, RGBcolor(),grid_border_color, grid_border_size);
		}
	}
}

void draw_map(SDL_Renderer* renderer) {
	float map_center_ratio = 0.85;
	float map_size_ratio = 0.25;
	SDL_Rect map;
	map.h = (map_size_ratio) * (float)MAIN_VIEW_HEIGHT;
	map.w = (map_size_ratio) * (float)MAIN_VIEW_WIDTH;
	map.x = map_center_ratio * (float)MAIN_VIEW_WIDTH;
	map.y = map_center_ratio * (float)MAIN_VIEW_HEIGHT;
	Point map_center = Point(map.x, map.y);

	draw_rect(renderer, map_center, map.h, map.w, RGBcolor(0, 0, 0, 0), RGBcolor(0, 0, 0, 0xFF), 2);
	float in_map_ratio = pixel_size / (float)MAIN_VIEW_WIDTH;
	Point in_map_center = Point(
		(float)map.x - ((float)map.w / (float)2) + (center.x * map_size_ratio),
		(float)map.y - ((float)map.h / (float)2) + (center.y * map_size_ratio)
	);
	SDL_Rect in_map;
	in_map.h = (float)map.h * in_map_ratio;
	in_map.w = (float)map.w * in_map_ratio;
	draw_rect(renderer, in_map_center, in_map.h, in_map.w, RGBcolor(0, 0, 0, 0), RGBcolor(0, 0, 0, 0xFF), 2);
}

void draw_line(SDL_Renderer* renderer, Point last, Point current) {

	float v_pixels_x = pixel_size;
	float v_pixels_y = ((float)MAIN_VIEW_HEIGHT / (float)MAIN_VIEW_WIDTH) * v_pixels_x;
	float v_window_x = center.x - ((float)v_pixels_x / (float)2); // 8.55
	float v_window_y = center.y - ((float)v_pixels_y / (float)2);

	float x_step, y_step;
	// Length of single virtual pixel in terms of original pixels
	if (abs(current.x - last.x) >= abs(current.y - last.y)) {
		x_step = (float)MAIN_VIEW_WIDTH / v_pixels_x; //145.45
		y_step = ((current.y - last.y) / (float)abs(current.x - last.x)) * x_step;
	}
	else {
		y_step = ((float)MAIN_VIEW_WIDTH / v_pixels_x) * ((current.y - last.y) / (abs(current.y - last.y)));
		x_step = abs(((current.x - last.x) / (float)abs(current.y - last.y)) * y_step);
	}
	float j = last.y;
	if (x_step == 0) {
		x_step += 1;
	}
	RGBcolor color = RGBcolor(0, 0xFF, 0, 0xFF);
	if (current.x >= last.x) {
		for (float i = last.x; i <= current.x; i += x_step) {

			int current_pixel_x = v_window_x + ((float)i / (float)MAIN_VIEW_WIDTH) * (float)v_pixels_x;
			int current_pixel_y = v_window_y + ((float)j / (float)MAIN_VIEW_HEIGHT) * (float)v_pixels_y;
			if (turn_on == -1) {

				if (sm[current_pixel_y][current_pixel_x].is_equal(initial_sm[current_pixel_y][current_pixel_x])) {
					turn_on = 1;
					sm[current_pixel_y][current_pixel_x] = on_color;
				}
				else {
					sm[current_pixel_y][current_pixel_x] = initial_sm[current_pixel_y][current_pixel_x];
					turn_on = 0;
				}
			}
			else {
				if (turn_on == 1) {
					sm[current_pixel_y][current_pixel_x] = on_color;
				}
				else {
					sm[current_pixel_y][current_pixel_x] = initial_sm[current_pixel_y][current_pixel_x];
				}
			}
			j += y_step;

		}
	}
	else {
		for (float i = last.x; i >= current.x; i -= x_step) {

			int current_pixel_x = v_window_x + ((float)i / (float)MAIN_VIEW_WIDTH) * (float)v_pixels_x;
			int current_pixel_y = v_window_y + ((float)j / (float)MAIN_VIEW_HEIGHT) * (float)v_pixels_y;



			if (turn_on == -1) {

				if (sm[current_pixel_y][current_pixel_x].is_equal(initial_sm[current_pixel_y][current_pixel_x])) {
					turn_on = 1;
					sm[current_pixel_y][current_pixel_x] = on_color;
				}
				else {
					sm[current_pixel_y][current_pixel_x] = initial_sm[current_pixel_y][current_pixel_x];
					turn_on = 0;
				}
			}
			else {
				if (turn_on == 1) {
					sm[current_pixel_y][current_pixel_x] = on_color;
				}
				else {
					sm[current_pixel_y][current_pixel_x] = initial_sm[current_pixel_y][current_pixel_x];
				}
			}
			j += y_step;

		}
	}


}

void mid_point_line_draw(Point start, Point end, int convert, Screen_memory &temp_sm, RGBcolor line_color, int erase_mode) {

	if (convert == 1) {
		start = ConvertScreenPointToDataPoint(start);
		end = ConvertScreenPointToDataPoint(end);
	}

	Point current = start;
	if (erase_mode == 0){
		temp_sm[current.y][current.x] = line_color;
	}
	else{
		temp_sm[current.y][current.x] = initial_sm[current.y][current.x];
	}
	if (floor(start.x) == floor(end.x) && floor(start.y) == floor(end.y)){
		return;
	}
	float dx = end.x - start.x;
	float dy = end.y - start.y;
	if (dx == 0 && dy == 0) return;
	float d = dy - (dx / 2.f);
	float x_step, y_step;
	x_step = (signbit(dx) == 0) ? 1 : -1;
	y_step = (signbit(dy) == 0) ? 1 : -1;
	if (abs(dy) <= abs(dx)) {
		d = (dy * (x_step)) - ((dx / 2.f) * (y_step));
		while (true) {
			current.x += x_step;
			if ((d * (y_step) * (x_step)) < 0) {
				d += dy * (x_step);
			}
			else {
				d += dy * (x_step)-dx * (y_step);
				current.y += y_step;
			}
			if (erase_mode == 0){
				temp_sm[current.y][current.x] = line_color;
			}
			else{
				temp_sm[current.y][current.x] = initial_sm[current.y][current.x];
			}
			if (floor(current.x) == floor(end.x)) {
				break;
			}

		}
	}
	else {
		d = (dy / 2.f) * (x_step)-dx * (y_step);
		while (true) {
			current.y += y_step;
			if (d * (y_step) * (x_step) > 0) {
				d -= dx * (y_step);
			}
			else {
				d += dy * (x_step)-dx * (y_step);
				current.x += x_step;
			}
			if (erase_mode == 0){
				temp_sm[current.y][current.x] = line_color;
			}
			else{
				temp_sm[current.y][current.x] = initial_sm[current.y][current.x];
			}
			if (floor(current.y) == floor(end.y)) {
				break;
			}

		}
	}

	// line eq:
	// d = dy x - dx y (c = 0)
	//  f = ((dy) * x) - ((dx)*y) + ((C)*dx);

}

void recursive_fill(Point p, int convert, Screen_memory &sm_temp, int oddeven) {
	temp_count++;
	if (temp_count > temp_limit){
		return;
	}
	if (convert == 1) p = ConvertScreenPointToDataPoint(p);
	
	int i,j;
	for (int i4 = 0; i4 < 3; i4++) {
		for (int dir_i = 0; dir_i < directions[i4].size(); dir_i++) {
			i = directions[i4][dir_i][0];
			j = directions[i4][dir_i][1];
			if (sm_temp[p.y + j][p.x + i].is_equal(initial_sm[p.y + j][p.x + i])) {
				if (oddeven == 1){
					if (((int)(p.y + j))%2 == 1 && ((int)(p.x + i))%2 == 1 || ((int)(p.y + j))%2 == 0 && ((int)(p.x + i))%2 == 0){
						sm_temp[p.y + j][p.x + i] = on_color;
					}
					else{
						sm_temp[p.y + j][p.x + i] = RGBcolor(255,255,255,255);
					}
				}else{
					sm_temp[p.y + j][p.x + i] = on_color;
				}
				
				recursive_fill(Point(p.x + i, p.y + j), 0, sm_temp, oddeven);
			}
		}
	}

}

void iterative_fill(Screen_memory &sm_temp, RGBcolor fill_color, int oddeven){
	Point p = fill_stack.back();
	fill_stack.pop_back();
	if (oddeven == 1){
		if (((int)(p.y))%2 == 1 && ((int)(p.x))%2 == 1 || ((int)(p.y))%2 == 0 && ((int)(p.x))%2 == 0){
			sm_temp[p.y][p.x] = fill_color;
		}
		else{
			RGBcolor tc = RGBcolor(255,255,255,255) - fill_color;
			tc.a = 255;
			tc = RGBcolor(255,255,0,255);
			sm_temp[p.y][p.x] = tc;
		}
	}else{
		sm_temp[last_highlighted_point.y][last_highlighted_point.x] = fill_color;
		sm_temp[p.y][p.x] = highlight_color;
		last_highlighted_point = p;
	}
	for (int dir_i = 0; dir_i < all_directions.size(); dir_i++){
		Point new_p = p + all_directions[dir_i];
		if (sm_temp[new_p.y][new_p.x].is_equal(initial_sm[new_p.y][new_p.x])){
			fill_stack.push_back(new_p);
		}
	}
}

void row_fill(vector<Point> polygon_points, Screen_memory &sm_temp, RGBcolor fill_color){
	int min_x = INT_MAX, max_x = INT_MIN, min_y = INT_MAX, max_y = INT_MIN;
	for (int i = 0; i < polygon_points.size(); i++){
		if (polygon_points[i].x < min_x)
			min_x = polygon_points[i].x;
		if (polygon_points[i].x > max_x)
			max_x = polygon_points[i].x;
		if (polygon_points[i].y < min_y)
			min_y = polygon_points[i].y;
		if (polygon_points[i].y > max_y)
			max_y = polygon_points[i].y;
	}
	
	Point start_p, end_p, next_p;
	vector<int> yv( max_y - min_y + 1,0);
	vector<float> empty_v(0);
	vector<vector<float>> all_points(max_y-min_y+1,empty_v);
	for (int i = 0; i < polygon_points.size(); i++){
		start_p = polygon_points[(i - 1 + polygon_points.size())%polygon_points.size()];
		end_p = polygon_points[i];
		int j = start_p.y;
		float x;
		while (j != end_p.y){
			if (start_p.y > end_p.y){
				j--;
			}
			else{
				j++;
			}
			if (j == end_p.y) break;
			x = ((((float)j - start_p.y)*(end_p.x - start_p.x))/(end_p.y - start_p.y)) + start_p.x;
			all_points[j - min_y].push_back(x);
		}

		next_p = polygon_points[(i+1)%polygon_points.size()];
		if (signbit(end_p.y - next_p.y) != signbit(end_p.y - start_p.y)){
			all_points[end_p.y - min_y].push_back(end_p.x);
		}
		
	}
	for (int i = 0; i < all_points.size(); i++){
		sort(all_points[i].begin(), all_points[i].end());
		
		for (int j = 0; j < all_points[i].size(); j++){
			if (j%2 == 0){
				fill_stack_scancode.push_back(Point(ceil(all_points[i][j]),i+min_y));
				fill_stack_scancode.push_back(Point(floor(all_points[i][j+1]),i+min_y));
				// mid_point_line_draw(Point(ceil(all_points[i][j]),i+min_y),Point(floor(all_points[i][j+1]),i+min_y),0,sm_temp,fill_color,0);
				// mid_point_line_draw(Point(floor(all_points[i][j])+1,i+min_y),Point(ceil(all_points[i][j+1])-1,i+min_y),0,sm_temp,fill_color,0);
			}
		}
	}
	
	

}

void fill_polygon() {
	Point clicked_world_point = ConvertScreenPointToDataPoint(mousePoint);
	vector<int> to_be_filled = point_inside_any_polygon(clicked_world_point, 0, polygons);
	Screen_memory temp_sm = sm;
	
	//recursive
	if (to_be_filled.size() > 0 ){
		// recursive_fill(clicked_world_point, 0, temp_sm, continous_events_map.at("e"));
	}
	
	//iterative
	if (to_be_filled.size() > 0 && continous_events_map.at("m")){
		fill_stack.push_back(clicked_world_point);
		last_highlighted_point = clicked_world_point;
	}
	
	//row fill
	if (to_be_filled.size() > 0 && !continous_events_map.at("m")){
		for (int i = 0; i < to_be_filled.size(); i++){
			row_fill(polygons[to_be_filled[i]], temp_sm, fill_color);
			polygons_being_filled.push_back(polygons[to_be_filled[i]]);
		}
	}

	sm = temp_sm;
	temp_count = 0;
}

void show_text(string s, Point p, float size) {
	if (s.size() == 0) return;
	SDL_Surface* text;
	SDL_Texture* text_texture;
	SDL_Color color = { 0,0,0 };
	SDL_Rect dest;
	Point startpoint__to_display, endpoint_to_display;
	string initial_point_string;
	int font_w;

	text = TTF_RenderText_Solid(f, s.c_str(), color);
	font_w = (float)font_h * ((float)text->w / (float)text->h);
	// TTF_SizeText();

	if (!text) {
		cout << "Failed to render text: " << TTF_GetError() << endl;
	}

	text_texture = SDL_CreateTextureFromSurface(renderer, text);

	dest = { (int)floor(p.x), (int)floor(p.y), font_w, font_h };
	SDL_RenderCopy(renderer, text_texture, NULL, &dest);

	SDL_DestroyTexture(text_texture);
}

void record_line_points(int record_flag) {

	if (record_flag == 1 && !continous_events_map.at("p")) {
		last_recorded_point = mousePoint;
		Point virtual_point = ConvertScreenPointToDataPoint(mousePoint);
		sm[virtual_point.y][virtual_point.x] = on_color;
		line_first_screen_point = virtual_point;
	}
	else if (record_flag == 2 && !continous_events_map.at("p")) {
		mid_point_line_draw(last_recorded_point, mousePoint, 1, sm, on_color,0);
	}
	if (record_flag == 1 && continous_events_map.at("p")) {
		last_recorded_point = mousePoint;
		Point virtual_point = ConvertScreenPointToDataPoint(mousePoint);
		sm[virtual_point.y][virtual_point.x] = on_color;
		polygons[polygons.size() - 1].push_back(virtual_point);
		if (polygons[polygons.size() - 1].size() > 1) {
			mid_point_line_draw(polygons[polygons.size() - 1][polygons[polygons.size() - 1].size() - 2], polygons[polygons.size() - 1][polygons[polygons.size() - 1].size() - 1], 0, sm, on_color, 0);
		}
	}
	else if (record_flag == 2 && continous_events_map.at("p")) {

		if (polygons[polygons.size() - 1].size() > 2) {
			mid_point_line_draw(polygons[polygons.size() - 1][0], polygons[polygons.size() - 1][polygons[polygons.size() - 1].size() - 1], 0, sm, on_color,0);
		}
		save_last_polygon = 1;
		reset_polygons();
	}
}

void text_overlay() {
	if (continous_events_map.at("t") == 1) {

		text_slot = Point(10, 10);
		// text 1
		Point startpoint__to_display = ConvertScreenPointToDataPoint(mousePoint), endpoint_to_display = ConvertScreenPointToDataPoint(mousePoint);
		string initial_point_string;
		initial_point_string = to_string((int)startpoint__to_display.x) + "," + to_string((int)startpoint__to_display.y);
		show_text(initial_point_string, Point(10, 10), font_h);
		text_slot.y += font_h;
		
		//text 5
		show_text("Frame rate: " + to_string( (int)ceil(frame_rate)), text_slot, font_h);
		text_slot.y += font_h;

		// text 2
		if (continous_events_map.at("p")) {
			show_text("Polygon Mode", text_slot, font_h);
			text_slot.y += font_h;
		}

		// text 3 and 4
		if (continous_events_map.at("f")) {
			string temps = (continous_events_map.at("e")?" : oddeven":"");
			show_text("Fill mode ON" + temps, text_slot, font_h);
			text_slot.y += font_h;
			show_text("fill speed: " + to_string(fill_speed), text_slot, font_h);
			text_slot.y += font_h;
			// text 6
			if (continous_events_map.at("m")){
				show_text("Recursive Fill",text_slot, font_h);
				text_slot.y += font_h;
			}
			else{
				show_text("line by line fill",text_slot, font_h);
				text_slot.y += font_h;
			}
		}

		

		
	}
}

//input
void continous_event() {
	
	if (continous_events_map.at("mouse_left_click") == 1) {
		Point start_point = lastmousePoint;
		if (first_frame_mouse_down == 1) {
			lastmousePoint = mousePoint;
			start_point = lastmousePoint;
			first_frame_mouse_down = 0;
		}
		if (abs(mousePoint.x - lastmousePoint.x) < drag_threshold && abs(mousePoint.y - lastmousePoint.y) < drag_threshold) {
			start_point = mousePoint;
		}
		// draw_line(renderer, start_point, mousePoint);
		
		mid_point_line_draw(start_point, mousePoint, 1, sm, on_color, erase_mode);
	}
	if (continous_events_map.at("enter") == 1) {
		for (int i = 0; i < 1; i++) {
			next_generation();
		}
	}
	for (int i = 0; i < fill_speed; i++){
		if (fill_stack.size() > 0)
			iterative_fill(sm, fill_color, continous_events_map.at("e"));
		else if (fill_stack_scancode.size() > 0){
			Point p1 = fill_stack_scancode.back();
			fill_stack_scancode.pop_back();
			Point p2 = fill_stack_scancode.back();
			fill_stack_scancode.pop_back();
			mid_point_line_draw(p1,p2,0,sm,fill_color,0);
			if (fill_stack_scancode.size() == 0){
				for (int j = 0; j < polygons_being_filled.size(); j++){
					vector<Point> polygon_points = polygons_being_filled[j];
					for (int i = 0; i < polygon_points.size(); i++){
						cout << polygon_points[i].x << "::" << polygon_points[i].y << "||" << polygon_points[(i+1)%polygon_points.size()].x << "::" << polygon_points[(i+1)%polygon_points.size()].y << endl;
						mid_point_line_draw(polygon_points[i],polygon_points[(i+1)%polygon_points.size()],0,sm,on_color,0);
					}
				}
			}
		}
		else break;
	}
}

void zoom(Point zoom_center, float zoom) {

	float total_d_pixels_x_1 = pixel_size; //5.5
	float total_d_pixels_y_1 = ((float)MAIN_VIEW_HEIGHT / (float)MAIN_VIEW_WIDTH) * total_d_pixels_x_1;

	float d_pixel_x_1 = center.x - ((virtual_center.x - zoom_center.x) / ((float)MAIN_VIEW_WIDTH / pixel_size));
	float d_pixel_y_1 = center.y - ((virtual_center.y - zoom_center.y) / ((float)MAIN_VIEW_HEIGHT / total_d_pixels_y_1));
	// print(to_string(d_pixel_y_1));

	pixel_size = ((float)pixel_size * zoom);

	float total_d_pixels_x_2 = pixel_size; //5.5
	float total_d_pixels_y_2 = ((float)MAIN_VIEW_HEIGHT / (float)MAIN_VIEW_WIDTH) * total_d_pixels_x_2;
	float d_pixel_x_2 = center.x - ((virtual_center.x - zoom_center.x) / ((float)MAIN_VIEW_WIDTH / pixel_size));
	float d_pixel_y_2 = center.y - ((+virtual_center.y - zoom_center.y) / ((float)MAIN_VIEW_HEIGHT / total_d_pixels_y_2));

	center.x -= d_pixel_x_2 - d_pixel_x_1;
	center.y -= d_pixel_y_2 - d_pixel_y_1;


	if (pixel_size > MAIN_VIEW_WIDTH)
		pixel_size = MAIN_VIEW_WIDTH;
	if (pixel_size < 1)
		pixel_size = 1.0;

	float v_pixels_x = pixel_size; //5.5
	float v_pixels_y = ((float)MAIN_VIEW_HEIGHT / (float)MAIN_VIEW_WIDTH) * v_pixels_x;

	if (center.y < v_pixels_y / (float)2) {
		center.y = v_pixels_y / (float)2;
	}
	if (center.x < v_pixels_x / (float)2) {
		center.x = v_pixels_x / (float)2;
	}
	if ((float)MAIN_VIEW_HEIGHT - center.y <= v_pixels_y / (float)2) {
		center.y = round_custom((float)MAIN_VIEW_HEIGHT - v_pixels_y / (float)2);
	}
	if ((float)MAIN_VIEW_WIDTH - center.x <= v_pixels_x / (float)2) {
		center.x = round_custom((float)MAIN_VIEW_WIDTH - v_pixels_x / (float)2);
	}
}

void handle_size() {
	const Uint8* p = SDL_GetKeyboardState(NULL);

	if (event.key.keysym.sym == SDLK_DOWN) {
		if (continous_events_map.at("ctrl_left") == 1) {
			zoom(mousePoint, (float)zoom_parameter);
		}
		else {
			zoom(virtual_center, zoom_parameter);
		}
	}
	if (event.key.keysym.sym == SDLK_UP) {
		if (continous_events_map.at("ctrl_left") == 1) {
			zoom(mousePoint, (float)1 / zoom_parameter);
		}
		else {
			zoom(virtual_center, (float)1 / zoom_parameter);
		}
	}
	if (event.type == SDL_MOUSEWHEEL) {

		// total Number of virtual pixels in window ++
		float v_pixel_length_1 = (float)MAIN_VIEW_WIDTH / pixel_size;

		if (event.wheel.y > 0) {
			zoom(mousePoint, (float)1 / zoom_parameter);
			// print(to_string(event.motion.x));
		}
		else if (event.wheel.y < 0) {
			zoom(mousePoint, zoom_parameter);
		}
	}
	if (event.key.keysym.sym == SDLK_KP_PLUS && continous_events_map.at("t") && !p[SDL_SCANCODE_G]) {
		font_h++;
		if (font_h > MAIN_VIEW_HEIGHT) {
			font_h = MAIN_VIEW_HEIGHT;
		}
	}
	if (event.key.keysym.sym == SDLK_KP_MINUS && continous_events_map.at("t") && !p[SDL_SCANCODE_G]) {
		font_h--;
		if (font_h < 0) {
			font_h = 0;
		}
	}
}

void handle_center() {
	float v_pixels_x = pixel_size; //5.5
	float v_pixels_y = ((float)MAIN_VIEW_HEIGHT / (float)MAIN_VIEW_WIDTH) * v_pixels_x;
	CENTER_SPEED = pixel_size / (float)10;

	if (event.key.keysym.sym == SDLK_w) {
		center.y -= CENTER_SPEED;
		if (center.y < v_pixels_y / (float)2) {
			center.y = v_pixels_y / (float)2;
		}
	}
	if (event.key.keysym.sym == SDLK_a) {
		center.x -= CENTER_SPEED;
		if (center.x < v_pixels_x / (float)2) {
			center.x = v_pixels_x / (float)2;
		}

	}
	if (event.key.keysym.sym == SDLK_s) {
		center.y += CENTER_SPEED;
		if ((float)MAIN_VIEW_HEIGHT - (float)center.y < (float)v_pixels_y / (float)2) {
			center.y = (float)MAIN_VIEW_HEIGHT - ((float)v_pixels_y / (float)2);
		}
	}
	if (event.key.keysym.sym == SDLK_d) {
		center.x += CENTER_SPEED;
		if ((float)MAIN_VIEW_WIDTH - center.x < v_pixels_x / (float)2) {
			center.x = (float)MAIN_VIEW_WIDTH - v_pixels_x / (float)2;
		}
	}
}

void handle_grid_border() {
	const Uint8* p = SDL_GetKeyboardState(NULL);

	if (p[SDL_SCANCODE_G] && p[SDL_SCANCODE_KP_PLUS]) {
		grid_border_size += 1;
		if (grid_border_size > limit_border_size) {
			grid_border_size = limit_border_size;
		}
	}
	if (p[SDL_SCANCODE_G] && p[SDL_SCANCODE_KP_MINUS]) {
		grid_border_size -= 1;
		if (grid_border_size < -1) {
			grid_border_size = -1;
		}
	}
}

void handle_mouse_events() {
	vector<int> mouse_inside_viewport = point_inside_any_polygon(mousePoint,0,viewport_polygons);
	print(to_string(mouse_inside_viewport.size()));
	if (mouse_inside_viewport.size() == 0){
		return;
	}
	for (int i = 0; i < mouse_inside_viewport.size(); i++){
		
		//mainView
		if (mouse_inside_viewport[i] == 0){
			if (event.button.button == SDL_BUTTON_LEFT) {
				turn_on = -1;
				SDL_StopTextInput();
				if (continous_events_map.at("f")) {
					fill_polygon();
				}
				else if (continous_events_map.at("p")) {
					record_line_points(1);
				}
				else{
					continous_events_map.at("mouse_left_click") = 1;
					// draw_line(renderer, lastmousePoint, mousePoint);
					Point start_point_world = ConvertScreenPointToDataPoint(lastmousePoint);
			
					if (turn_on == -1){
						if (sm[start_point_world.y][start_point_world.x].is_equal(initial_sm[start_point_world.y][start_point_world.x])) {
							erase_mode = 0;
						}
						else {
							erase_mode = 1;
						}
						turn_on = 0;
					}
					
					mid_point_line_draw(lastmousePoint, mousePoint, 1, sm, on_color, erase_mode);
					record_line_points(1);
				}
			}
			if (event.button.button == SDL_BUTTON_RIGHT && continous_events_map.at("mouse_right_click") == 0) {
				record_line_points(2);
				continous_events_map.at("mouse_right_click") = 1;
			}
			if (event.type == SDL_MOUSEWHEEL){
				handle_size();
			}
		}
		//dashboard
		if (mouse_inside_viewport[i] == 1){
			if (event.button.button == SDL_BUTTON_LEFT){
				SDL_StopTextInput();
			}
		}
		//input field
		if (mouse_inside_viewport[i] == 2){
			if (event.button.button == SDL_BUTTON_LEFT){
				cout << "button .." << endl;
				SDL_StartTextInput();
			}
		}
		// select button
		if (mouse_inside_viewport[i] == 3){
			if (!set_hex_color(input_text, fill_color)) cout << "invalid color" << endl;
		}
	}
}

void handle_text_input(){

	if( event.type == SDL_TEXTINPUT ){
		//Not copy or pasting
		if( !( SDL_GetModState() & KMOD_CTRL && ( event.text.text[ 0 ] == 'c' || event.text.text[ 0 ] == 'C' || event.text.text[ 0 ] == 'v' || event.text.text[ 0 ] == 'V' ) ) ){
			input_text += event.text.text;
			set_hex_color(input_text, fill_color);
		}
	
	}
	else if( event.type == SDL_KEYDOWN ){
		//Handle backspace
		if( event.key.keysym.sym == SDLK_BACKSPACE && input_text.size() > 1 ){
			input_text.pop_back();
			set_hex_color(input_text, fill_color);
		}
		//Handle copy
		else if( event.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL ){
			SDL_SetClipboardText( input_text.c_str() );
			set_hex_color(input_text, fill_color);
		}
		//Handle paste
		else if( event.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL ){
			input_text = SDL_GetClipboardText();
			set_hex_color(input_text, fill_color);
		}
	}
	
}

int main(int argv, char** args) {
	//random colors to pixels
	color_distr = uniform_int_distribution<>(200, 255);
	for (int i = 0; i < MAIN_VIEW_HEIGHT; i++) {
		for (int j = 0; j < MAIN_VIEW_WIDTH; j++) {
			sm[i][j] = RGBcolor(color_distr(gen), color_distr(gen), color_distr(gen), 255);
		}
	}
	initial_sm = sm;

	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow("Hello SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
	renderer = SDL_CreateRenderer(window, -1, 0);

	center = Point(MAIN_VIEW_WIDTH / (float)2, MAIN_VIEW_HEIGHT / (float)2);

	TTF_Init();
	if (TTF_Init() < 0) {
		cout << "Error initializing SDL_ttf: " << TTF_GetError() << endl;
	}
	f = TTF_OpenFont(FONT_PATH, 24);
	if (!f) {
		cout << "Failed to load font: " << TTF_GetError() << endl;
	}

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	newViewPort(0,0,MAIN_VIEW_WIDTH,MAIN_VIEW_HEIGHT);
	newViewPort(MAIN_VIEW_WIDTH,0,WINDOW_WIDTH-MAIN_VIEW_WIDTH, WINDOW_HEIGHT);
	newViewPort(MAIN_VIEW_WIDTH + 10 ,floor((float)WINDOW_HEIGHT/((float)2)),
			floor(((float)WINDOW_WIDTH-MAIN_VIEW_WIDTH)*(0.8)), font_h);
	newViewPort(MAIN_VIEW_WIDTH + 10 ,floor((float)WINDOW_HEIGHT/((float)2)) + font_h,
			floor(((float)WINDOW_WIDTH-MAIN_VIEW_WIDTH)*(0.8)), font_h);

	old_time = SDL_GetTicks64();
	SDL_StopTextInput();

	while (isRunning)
	{

		//Inputs / Events
		while (SDL_PollEvent(&event))
		{

			if (event.type == SDL_MOUSEMOTION) {
				lastmousePoint = mousePoint;
				mousePoint.x = event.motion.x;
				mousePoint.y = event.motion.y;
			}
			
			handle_text_input();
			
			switch (event.type){
			case SDL_QUIT:
				isRunning = false;
				break;

			case SDL_KEYDOWN:

				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					isRunning = false;
					break;
				}
				if (event.key.keysym.sym == SDLK_SPACE) {
					sm = initial_sm;
					polygons = vector<vector<Point>>(1, vector<Point>());
					fill_stack = vector<Point>(0);
					fill_stack_scancode = vector<Point>(0);
					polygons_being_filled = vector<vector<Point>>(0);
					break;
				}
				if (event.key.keysym.sym == SDLK_RETURN) {
					continous_events_map.at("enter")++;
					continous_events_map.at("enter") = continous_events_map.at("enter") % 2;
					break;
				}
				if (event.key.keysym.sym == SDLK_LCTRL) {
					continous_events_map.at("ctrl_left") = 1;
				}
				if (event.key.keysym.sym == SDLK_t) {
					continous_events_map.at("t") = (continous_events_map.at("t") + 1) % 2;
				}
				if (event.key.keysym.sym == SDLK_p) {
					continous_events_map.at("p") = (continous_events_map.at("p") + 1) % 2;
					if (continous_events_map.at("p")) {
						reset_polygons();
						continous_events_map.at("f") = 0;
					}
				}
				if (event.key.keysym.sym == SDLK_f) {
					continous_events_map.at("f") = (continous_events_map.at("f") + 1) % 2;
					if (continous_events_map.at("f")) {
						continous_events_map.at("p") = 0;
						reset_polygons();
					}
				}
				if (event.key.keysym.sym == SDLK_e) {
					continous_events_map.at("e") = (continous_events_map.at("e") + 1) % 2;
				}
				if (event.key.keysym.sym == SDLK_m) {
					continous_events_map.at("m") = (continous_events_map.at("m") + 1) % 2;
				}


				if (event.key.keysym.sym == SDLK_j) {
					temp_limit-=50;
					fill_speed= fill_speed * ((fill_speed > 1)?0.5:1);
				}
				if (event.key.keysym.sym == SDLK_k) {
					temp_limit+=50;
					fill_speed = fill_speed*((fill_speed < 30000)?2:1);
				}

				handle_size();
				handle_center();
				handle_grid_border();

				break;

			case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_LCTRL) {
					continous_events_map.at("ctrl_left") = 0;
				}
				break;

			
			case SDL_MOUSEBUTTONDOWN:
				handle_mouse_events();
				break;

			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_RIGHT) {
					cout << "uped" << endl;
					continous_events_map.at("mouse_right_click") = 0;

				}
				continous_events_map.at("mouse_left_click") = 0;
				first_frame_mouse_down = 1;
				break;

			case SDL_MOUSEWHEEL:
				handle_mouse_events();

			default:
				break;

			}
		}



		SDL_RenderSetViewport( renderer, &all_viewports[0] );
		//keep doing long event
		continous_event();

		//background --
		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0XFF);
		SDL_RenderClear(renderer);

		//draw grid
		draw_grid(renderer, pixel_size, 0, sm, center, RGBcolor(0, 0, 0xFF, 0xFF), grid_border_size);
		
		//box
		drawBox(renderer, virtual_center, 50, RGBcolor(0, 0, 0, 0), RGBcolor(0, 0, 0xFF, 0xFF), 10);

		//map
		draw_map(renderer);

		SDL_RenderSetViewport( renderer, &all_viewports[1] );
		//font
		text_overlay();	

		//buttons
		SDL_RenderSetViewport( renderer, &all_viewports[2] );
		show_text(input_text, Point(0,0), font_h);
		SDL_RenderSetViewport( renderer, &all_viewports[3] );
		show_text("select color", Point(0,0), font_h);
		
		SDL_RenderPresent(renderer);
		
		//frame rate
		new_time = SDL_GetTicks64();
		time_diff = new_time - old_time;
		frame_count++;
		if (time_diff >= 1000){
			frame_rate = ceil(((float)(frame_count*1000))/(float)time_diff);
			frame_count = 0;
			old_time = new_time;
		}
	}



	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
