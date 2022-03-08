#include <iostream>
#include <cmath>
#include <vector>
#include <random>
using namespace std;

#define WINDOW_WIDTH 8
#define WINDOW_HEIGHT 5

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
	bool is_equal(RGBcolor object2){
		if (r == object2.r && g == object2.g && b == object2.b && a == object2.a){
			return true;
		}
		return false;
	}
	
};

class Screen_memory {

public:
	vector<RGBcolor> tempv;
	vector<vector<RGBcolor>> v;

	Screen_memory() {
		RGBcolor r = RGBcolor(255,0,0,255);
		tempv = vector<RGBcolor>(WINDOW_WIDTH, r);
		v = vector<vector<RGBcolor>>(WINDOW_HEIGHT, tempv);
	}
	vector<RGBcolor> &operator [](int index) {
		return v[index];
	}
};

const int range_from  = 0;
const int range_to    = 10;
random_device rand_dev;
mt19937 generator(rand_dev());
uniform_int_distribution<int>  distr(range_from, range_to);



int main(){
	vector<RGBcolor> v(10,RGBcolor());
	cout << distr(generator) << '\n';


	RGBcolor r;
	RGBcolor r2;
	cout << r.r << endl;
	int g = 5;
	Screen_memory sm;
	sm[3][0] = RGBcolor(5,5,5,5);

	float a = 8.55;
	float m = floor(a) + 0.5 - a;
	cout << 6 + g*a << endl;

	cout << "ww: " << sm[0].size() << endl;


	for (int i = 0; i < WINDOW_HEIGHT; i++){
		for (int j = 0; j < WINDOW_WIDTH; j++){
			cout << sm[i][j].r << " ";
		}
		cout << endl;
	}
	
	cout << ceil(-16.2) << endl;
	if (r.is_equal(r2)){
		cout << "äsdfas" << endl;
	}
	bool x = true;
	cout << x << endl;
	return 0;
}