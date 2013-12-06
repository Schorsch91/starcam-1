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

bool isValidPixel(coordinates pixel){
	return !(pixel.x < 0 || pixel.x > imageDimension.x || 
			pixel.y < 0 || pixel.y > imageDimension.y);
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

int findMaxBrightnessAndDenoiseMatrix(int** brightnessMatrix, int brightnessThreshold = 20){
	int curMaxBrightness = 0;
	int cnt = 0;
	printf("val 851,57: %d\n", brightnessMatrix[851][57]);
	
	for(int x = 0; x < imageDimension.x; x++){
		for (int y = 0; y < imageDimension.y; y++)
		{
			if(brightnessMatrix[x][y] < brightnessThreshold){
				brightnessMatrix[x][y] = 0;
				cnt++;
			}
			else if(brightnessMatrix[x][y] > curMaxBrightness){
				curMaxBrightness = brightnessMatrix[x][y];
			}
		}
	}
	printf("%d pixel below brightnessThreshold(%d) \n", cnt, brightnessThreshold);
	return curMaxBrightness;
}

coordinates findLocalMaxBrightness(int** brightnessMatrix, coordinates center, int radius = 3){
	coordinates curPoint, maxBrightnessPoint;
	printf("val 851,57: %d\n", brightnessMatrix[851][57]);
	int localMaxBrightness = -1;
	int cnt = 0;
	int xVal = 0, yVal = 0;
	for(int x = -radius; x <= radius; x++){
		for (int y = -radius; y <= radius; y++)
		{
			curPoint.x = center.x + x;
			curPoint.y = center.y + y;

			if(isValidPixel(curPoint)){
				printf("%d at ", brightnessMatrix[curPoint.x][curPoint.x]);
			}
			printCoord(curPoint);
			if(isValidPixel(curPoint) && brightnessMatrix[curPoint.x][curPoint.x] > localMaxBrightness){
			
				maxBrightnessPoint.x = curPoint.x;
				maxBrightnessPoint.y = curPoint.y;

				localMaxBrightness = brightnessMatrix[curPoint.x][curPoint.y];
				printf("new maxBrightness %d\n", localMaxBrightness);
			}
		}
	}
	printCoord(maxBrightnessPoint, "maxBrightnessPoint in Function");
	printf("processed %d points\n", cnt);
	return maxBrightnessPoint;
}

vector<Cluster> createClusterArray(int** brightnessMatrix){
	vector<Cluster> clusters;
	bool** processedMatrix = createBinaryMatrix(imageDimension);
	printf("procMatr: %d\n", processedMatrix[123][312]);
	coordinates curPoint;

	int maxBrightness = findMaxBrightnessAndDenoiseMatrix(brightnessMatrix, 25);

	printf("Max Brightness: %d\n", maxBrightness);

	for(int x = 0; x < imageDimension.x; x++){
		for (int y = 0; y < imageDimension.y; y++)
		{
			curPoint.x = x;
			curPoint.y = y;
			coordinates localMaxBrightnessPoint;
			int localMaxBrightness;
			double relBrightness;

			if(brightnessMatrix[x][y] != 0){
				printf("curBrightness: %d at ", brightnessMatrix[x][y]);
				printCoord(curPoint, "curPoint: ");
				localMaxBrightnessPoint = findLocalMaxBrightness(brightnessMatrix, curPoint);
				printCoord(localMaxBrightnessPoint, "localMaxBrightnessPoint: ");
				localMaxBrightness = brightnessMatrix[localMaxBrightnessPoint.x][localMaxBrightnessPoint.y];
				relBrightness = (double)localMaxBrightness / maxBrightness;
				printCoord(localMaxBrightnessPoint, "localMaxBrightnessPoint: ");
				printf("localMax: %d\n", localMaxBrightness);
				printf("relBrightness: %f\n\n\n", relBrightness);
			}

		}
	}
	printf("FINISHED LOOP!\n");
	//printf("%lu cluster angelegt\n", clusters.size());
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