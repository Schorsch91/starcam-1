#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "cluster.cpp"

using namespace std;

ifstream inputPic;
ofstream outputPic;

coordinates imageDimension;

int getHeaderSize(string filename){
	inputPic.close();
	inputPic.open(filename);
	for(int a=0; a<10; a++)
    {
        inputPic.get();
    }
    int val = inputPic.get();
    inputPic.close();
    return val;
}

/* 
	returns file size in kb
*/
int getFileSize(string filename){
	long begin,end;
	// inputPic.close();
	inputPic.open(filename);
  	begin = inputPic.tellg();
  	inputPic.seekg(0,ios::end); 
  	end = inputPic.tellg();
  	// inputPic.close();
  	return (end-begin);
}

void printCoord(coordinates c, string s = ""){
	printf("%s(%d|%d)\n",s.c_str(), c.x, c.y);
}

bool** createBinaryMatrix(coordinates dimension){
	bool** m = new bool*[dimension.x];
	for(int i=0; i < dimension.x; i++){
		m[i] = new bool[dimension.y];
	}
	return m;
}

int** createIntegerMatrix(coordinates dimension){
	int** m = new int*[dimension.x];
	for(int i=0; i < dimension.x; i++){
		m[i] = new int[dimension.y];
	}
	return m;
}

int** createBrightnessMatrix(string filename, int headerSize){
	int** matrix = createIntegerMatrix(imageDimension);
	int count = 0, brightness = 0;
	inputPic.open(filename);
	inputPic.seekg(0,ios_base::beg);
	for (int i=0; i < headerSize; i++){
		inputPic.get();
	}
	for(int i = 0; i < imageDimension.x; i++){
		for (int j = 0; j < imageDimension.y; j++)
		{
			brightness += inputPic.get();
			brightness += inputPic.get();
			brightness += inputPic.get();
			matrix[i][j] = brightness / 3;
			if(brightness > 0){
				count++;
			}
			brightness = 0;	
		}
	}
	printf("non-black pixels: %d\n", count);
	return matrix;
}

Cluster findClusterAroundPoint(coordinates point, int** brightnessMatrix, bool** processedMatrix) {
	Cluster curCluster;

	

	return curCluster;
}

vector<Cluster> createClusterArray(int** brightnessMatrix){
	vector<Cluster> clusters;
	bool** processedMatrix = createBinaryMatrix(imageDimension);
	coordinates curPoint;
	for(int i = 0; i < imageDimension.x; i++){
		for (int j = 0; j < imageDimension.y; j++)
		{
			curPoint.x = i;
			curPoint.y = j;

			if(brightnessMatrix[i][j] >= 15 && !processedMatrix[i][j]){
				clusters.push_back(findClusterAroundPoint(curPoint, brightnessMatrix, processedMatrix));
			}
		}
	}
	printf("%lu cluster angelegt\n", clusters.size());
	return clusters;
}

coordinates getPicDimensions(string filename){
	int width;
	int height;
	int c;
	

	inputPic.open(filename);
	inputPic.seekg(0, ios_base::beg);
	for(int i=0; i < 18; i++){
		inputPic.get();
	}
	c = inputPic.get();
	width = c;
	c = inputPic.get();
	width = c << 8 | width;
	c = inputPic.get();
	width = c << 16 | width;
	c = inputPic.get();
	width = c << 24 | width;
	
	c = inputPic.get();
	height = c;
	c = inputPic.get();
	height = c << 8 | height;
	c = inputPic.get();
	height = c << 16 | height;
	c = inputPic.get();
	height = c << 24 | height;
	inputPic.close();
	coordinates coord;
	coord.x = width;
	coord.y = height;
	printCoord(coord, "Dimension");
	return coord;
}

int main(){
	string filename = "bild.bmp";
	int headerSize = getHeaderSize(filename);
	printf("Headersize: %d\n", headerSize);
	imageDimension = getPicDimensions(filename);
	int** brightnessMatrix = createBrightnessMatrix(filename, headerSize);
	vector<Cluster> roi = createClusterArray(brightnessMatrix);
	return 0;
}