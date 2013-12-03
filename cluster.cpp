#include <vector>
#include <stdio.h>

using namespace std;

struct coordinates
{
	int x = -1;
	int y = -1;
};

class Cluster{
	private:
	coordinates maxPoint;
	vector<coordinates> points;

	public:
	Cluster(void);
	void addPoint(coordinates point);
	unsigned long size(void);
};

Cluster::Cluster (void) {
	//printf("Cluster created\n");
}

void Cluster::addPoint(coordinates point){
	bool isNewPoint = true;
	
	for(vector<coordinates>::iterator it = points.begin(); it != points.end(); it++){
		if(it->x == point.x && it->y == point.y){
			isNewPoint = false;
			break;
		}
	}

	if(!isNewPoint){
		points.push_back(point);
	}
}

unsigned long Cluster::size(void){
	return points.size();
}